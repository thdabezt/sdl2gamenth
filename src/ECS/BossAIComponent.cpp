// --- Includes ---
#include "BossAIComponent.h"
#include "Components.h"         // Includes TransformComponent, SpriteComponent, ColliderComponent, HealthComponent
#include "../game.h"            // Access Game instance, Manager, PlayerManager
#include "../AssetManager.h"    // For CreateProjectile
#include "../constants.h"       // For BOSS constants
#include "Player.h"             // For Player class definition (used via PlayerManager)
#include "../Collision.h"       // For Collision::AABB
#include <cmath>             // For std::cos, std::sin, std::sqrt, std::pow, std::max
#include <vector>          // Required for Vector2D usage (though often included indirectly)
#include <iostream>        // For std::cerr error logging

// --- Helper Functions ---

// Gets the center position of the player's collider, if available.
Vector2D getPlayerColliderCenter(Entity* pEntity) {
    if (pEntity && pEntity->isActive() && pEntity->hasComponent<ColliderComponent>()) {
        const auto& playerCollider = pEntity->getComponent<ColliderComponent>();
        // Use the collider's stored position which should reflect updates
        return Vector2D(playerCollider.position.x + playerCollider.collider.w / 2.0f,
                        playerCollider.position.y + playerCollider.collider.h / 2.0f);
    }
    return Vector2D(); // Return zero vector if player/component invalid
}

// Gets the center position based on the player's transform component.
Vector2D getPlayerTransformCenter(Entity* pEntity) {
    if (pEntity && pEntity->isActive() && pEntity->hasComponent<TransformComponent>()) {
        const auto& playerTransform = pEntity->getComponent<TransformComponent>();
        return Vector2D(playerTransform.position.x + (playerTransform.width * playerTransform.scale) / 2.0f,
                        playerTransform.position.y + (playerTransform.height * playerTransform.scale) / 2.0f);
    }
    return Vector2D(); // Return zero vector if player/component invalid
}


// --- Method Definitions ---

// --- Constructor ---
BossAIComponent::BossAIComponent(float moveSpeed, float approachDist, float contactDist, int slamDmg, int projDmg, float knockback, Entity* playerEnt)
    : playerEntity(playerEnt),
      speed(moveSpeed),
      approachDistanceThreshold(approachDist),
      contactDistanceThreshold(contactDist),
      slamDamage(slamDmg),
      projectileDamage(projDmg),
      knockbackForce(knockback)
{
    // Initial state and pointers are set in init() or header defaults
}

// --- Initialization ---
void BossAIComponent::init() {
    initialized = false;
    if (!entity) { std::cerr << "BossAIComponent Error: Entity is null!\n"; return; }

    // Get required components from owner entity
    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "BossAIComponent Error: Missing TransformComponent!\n"; return; }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<SpriteComponent>()) { std::cerr << "BossAIComponent Error: Missing SpriteComponent!\n"; return; }
    sprite = &entity->getComponent<SpriteComponent>();

    if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "BossAIComponent Error: Missing ColliderComponent!\n"; return; }
    collider = &entity->getComponent<ColliderComponent>();

    // Validate player entity pointer
    if (!playerEntity) { std::cerr << "BossAIComponent Error: Player entity pointer is null!\n"; return; }

    // Define animations (assuming separate textures per state)
    sprite->animations.clear(); // Clear any default animations
    sprite->animations.emplace("Walk", Animation(0, 8, 150));   // Assumes 8 frames, 150ms each
    sprite->animations.emplace("Charge", Animation(0, 3, 150)); // Assumes 3 frames, 150ms each
    sprite->animations.emplace("Slam", Animation(0, 10, 80));   // Assumes 10 frames, 80ms each

    // Initialize attack parameters based on base values
    currentProjectileCount = baseProjectileCount;
    currentBurstCount = baseBurstCount;
    updateAttackScaling(); // Perform initial scaling check

    // Set initial timers
    projectileAttackTimer = SDL_GetTicks() + projectileAttackInterval; // Ready to shoot after first interval
    slamCooldownEndTime = 0; // Slam available immediately

    changeState(BossState::WALKING); // Start in walking state
    initialized = true;
    // std::cout << "Boss AI Initialized." << std::endl; // Removed log
}

