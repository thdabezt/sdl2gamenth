#include "BossAIComponent.h"
#include "Components.h"         // Includes TransformComponent, SpriteComponent, ColliderComponent, HealthComponent
#include "../game.h"            // Access Game instance, Manager, PlayerManager
#include "../AssetManager.h"    // For CreateProjectile
#include "../constants.h"       // For BOSS_PROJECTILE_SIZE, BOSS_PROJECTILE_PIERCE, BOSS_PROJECTILE_SPEED
#include "Player.h"             // For Player class definition (used via PlayerManager)
#include <cmath>
#include <iostream>
#include "../Collision.h" // For Collision::AABB
#include <vector> // Required for Vector2D Distance function

// --- Helper function to get player collider center ---
Vector2D getPlayerColliderCenter(Entity* pEntity) {
    Vector2D centerPos;
    centerPos.Zero(); // Default to 0,0
    if (pEntity && pEntity->isActive() && pEntity->hasComponent<ColliderComponent>()) {
        const auto& playerCollider = pEntity->getComponent<ColliderComponent>();
        // Use collider's position member which should be updated
        centerPos.x = playerCollider.position.x + playerCollider.collider.w / 2.0f;
        centerPos.y = playerCollider.position.y + playerCollider.collider.h / 2.0f;
    }
    return centerPos;
}

// --- Helper function to get player transform center ---
Vector2D getPlayerTransformCenter(Entity* pEntity) {
    Vector2D centerPos;
    centerPos.Zero(); // Default to 0,0
    if (pEntity && pEntity->isActive() && pEntity->hasComponent<TransformComponent>()) {
        const auto& playerTransform = pEntity->getComponent<TransformComponent>();
        centerPos.x = playerTransform.position.x + (playerTransform.width * playerTransform.scale) / 2.0f;
        centerPos.y = playerTransform.position.y + (playerTransform.height * playerTransform.scale) / 2.0f;
    }
    return centerPos;
}


void BossAIComponent::init() {
    initialized = false;
    if (!entity) { std::cerr << "BossAIComponent Error: Entity is null!\n"; return; }

    // Get required components
    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "BossAIComponent Error: Missing TransformComponent!\n"; return; }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<SpriteComponent>()) { std::cerr << "BossAIComponent Error: Missing SpriteComponent!\n"; return; }
    sprite = &entity->getComponent<SpriteComponent>();

    if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "BossAIComponent Error: Missing ColliderComponent!\n"; return; }
    collider = &entity->getComponent<ColliderComponent>();

    // Get player entity pointer (should be valid if spawning logic is correct)
    if (!playerEntity) { std::cerr << "BossAIComponent Error: Player entity pointer is null!\n"; return; }

    // Define animations based on FRAME COUNTS provided by user, assuming each state uses its own texture.
    // The 'index' (row) will likely be 0 for these if each texture holds one animation.
    sprite->animations.clear();
    sprite->animations.emplace("Walk", Animation(0, 8, 150));   // Walk: 4x2=8 frames, index 0 on boss_walk.png
    sprite->animations.emplace("Charge", Animation(0, 3, 150)); // Charge: 3x1=3 frames, index 0 on boss_charge.png
    sprite->animations.emplace("Slam", Animation(0, 10, 80));   // Slam: 10 frames, index 0 on boss_slam.png

    // Initialize counts
    currentProjectileCount = baseProjectileCount;
    currentBurstCount = baseBurstCount;
    updateAttackScaling(); // Initial scaling check

    projectileAttackTimer = SDL_GetTicks() + projectileAttackInterval; // Ready to shoot after first interval passes
    slamCooldownEndTime = 0; // Ensure slam is available initially

    changeState(BossState::WALKING); // Start walking
    initialized = true;
    std::cout << "Boss AI Initialized." << std::endl;
}

