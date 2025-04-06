#include <cmath>
#include <iostream>
#include <vector>

#include "../AssetManager.h"
#include "../Collision.h"
#include "../constants.h"
#include "../game.h"
#include "BossAIComponent.h"
#include "Components.h"
#include "Player.h"

Vector2D getPlayerColliderCenter(Entity* pEntity) {
    if (pEntity && pEntity->isActive() &&
        pEntity->hasComponent<ColliderComponent>()) {
        const auto& playerCollider = pEntity->getComponent<ColliderComponent>();

        return Vector2D(
            playerCollider.position.x + playerCollider.collider.w / 2.0f,
            playerCollider.position.y + playerCollider.collider.h / 2.0f);
    }
    return Vector2D();
}

Vector2D getPlayerTransformCenter(Entity* pEntity) {
    if (pEntity && pEntity->isActive() &&
        pEntity->hasComponent<TransformComponent>()) {
        const auto& playerTransform =
            pEntity->getComponent<TransformComponent>();
        return Vector2D(
            playerTransform.position.x +
                (playerTransform.width * playerTransform.scale) / 2.0f,
            playerTransform.position.y +
                (playerTransform.height * playerTransform.scale) / 2.0f);
    }
    return Vector2D();
}

BossAIComponent::BossAIComponent(float moveSpeed, float approachDist,
    float contactDist, int baseSlamDmg, int baseProjDmg,
    float knockback, Entity* playerEnt)
        : playerEntity(playerEnt),
        speed(moveSpeed),
        approachDistanceThreshold(approachDist),
        contactDistanceThreshold(contactDist),
        knockbackForce(knockback),         
        baseSlamDamage(baseSlamDmg),
        baseProjectileDamage(baseProjDmg),
        currentSlamDamage(baseSlamDmg),
        currentProjectileDamage(baseProjDmg) 
        {}

void BossAIComponent::init() {
    initialized = false;
    if (!entity) {
        std::cerr << "BossAIComponent Error: Entity is null!\n";
        return;
    }

    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr << "BossAIComponent Error: Missing TransformComponent!\n";
        return;
    }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<SpriteComponent>()) {
        std::cerr << "BossAIComponent Error: Missing SpriteComponent!\n";
        return;
    }
    sprite = &entity->getComponent<SpriteComponent>();

    if (!entity->hasComponent<ColliderComponent>()) {
        std::cerr << "BossAIComponent Error: Missing ColliderComponent!\n";
        return;
    }
    collider = &entity->getComponent<ColliderComponent>();

    if (!playerEntity) {
        std::cerr << "BossAIComponent Error: Player entity pointer is null!\n";
        return;
    }

    sprite->animations.clear();
    sprite->animations.emplace("Walk", Animation(0, 8, 150));
    sprite->animations.emplace("Charge", Animation(0, 3, 150));
    sprite->animations.emplace("Slam", Animation(0, 10, 80));

    currentProjectileCount = baseProjectileCount;
    currentBurstCount = baseBurstCount;
    updateAttackScaling();

    projectileAttackTimer = SDL_GetTicks() + projectileAttackInterval;
    slamCooldownEndTime = 0;

    changeState(BossState::WALKING);
    initialized = true;
}

void BossAIComponent::update() {
    if (!initialized || !transform || !sprite || !collider || !playerEntity ||
        !playerEntity->isActive()) {
        if (transform) transform->velocity.Zero();
        return;
    }

    if (SDL_GetTicks() % 1000 < 20) {
        updateAttackScaling();
    }

    Uint32 currentTime = SDL_GetTicks();

    bool canShoot = (currentState != BossState::PRE_CHARGE &&
                     currentState != BossState::CHARGING &&
                     currentState != BossState::SHOOTING_BURST &&
                     currentState != BossState::PRE_SLAM &&
                     currentState != BossState::SLAMMING &&
                     currentTime >= slamCooldownEndTime);

    if (canShoot && currentTime >= projectileAttackTimer) {
        changeState(BossState::PRE_CHARGE);
        projectileAttackTimer = currentTime + projectileAttackInterval;
    }

    switch (currentState) {
        case BossState::WALKING:
            updateWalking();
            break;
        case BossState::PRE_CHARGE:
            updatePreCharge();
            break;
        case BossState::CHARGING:
            updateCharging();
            break;
        case BossState::SHOOTING_BURST:
            updateShootingBurst();
            break;
        case BossState::PROJECTILE_COOLDOWN:
            updateProjectileCooldown();
            break;
        case BossState::PRE_SLAM:
            updatePreSlam();
            break;
        case BossState::SLAMMING:
            updateSlamming();
            break;
    }

    if (currentState != BossState::PRE_CHARGE &&
        currentState != BossState::CHARGING &&
        currentState != BossState::SHOOTING_BURST &&
        currentState != BossState::PROJECTILE_COOLDOWN &&
        currentState != BossState::PRE_SLAM &&
        currentState != BossState::SLAMMING) {
    } else {
        transform->velocity.Zero();
    }
}