// --- Update Logic ---
void BossAIComponent::update() {
    // Basic validity checks
    if (!initialized || !transform || !sprite || !collider || !playerEntity || !playerEntity->isActive()) {
        if (transform) transform->velocity.Zero(); // Stop movement if invalid state
        return;
    }

    // Periodically update attack scaling based on player level
    // (Can adjust frequency, e.g., only on player level up event)
    if (SDL_GetTicks() % 1000 < 20) { // Update roughly once per second
        updateAttackScaling();
    }

    Uint32 currentTime = SDL_GetTicks();

    // --- Global Projectile Attack Check ---
    // Can the boss initiate a projectile attack sequence?
    bool canShoot = (currentState != BossState::PRE_CHARGE &&
                     currentState != BossState::CHARGING &&
                     currentState != BossState::SHOOTING_BURST &&
                     currentState != BossState::PRE_SLAM &&
                     currentState != BossState::SLAMMING &&
                     currentTime >= slamCooldownEndTime); // Not during slam cooldown

    if (canShoot && currentTime >= projectileAttackTimer) {
        changeState(BossState::PRE_CHARGE); // Start the shooting sequence
        projectileAttackTimer = currentTime + projectileAttackInterval; // Reset the global timer
        // Allow state update for PRE_CHARGE to run this frame
    }

    // --- State Machine Update ---
    switch (currentState) {
        case BossState::WALKING:            updateWalking(); break;
        case BossState::PRE_CHARGE:         updatePreCharge(); break;
        case BossState::CHARGING:           updateCharging(); break;
        case BossState::SHOOTING_BURST:     updateShootingBurst(); break;
        case BossState::PROJECTILE_COOLDOWN: updateProjectileCooldown(); break;
        case BossState::PRE_SLAM:           updatePreSlam(); break;
        case BossState::SLAMMING:           updateSlamming(); break;
    }

    // Apply velocity *unless* in a state that explicitly stops movement
    if (currentState != BossState::PRE_CHARGE &&
        currentState != BossState::CHARGING &&
        currentState != BossState::SHOOTING_BURST &&
        currentState != BossState::PROJECTILE_COOLDOWN &&
        currentState != BossState::PRE_SLAM &&
        currentState != BossState::SLAMMING)
    {
        // Position update is handled by TransformComponent::update
        // This component only sets the desired velocity.
    } else {
        // Ensure velocity is zero if in a non-moving state
        transform->velocity.Zero();
    }
}