// --- ADDED: Function to Update Scaling ---
void BossAIComponent::updateAttackScaling() {
     if (!Game::instance || !Game::instance->playerManager) return;

     int playerLevel = Game::instance->playerManager->getLevel();
     int levelBonus = playerLevel / 10; // Integer division gives bonus per 10 levels

     currentProjectileCount = baseProjectileCount + levelBonus;
     currentBurstCount = baseBurstCount + levelBonus;

     // Optional: Clamp values if needed
     // currentProjectileCount = std::min(currentProjectileCount, MAX_PROJECTILES);
     // currentBurstCount = std::min(currentBurstCount, MAX_BURST);

     // Optional: Log scaling changes
     // std::cout << "Boss Scaling Update (Player Lvl " << playerLevel << "): Proj=" << currentProjectileCount << ", Burst=" << currentBurstCount << std::endl;
}
// ---

void BossAIComponent::update() {
    if (!initialized || !transform || !sprite || !collider || !playerEntity || !playerEntity->isActive()) {
        if (transform) transform->velocity.Zero();
        return;
    }
    if (SDL_GetTicks() % 1000 < 20) { // Update scaling periodically
        updateAttackScaling();
    }

    Uint32 currentTime = SDL_GetTicks();

    // <<< --- GLOBAL PROJECTILE TIMER CHECK --- >>>
    // Check if NOT currently in an attack/cooldown state where shooting is disallowed
    bool canShoot = (currentState != BossState::PRE_CHARGE &&
                     currentState != BossState::CHARGING &&
                     currentState != BossState::SHOOTING_BURST &&
                     currentState != BossState::PRE_SLAM &&    // Prevent shooting while preparing slam
                     currentState != BossState::SLAMMING &&   // Prevent shooting while slamming
                     currentTime >= slamCooldownEndTime);      // Prevent shooting during slam cooldown

    if (canShoot && currentTime >= projectileAttackTimer) {
        // Time to shoot, regardless of current state (if allowed)
        changeState(BossState::PRE_CHARGE); // Start the shooting sequence
        projectileAttackTimer = currentTime + projectileAttackInterval; // Reset the 5-second timer
        // Don't return here, let the state update run for PRE_CHARGE this frame
    }
    // <<< --- END GLOBAL PROJECTILE TIMER CHECK --- >>>


    // Call state-specific update function
    switch (currentState) {
        case BossState::WALKING:            updateWalking(); break;
        // Removed IDLE_CHARGE case
        case BossState::PRE_CHARGE:         updatePreCharge(); break;
        case BossState::CHARGING:           updateCharging(); break;
        case BossState::SHOOTING_BURST:     updateShootingBurst(); break;
        case BossState::PROJECTILE_COOLDOWN: updateProjectileCooldown(); break;
        case BossState::PRE_SLAM:           updatePreSlam(); break;
        case BossState::SLAMMING:           updateSlamming(); break;
    }

    // Apply velocity (only if not in a state that forces velocity to zero, like PRE_CHARGE)
    if (currentState != BossState::PRE_CHARGE &&
        currentState != BossState::CHARGING &&
        currentState != BossState::SHOOTING_BURST &&
        currentState != BossState::PROJECTILE_COOLDOWN && // Already zeroed in changeState
        currentState != BossState::PRE_SLAM &&
        currentState != BossState::SLAMMING)
    {
        transform->position += transform->velocity;
    } else {
        // Ensure velocity is zero if in a non-moving state
        transform->velocity.Zero();
    }
}