void BossAIComponent::changeState(BossState newState) {
    if (!sprite || !transform) return;

    currentState = newState;
    stateTimer = SDL_GetTicks();

    switch (newState) {
        case BossState::WALKING:
            sprite->setTex("boss_walk");
            sprite->Play("Walk");

            break;

        case BossState::PRE_CHARGE:
            transform->velocity.Zero();
            sprite->setTex("boss_charge");
            sprite->Play("Charge");

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

        case BossState::PROJECTILE_COOLDOWN:
            transform->velocity.Zero();
            sprite->setTex("boss_charge");
            sprite->Play("Charge");
            break;

        case BossState::PRE_SLAM:
            transform->velocity.Zero();
            sprite->setTex("boss_slam");
            sprite->Play("Slam");

            break;

        case BossState::SLAMMING:
            transform->velocity.Zero();
            sprite->setTex("boss_slam");
            sprite->Play("Slam");
            sprite->speed = 80;
            break;
    }
}

void BossAIComponent::updateWalking() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite ||
        !playerEntity->hasComponent<ColliderComponent>())
        return;

    Uint32 currentTime = SDL_GetTicks();
    bool slamReady = currentTime >= slamCooldownEndTime;

    Vector2D playerColliderCenter = getPlayerColliderCenter(playerEntity);
    Vector2D bossCenter = transform->position +
                          Vector2D(transform->width * transform->scale / 2.0f,
                                   transform->height * transform->scale / 2.0f);
    Vector2D directionToPlayer = playerColliderCenter - bossCenter;
    float distance = Vector2D::DistanceSq(bossCenter, playerColliderCenter);

    if (distance <= (contactDistanceThreshold * contactDistanceThreshold) &&
        slamReady) {
        performSlam();
        changeState(BossState::SLAMMING);
        return;
    }

    if (distance > 0.1f * 0.1f) {
        transform->velocity = directionToPlayer.Normalize() * speed;
    } else {
        transform->velocity.Zero();
    }

    if (sprite->getTexture() !=
        Game::instance->assets->GetTexture("boss_walk")) {
        sprite->setTex("boss_walk");
    }
    sprite->Play("Walk");
    if (transform->velocity.x < -0.1f)
        sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (transform->velocity.x > 0.1f)
        sprite->spriteFlip = SDL_FLIP_NONE;
}

void BossAIComponent::updatePreCharge() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite)
        return;

    Vector2D faceDirection =
        getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0)
        sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0)
        sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() >= stateTimer + preChargeDuration) {
        changeState(BossState::CHARGING);
    }
}

void BossAIComponent::updateCharging() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite)
        return;

    Vector2D faceDirection =
        getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0)
        sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0)
        sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() >= stateTimer + chargeDuration) {
        shootProjectile();
        changeState(BossState::SHOOTING_BURST);
    }
}

void BossAIComponent::updateShootingBurst() {
    Uint32 currentTime = SDL_GetTicks();
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            shootSingleBurstProjectile();
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) {
                nextBurstShotTime = currentTime + burstDelay;
            } else {
                changeState(BossState::PROJECTILE_COOLDOWN);
            }
        }
    } else {
        changeState(BossState::PROJECTILE_COOLDOWN);
    }
}

void BossAIComponent::updateProjectileCooldown() {
    if (SDL_GetTicks() >= stateTimer + projectileCooldownDuration) {
        changeState(BossState::WALKING);
    }
}

void BossAIComponent::updatePreSlam() {
    if (!playerEntity || !playerEntity->isActive() || !transform || !sprite)
        return;

    Vector2D faceDirection =
        getPlayerTransformCenter(playerEntity) - transform->position;
    if (faceDirection.x < 0)
        sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
    else if (faceDirection.x > 0)
        sprite->spriteFlip = SDL_FLIP_NONE;

    if (SDL_GetTicks() >= stateTimer + 100) {
        performSlam();
        changeState(BossState::SLAMMING);
    }
}

void BossAIComponent::updateSlamming() {
    if (SDL_GetTicks() >= stateTimer + slamDuration) {
        changeState(BossState::WALKING);
    }
}

void BossAIComponent::shootProjectile() {
    if (!initialized || !transform) return;
    burstShotsRemaining = currentBurstCount;
    nextBurstShotTime = SDL_GetTicks();
}