// --- State Management ---
void BossAIComponent::changeState(BossState newState) {
    if (!sprite || !transform) return; // Need components to change state visuals/movement

    currentState = newState;
    stateTimer = SDL_GetTicks(); // Record time state was entered

    // Configure sprite and velocity for the new state
    switch (newState) {
        case BossState::WALKING:
            sprite->setTex("boss_walk");
            sprite->Play("Walk");
            // Velocity set by updateWalking based on player direction
            break;

        case BossState::PRE_CHARGE:
            transform->velocity.Zero(); // Stop movement
            sprite->setTex("boss_charge");
            sprite->Play("Charge");
            // Adjust animation speed to match state duration (optional)
            // if (sprite->animations.count("Charge") && sprite->animations["Charge"].frames > 0) {
            //      sprite->speed = preChargeDuration / sprite->animations["Charge"].frames;
            // } else { sprite->speed = 200; }
            break;

        case BossState::CHARGING:
             // Velocity already zeroed from PRE_CHARGE
            sprite->setTex("boss_charge"); // Keep charge texture
            sprite->Play("Charge");
            sprite->speed = 150; // Reset to default animation speed
            break;

        case BossState::SHOOTING_BURST:
             // Velocity already zeroed
             sprite->setTex("boss_charge"); // Keep charge texture
             sprite->Play("Charge");
            break;

        case BossState::PROJECTILE_COOLDOWN:
            transform->velocity.Zero(); // Ensure stopped
            sprite->setTex("boss_charge"); // Show charge briefly during cooldown
            sprite->Play("Charge");
            break;

        case BossState::PRE_SLAM:
            transform->velocity.Zero(); // Stop movement
            sprite->setTex("boss_slam");
            sprite->Play("Slam");
            // Adjust animation speed to match state duration (optional)
            // if (sprite->animations.count("Slam") && sprite->animations["Slam"].frames > 0) {
            //     sprite->speed = slamDuration / sprite->animations["Slam"].frames;
            // } else { sprite->speed = 80; }
            break;

        case BossState::SLAMMING:
            transform->velocity.Zero(); // Ensure stopped
            sprite->setTex("boss_slam");
            sprite->Play("Slam");
            sprite->speed = 80; // Use default slam animation speed
            break;
    }
}

// --- State Update Helpers ---
void BossAIComponent::updateWalking() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite || !playerEntity->hasComponent<ColliderComponent>()) return;

    Uint32 currentTime = SDL_GetTicks();
    bool slamReady = currentTime >= slamCooldownEndTime;

    Vector2D playerColliderCenter = getPlayerColliderCenter(playerEntity);
    Vector2D bossCenter = transform->position + Vector2D(transform->width * transform->scale / 2.0f, transform->height * transform->scale / 2.0f);
    Vector2D directionToPlayer = playerColliderCenter - bossCenter;
    float distance = Vector2D::DistanceSq(bossCenter, playerColliderCenter); // Use squared distance for efficiency

    // Check for Slam condition (in range and cooldown ready)
    if (distance <= (contactDistanceThreshold * contactDistanceThreshold) && slamReady) {
        performSlam(); // Perform damage/knockback
        changeState(BossState::SLAMMING); // Enter slamming animation state
        return; // Exit update for this frame
    }

    // Movement Logic (if not slamming)
    if (distance > 0.1f * 0.1f) { // Check if not already on top of player (small threshold)
        transform->velocity = directionToPlayer.Normalize() * speed;
    } else {
        transform->velocity.Zero(); // Stop if very close
    }

    // Update sprite animation and facing direction
    if (sprite->getTexture() != Game::instance->assets->GetTexture("boss_walk")) {
         sprite->setTex("boss_walk"); // Ensure correct texture
    }
    sprite->Play("Walk");
    if (transform->velocity.x < -0.1f) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (transform->velocity.x > 0.1f) sprite->spriteFlip = SDL_FLIP_NONE;
    // else keep current flip if moving vertically or stopped
}

void BossAIComponent::updatePreCharge() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
    // Face player during pre-charge
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    // Transition after duration
    if (SDL_GetTicks() >= stateTimer + preChargeDuration) {
        changeState(BossState::CHARGING);
    }
}

void BossAIComponent::updateCharging() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
    // Face player during charge
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    // Transition after duration, initiating the burst
    if (SDL_GetTicks() >= stateTimer + chargeDuration) {
        shootProjectile(); // Sets up burst parameters
        changeState(BossState::SHOOTING_BURST);
    }
}

void BossAIComponent::updateShootingBurst() {
    // Manages firing multiple shots with delays
    Uint32 currentTime = SDL_GetTicks();
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
             shootSingleBurstProjectile(); // Shoot one projectile
             burstShotsRemaining--;
             if (burstShotsRemaining > 0) { // If more shots left in burst
                  nextBurstShotTime = currentTime + burstDelay; // Set timer for next shot
             } else {
                  changeState(BossState::PROJECTILE_COOLDOWN); // Last shot fired
             }
        }
    } else {
         // Should not happen if logic is correct, but transition as fallback
         changeState(BossState::PROJECTILE_COOLDOWN);
    }
}