// changeState remains the same as the previous version (transitioning from cooldowns/slamming to WALKING)
void BossAIComponent::changeState(BossState newState) {
    if (!sprite || !transform) return;

    currentState = newState;
    stateTimer = SDL_GetTicks();

    switch (newState) {
        case BossState::WALKING:
            sprite->setTex("boss_walk");
            sprite->Play("Walk");
            break;
        // NOTE: PROJECTILE_COOLDOWN now shows charge animation but transitions back to WALKING
        case BossState::PROJECTILE_COOLDOWN:
            transform->velocity.Zero();
            sprite->setTex("boss_charge");
            sprite->Play("Charge");
            break;
        // ... PRE_CHARGE, CHARGING, SHOOTING_BURST, PRE_SLAM, SLAMMING cases remain the same ...
         case BossState::PRE_CHARGE:
             transform->velocity.Zero();
             sprite->setTex("boss_charge");
             sprite->Play("Charge");
             if (sprite->animations.count("Charge") && sprite->animations["Charge"].frames > 0) {
                  sprite->speed = preChargeDuration / sprite->animations["Charge"].frames;
             } else { sprite->speed = 200; }
             break;
        case BossState::CHARGING:
            sprite->setTex("boss_charge");
            sprite->Play("Charge");
            sprite->speed = 150;
            break;
        case BossState::SHOOTING_BURST:
             sprite->setTex("boss_charge");
             sprite->Play("Charge");
            break;
        case BossState::PRE_SLAM:
            transform->velocity.Zero();
             sprite->setTex("boss_slam");
            sprite->Play("Slam");
             if (sprite->animations.count("Slam") && sprite->animations["Slam"].frames > 0) {
                 sprite->speed = slamDuration / sprite->animations["Slam"].frames;
             } else { sprite->speed = 80; }
            break;
        case BossState::SLAMMING:
            transform->velocity.Zero();
            sprite->setTex("boss_slam");
            sprite->Play("Slam");
            if (sprite->animations.count("Slam") && sprite->animations["Slam"].frames > 0) {
                sprite->speed = slamDuration / sprite->animations["Slam"].frames;
            } else { sprite->speed = 80; }
            break;
    }
}


void BossAIComponent::updateWalking() {
    // Simplified: Primarily handles movement and triggering slam on contact.
    // Projectile decision is now handled globally in update().
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite || !playerEntity->hasComponent<ColliderComponent>()) return;

    Uint32 currentTime = SDL_GetTicks();
    bool slamReady = currentTime >= slamCooldownEndTime;

    Vector2D playerColliderCenter = getPlayerColliderCenter(playerEntity);
    Vector2D bossCenter = transform->position;
    bossCenter.x += (transform->width * transform->scale) / 2.0f;
    bossCenter.y += (transform->height * transform->scale) / 2.0f;

    Vector2D direction = playerColliderCenter - bossCenter;
    float distance = Vector2D::Distance(bossCenter, playerColliderCenter);

    // --- Slam Check ---
    if (distance <= contactDistanceThreshold && slamReady) {
        performSlam();
        changeState(BossState::SLAMMING);
        return; // Action taken, exit updateWalking for this frame
    }

    // --- Movement Logic ---
    // If slam isn't ready or not in contact range, move towards player
    if (distance > 0) { // Avoid normalizing zero vector if already on target
        Vector2D moveDir = direction.Normalize();
        transform->velocity = moveDir * speed;
    } else {
        transform->velocity.Zero(); // Stop if exactly on target (unlikely)
    }

    // Ensure correct animation/texture/flip while walking
    if (sprite->getTexture() != Game::instance->assets->GetTexture("boss_walk")) {
         sprite->setTex("boss_walk");
    }
    sprite->Play("Walk");
     if (transform->velocity.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
     else if (transform->velocity.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;
     else { /* Keep current flip if velocity is zero */ }

    // Note: No transition to IDLE_CHARGE or PRE_CHARGE from here anymore.
    // The global timer check in update() handles PRE_CHARGE transition.
}



void BossAIComponent::updatePreCharge() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
    // Keep facing player
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() > stateTimer + preChargeDuration) {
        changeState(BossState::CHARGING);
    }
}

void BossAIComponent::updateCharging() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
     // Keep facing player
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() > stateTimer + chargeDuration) {
        shootProjectile(); // Setup burst parameters here
        changeState(BossState::SHOOTING_BURST); // Transition to burst handling state
    }
}

void BossAIComponent::updateShootingBurst() {
    // This state manages firing multiple shots with delays
    Uint32 currentTime = SDL_GetTicks();
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
             shootSingleBurstProjectile(); // Shoot one projectile
             burstShotsRemaining--;
             if (burstShotsRemaining > 0) { // If more shots left in burst
                  nextBurstShotTime = currentTime + burstDelay; // Set timer for next burst shot
             } else {
                  // Last shot fired, transition to cooldown
                  changeState(BossState::PROJECTILE_COOLDOWN);
             }
        }
    } else {
         // Should not happen if state logic is correct, but as fallback:
         changeState(BossState::PROJECTILE_COOLDOWN);
    }
}