void BossAIComponent::shootSingleBurstProjectile() {
    if (!Game::instance || !Game::instance->assets || !transform ||
        !playerEntity || !playerEntity->isActive()) {
        burstShotsRemaining = 0;
        std::cerr << "BossAI Error: Cannot shoot burst projectile - invalid "
                     "pointers or inactive player."
                  << std::endl;
        return;
    }

    Vector2D spawnPos = transform->position +
                        Vector2D(transform->width * transform->scale / 2.0f,
                                 transform->height * transform->scale / 2.0f);
    Vector2D targetPos = getPlayerTransformCenter(playerEntity);
    Vector2D baseDirection = targetPos - spawnPos;

    if (baseDirection.x == 0.0f && baseDirection.y == 0.0f) {
        baseDirection.y = -1.0f;
    } else {
        baseDirection = baseDirection.Normalize();
    }

    int numProjectiles = currentProjectileCount;
    const double PI = acos(-1.0);
    (void)PI;
    float totalSpreadAngleRad = 0.8f;
    float angleStep = (numProjectiles > 1)
                          ? totalSpreadAngleRad / (numProjectiles - 1)
                          : 0.0f;
    float startAngle =
        (numProjectiles > 1) ? -totalSpreadAngleRad / 2.0f : 0.0f;

    for (int i = 0; i < numProjectiles; ++i) {
        float currentAngleOffset = startAngle + i * angleStep;
        Vector2D velocity;
        float cosA = std::cos(currentAngleOffset);
        float sinA = std::sin(currentAngleOffset);
        velocity.x = baseDirection.x * cosA - baseDirection.y * sinA;
        velocity.y = baseDirection.x * sinA + baseDirection.y * cosA;
        velocity = velocity * BOSS_PROJECTILE_SPEED;

        Game::instance->assets->CreateProjectile(
            spawnPos, velocity, currentProjectileDamage, BOSS_PROJECTILE_SIZE,
            "boss_projectile", BOSS_PROJECTILE_PIERCE);
    }
}

void BossAIComponent::performSlam() {
    if (!playerEntity || !playerEntity->isActive() ||
        !playerEntity->hasComponent<HealthComponent>())
        return;

    playerEntity->getComponent<HealthComponent>().takeDamage(currentSlamDamage);

    if (playerEntity->hasComponent<SpriteComponent>()) {
        playerEntity->getComponent<SpriteComponent>().isHit = true;
        playerEntity->getComponent<SpriteComponent>().hitTime = SDL_GetTicks();
    }

    applyKnockback();

    slamCooldownEndTime = SDL_GetTicks() + 1500;
}

void BossAIComponent::applyKnockback() {
    if (!playerEntity || !playerEntity->hasComponent<TransformComponent>() ||
        !playerEntity->hasComponent<ColliderComponent>() || !transform) {
        return;
    }

    TransformComponent& playerTransform =
        playerEntity->getComponent<TransformComponent>();
    ColliderComponent& playerCollider =
        playerEntity->getComponent<ColliderComponent>();
    Vector2D originalPosition = playerTransform.position;

    Vector2D playerCenter =
        playerTransform.position +
        Vector2D(playerTransform.width * playerTransform.scale / 2.0f,
                 playerTransform.height * playerTransform.scale / 2.0f);
    Vector2D bossCenter = transform->position +
                          Vector2D(transform->width * transform->scale / 2.0f,
                                   transform->height * transform->scale / 2.0f);
    Vector2D knockbackDir = playerCenter - bossCenter;

    if (knockbackDir.x == 0.0f && knockbackDir.y == 0.0f) {
        knockbackDir.y = -1.0f;
    }
    knockbackDir = knockbackDir.Normalize();

    playerTransform.position += knockbackDir * knockbackForce;

    playerCollider.update();
    SDL_Rect knockedBackPlayerRect = playerCollider.collider;
    bool collisionDetected = false;

    if (Game::instance) {
        for (auto* terrainEntity :
             Game::instance->manager.getGroup(Game::groupColliders)) {
            if (terrainEntity && terrainEntity->isActive() &&
                terrainEntity->hasComponent<ColliderComponent>()) {
                ColliderComponent& terrainColliderComp =
                    terrainEntity->getComponent<ColliderComponent>();
                if (terrainColliderComp.tag == "terrain") {
                    if (Collision::AABB(knockedBackPlayerRect,
                                        terrainColliderComp.collider)) {
                        collisionDetected = true;
                        break;
                    }
                }
            }
        }
    } else {
        std::cerr << "Warning: Game::instance is null in "
                     "BossAIComponent::applyKnockback. Cannot check terrain "
                     "collision."
                  << std::endl;
    }

    if (collisionDetected) {
        playerTransform.position = originalPosition;
        playerCollider.update();
    }
}

void BossAIComponent::updateAttackScaling() {
    if (!Game::instance || !Game::instance->playerManager) return;

    int playerLevel = Game::instance->playerManager->getLevel();
    int levelBonus = playerLevel / 10;

    currentProjectileCount = std::max(1, baseProjectileCount + levelBonus);
    currentBurstCount = std::max(1, baseBurstCount + levelBonus);

    float damageScaleFactor = std::pow(1.08f, playerLevel / 2);

    currentSlamDamage = std::max(1, static_cast<int>(baseSlamDamage * damageScaleFactor));
    currentProjectileDamage = std::max(1, static_cast<int>(baseProjectileDamage * damageScaleFactor));

}