void BossAIComponent::updateProjectileCooldown() {
    // Wait for cooldown duration
    if (SDL_GetTicks() >= stateTimer + projectileCooldownDuration) {
        changeState(BossState::WALKING); // Return to walking state
    }
}

void BossAIComponent::updatePreSlam() {
    // This state might be very short or skipped if slam is purely contact-based
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
    // Face player
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    // Transition quickly to slamming (adjust duration as needed)
    if (SDL_GetTicks() >= stateTimer + 100) {
         performSlam(); // Damage/knockback happens here
         changeState(BossState::SLAMMING); // Start animation
    }
}

void BossAIComponent::updateSlamming() {
    // Wait for slam animation/duration to finish
    if (SDL_GetTicks() >= stateTimer + slamDuration) {
        // Transition back to walking after slam animation completes
        changeState(BossState::WALKING);
    }
}


// --- Action Helpers ---

// Sets up the parameters for the projectile burst sequence.
void BossAIComponent::shootProjectile() {
     if (!initialized || !transform) return;
     burstShotsRemaining = currentBurstCount; // Use scaled count
     nextBurstShotTime = SDL_GetTicks(); // First shot happens immediately
     // Optional: Play a sound indicating the start of the attack sequence
}

// Creates and fires a single spread of projectiles as part of a burst.
void BossAIComponent::shootSingleBurstProjectile() {
    if (!Game::instance || !Game::instance->assets || !transform || !playerEntity || !playerEntity->isActive()) {
        burstShotsRemaining = 0; // Cancel burst if critical components missing
        std::cerr << "BossAI Error: Cannot shoot burst projectile - invalid pointers or inactive player." << std::endl;
        return;
    }
    // Optional: Play sound per shot

    Vector2D spawnPos = transform->position + Vector2D(transform->width * transform->scale / 2.0f, transform->height * transform->scale / 2.0f);
    Vector2D targetPos = getPlayerTransformCenter(playerEntity); // Aim at player center
    Vector2D baseDirection = targetPos - spawnPos;

    // Handle case where target is exactly at spawn position
    if (baseDirection.x == 0.0f && baseDirection.y == 0.0f) { baseDirection.y = -1.0f; } // Default shoot upwards
    else { baseDirection = baseDirection.Normalize(); }

    // Calculate spread based on scaled projectile count
    int numProjectiles = currentProjectileCount;
    const double PI = acos(-1.0);
    (void)PI; // Avoid unused variable warning
    float totalSpreadAngleRad = 0.8f; // Approx 45 degrees in radians
    float angleStep = (numProjectiles > 1) ? totalSpreadAngleRad / (numProjectiles - 1) : 0.0f;
    float startAngle = (numProjectiles > 1) ? -totalSpreadAngleRad / 2.0f : 0.0f;

    for (int i = 0; i < numProjectiles; ++i) {
        float currentAngleOffset = startAngle + i * angleStep;
        Vector2D velocity;
        float cosA = std::cos(currentAngleOffset);
        float sinA = std::sin(currentAngleOffset);
        velocity.x = baseDirection.x * cosA - baseDirection.y * sinA;
        velocity.y = baseDirection.x * sinA + baseDirection.y * cosA;
        velocity = velocity * BOSS_PROJECTILE_SPEED; // Apply speed

        // Use AssetManager to create the projectile
        Game::instance->assets->CreateProjectile(
            spawnPos,
            velocity,
            projectileDamage,
            BOSS_PROJECTILE_SIZE,
            "boss_projectile", // Asset ID for the boss projectile texture
            BOSS_PROJECTILE_PIERCE
        );
    }
}