void BossAIComponent::updateProjectileCooldown() {
    // State logic (playing charge anim) is handled in changeState
    // Check if cooldown finished
    if (SDL_GetTicks() > stateTimer + projectileCooldownDuration) {
        changeState(BossState::WALKING); // Go back to walking (which will check distances again)
    }
}

void BossAIComponent::updatePreSlam() {
    // This state might be skipped if slam happens on contact during WALKING
    // Keep facing player
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite) return;
    Vector2D faceDirection = getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0) sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() > stateTimer + 100) { // Very short pre-slam state?
         performSlam();
         changeState(BossState::SLAMMING);
    }
}

void BossAIComponent::updateSlamming() {
    // Wait for animation to finish
    if (SDL_GetTicks() > stateTimer + slamDuration) {
        // After slam animation, go directly back to walking state
        changeState(BossState::WALKING); // <<< Go back to WALKING
    }
}

// updateSlamCooldown function removed


// --- Action Functions ---

// Modified shootProjectile to SET UP the burst
void BossAIComponent::shootProjectile() {
     if (!initialized || !transform) return;

     // std::cout << "Boss starting projectile burst! (Burst Count: " << currentBurstCount << ")" << std::endl; // Debug
     burstShotsRemaining = currentBurstCount; // Use scaled burst count
     nextBurstShotTime = SDL_GetTicks(); // First shot happens immediately in SHOOTING_BURST state

     // Sound played ONCE per burst perhaps? Or per shot in shootSingleBurstProjectile?
     // if (sound) sound->playSoundEffect("boss_shoot_start");
}

// Shoots a single projectile as part of a burst
void BossAIComponent::shootSingleBurstProjectile() {

    // --- DEBUG LOG ---
    std::cout << "[DEBUG] BossAI: Attempting shootSingleBurstProjectile. Shots left in burst: " << burstShotsRemaining << std::endl;
    // --- END DEBUG LOG ---


    // Ensure all pointers and game instance are valid
    if (!Game::instance || !Game::instance->assets || !transform || !playerEntity || !playerEntity->isActive()) {
        burstShotsRemaining = 0; // Cancel burst if something is wrong
        std::cerr << "BossAI Error: Cannot shoot burst projectile due to invalid pointers or inactive player." << std::endl;
        return;
    }

    // Sound per shot?
    // if (sound) sound->playSoundEffect("boss_shoot_burst");

    Vector2D spawnPos = transform->position;
    spawnPos.x += (transform->width * transform->scale) / 2.0f;
    spawnPos.y += (transform->height * transform->scale) / 2.0f;

    // --- Aim at player's TRANSFORM center for better accuracy ---
    Vector2D targetPos = getPlayerTransformCenter(playerEntity);
    // ---

    Vector2D baseDirection = targetPos - spawnPos;

    // Handle case where target is exactly at spawn position
    if (baseDirection.x == 0.0f && baseDirection.y == 0.0f) {
         baseDirection.y = -1.0f; // Default shoot upwards
    } else {
         baseDirection = baseDirection.Normalize();
    }

    // Use scaled projectile count for spread
    int numProjectiles = currentProjectileCount;
    float totalSpreadAngle = 0.8f; // Radians (~45 degrees). Adjust as needed.
    float angleStep = (numProjectiles > 1) ? totalSpreadAngle / (numProjectiles - 1) : 0.0f;
    float startAngle = (numProjectiles > 1) ? -totalSpreadAngle / 2.0f : 0.0f;

    // std::cout << "  Shooting spread: " << numProjectiles << " projectiles." << std::endl; // Debug

    for (int i = 0; i < numProjectiles; ++i) {
        float currentAngleOffset = startAngle + i * angleStep;
        Vector2D velocity;
        // Rotate baseDirection by currentAngleOffset
        float cosA = std::cos(currentAngleOffset);
        float sinA = std::sin(currentAngleOffset);
        velocity.x = baseDirection.x * cosA - baseDirection.y * sinA;
        velocity.y = baseDirection.x * sinA + baseDirection.y * cosA;
        // No need to re-normalize if baseDirection was normalized and rotation preserves length
        velocity = velocity * BOSS_PROJECTILE_SPEED; // Apply speed (Use constant from constants.h)

        Game::instance->assets->CreateProjectile(
            spawnPos,
            velocity,
            projectileDamage,
            BOSS_PROJECTILE_SIZE,   // Use constant from constants.h
            "boss_projectile",      // Asset ID for the boss projectile
            BOSS_PROJECTILE_PIERCE  // Use constant from constants.h
        );
    }
}