// Applies slam damage and knockback to the player.
void BossAIComponent::performSlam() {
    if (!playerEntity || !playerEntity->isActive() || !playerEntity->hasComponent<HealthComponent>()) return;

    // Damage player
    playerEntity->getComponent<HealthComponent>().takeDamage(slamDamage);

    // Apply visual hit effect to player
    if(playerEntity->hasComponent<SpriteComponent>()) {
        playerEntity->getComponent<SpriteComponent>().isHit = true;
        playerEntity->getComponent<SpriteComponent>().hitTime = SDL_GetTicks();
    }

    // Apply knockback force
    applyKnockback();

    // Set Cooldown Timer - Slam cannot occur again until this time is reached
    // Using a cooldown duration constant would be cleaner, e.g., `slamCooldownDuration = 1500;`
    slamCooldownEndTime = SDL_GetTicks() + 1500; // Example: 1.5 second cooldown

    // Optional: Play slam sound effect
    // if (sound) sound->playSoundEffect("boss_slam");
}

// Applies knockback force to the player, checking for terrain collisions.
void BossAIComponent::applyKnockback() {
    if (!playerEntity || !playerEntity->hasComponent<TransformComponent>() || !playerEntity->hasComponent<ColliderComponent>() || !transform) {
        return; // Need components on both boss and player
    }

    TransformComponent& playerTransform = playerEntity->getComponent<TransformComponent>();
    ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();
    Vector2D originalPosition = playerTransform.position; // Store pre-knockback position

    // Calculate knockback direction (away from boss center towards player center)
    Vector2D playerCenter = playerTransform.position + Vector2D(playerTransform.width*playerTransform.scale/2.0f, playerTransform.height*playerTransform.scale/2.0f);
    Vector2D bossCenter = transform->position + Vector2D(transform->width*transform->scale/2.0f, transform->height*transform->scale/2.0f);
    Vector2D knockbackDir = playerCenter - bossCenter;

    if (knockbackDir.x == 0.0f && knockbackDir.y == 0.0f) { knockbackDir.y = -1.0f; } // Default up if overlapping
    knockbackDir = knockbackDir.Normalize();

    // Apply initial knockback push
    playerTransform.position += knockbackDir * knockbackForce;

    // --- Terrain Collision Check ---
    playerCollider.update(); // Update player collider to the new potential position
    SDL_Rect knockedBackPlayerRect = playerCollider.collider;
    bool collisionDetected = false;

    if (Game::instance) {
        for (auto* terrainEntity : Game::instance->manager.getGroup(Game::groupColliders)) {
            if (terrainEntity && terrainEntity->isActive() && terrainEntity->hasComponent<ColliderComponent>()) {
                ColliderComponent& terrainColliderComp = terrainEntity->getComponent<ColliderComponent>();
                if (terrainColliderComp.tag == "terrain") { // Check only against terrain
                    if (Collision::AABB(knockedBackPlayerRect, terrainColliderComp.collider)) {
                        collisionDetected = true;
                        break; // Collision found, no need to check further
                    }
                }
            }
        }
    } else {
        std::cerr << "Warning: Game::instance is null in BossAIComponent::applyKnockback. Cannot check terrain collision." << std::endl;
    }

    // Revert position if collision occurred
    if (collisionDetected) {
        playerTransform.position = originalPosition;
        playerCollider.update(); // Update collider back to original position
    }
    // If no collision, the player keeps the knocked-back position.
}

// --- Attack Scaling ---
// Updates projectile/burst counts based on player level.
void BossAIComponent::updateAttackScaling() {
     if (!Game::instance || !Game::instance->playerManager) return;

     int playerLevel = Game::instance->playerManager->getLevel();
     int levelBonus = playerLevel / 10; // Bonus increases every 10 player levels

     // Apply scaling (ensure minimum values if needed)
     currentProjectileCount = std::max(1, baseProjectileCount + levelBonus);
     currentBurstCount = std::max(1, baseBurstCount + levelBonus);
}