void BossAIComponent::performSlam() {
    if (!playerEntity || !playerEntity->isActive() || !playerEntity->hasComponent<HealthComponent>()) return;
    // std::cout << "Boss SLAM!" << std::endl; // Debug

    // Damage player
    playerEntity->getComponent<HealthComponent>().takeDamage(slamDamage);
    // Apply visual hit effect to player
    if(playerEntity->hasComponent<SpriteComponent>()) {
        playerEntity->getComponent<SpriteComponent>().isHit = true;
        playerEntity->getComponent<SpriteComponent>().hitTime = SDL_GetTicks();
    }

    // Apply knockback
    applyKnockback();

    // --- Set Cooldown Timer ---
    slamCooldownEndTime = SDL_GetTicks() + 1000; // Set cooldown end time (1000ms = 1s)
    // ---

    // if (sound) sound->playSoundEffect("boss_slam");
}

void BossAIComponent::applyKnockback() {
    // Basic checks for player entity and necessary components
    if (!playerEntity || !playerEntity->hasComponent<TransformComponent>() || !playerEntity->hasComponent<ColliderComponent>() || !transform) {
        return; // Cannot apply knockback if essential components are missing
    }

    TransformComponent& playerTransform = playerEntity->getComponent<TransformComponent>();
    ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();

    // --- Store position BEFORE knockback ---
    Vector2D originalPosition = playerTransform.position;

    // --- Calculate knockback direction ---
    Vector2D knockbackDir = playerTransform.position - transform->position;
    if (knockbackDir.x == 0.0f && knockbackDir.y == 0.0f) {
        knockbackDir.y = -1.0f; // Default knockback upwards if positions overlap
    }
    knockbackDir = knockbackDir.Normalize();

    // --- Apply initial knockback push ---
    playerTransform.position += knockbackDir * knockbackForce; // Use knockbackForce member

    // --- Update player collider to the new potential position ---
    playerCollider.update(); // Updates collider.x/y based on new transform.position
    SDL_Rect knockedBackPlayerRect = playerCollider.collider; // Get the rect at the new position

    // --- Check for collision with terrain AFTER knockback ---
    bool collisionDetected = false;
    if (Game::instance) { // Check if Game instance exists
        auto& terrainColliders = Game::instance->manager.getGroup(Game::groupColliders);
        for (auto* terrainEntity : terrainColliders) {
            if (terrainEntity && terrainEntity->isActive() && terrainEntity->hasComponent<ColliderComponent>()) {
                ColliderComponent& terrainColliderComp = terrainEntity->getComponent<ColliderComponent>();
                // Ensure it's actually terrain (though the group should handle this)
                if (terrainColliderComp.tag == "terrain") {
                    if (Collision::AABB(knockedBackPlayerRect, terrainColliderComp.collider)) {
                        collisionDetected = true;
                        // std::cout << "Knockback collision detected with terrain!" << std::endl; // Optional debug log
                        break; // Stop checking once a collision is found
                    }
                }
            }
        }
    } else {
        std::cerr << "Warning: Game::instance is null in BossAIComponent::applyKnockback. Cannot check terrain collision." << std::endl;
    }

    // --- Revert position if collision occurred ---
    if (collisionDetected) {
        playerTransform.position = originalPosition; // Revert to position before knockback
        playerCollider.update(); // Update collider back to original position
        // std::cout << "Player position reverted due to knockback collision." << std::endl; // Optional debug log
    }
    // If no collision, the player keeps the knocked-back position.
}