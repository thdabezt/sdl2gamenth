#pragma once

#include "ECS.h"
#include "../Vector2D.h"
#include <SDL.h>
#include <string>
#include <cstdlib> // For rand()

// Forward declarations
class TransformComponent;
class SpriteComponent;
class ColliderComponent;
class HealthComponent;
class SoundComponent;
class Game;

class BossAIComponent : public Component {
public:
    enum class BossState {
        WALKING,
        PRE_CHARGE,
        CHARGING,
        SHOOTING_BURST,
        PROJECTILE_COOLDOWN,
        PRE_SLAM, // Can likely be removed if slam is purely contact-based
        SLAMMING
        // SLAM_COOLDOWN // <<< REMOVE THIS STATE
    };

private:
    // ... (other private members: transform, sprite, playerEntity, currentState, thresholds, damage, counts, timers etc.) ...
    TransformComponent* transform = nullptr;
    SpriteComponent* sprite = nullptr;
    ColliderComponent* collider = nullptr;
    Entity* playerEntity = nullptr;
    BossState currentState = BossState::WALKING;
    float approachDistanceThreshold = 100.0f;
    float contactDistanceThreshold = 90.0f;
    float speed;
    int slamDamage;
    int projectileDamage;
    float knockbackForce;
    Uint32 projectileAttackTimer = 0;
    Uint32 projectileAttackInterval = 5000;
    int baseProjectileCount = 5;
    int baseBurstCount = 2;
    int currentProjectileCount = 5;
    int currentBurstCount = 2;
    int burstShotsRemaining = 0;
    Uint32 nextBurstShotTime = 0;
    Uint32 burstDelay = 100;
    Uint32 stateTimer = 0;
    Uint32 preChargeDuration = 1000;
    Uint32 chargeDuration = 300;
    Uint32 projectileCooldownDuration = 1000;
    Uint32 slamDuration = 800;
    // Uint32 slamCooldownDuration = 1500; // <<< REMOVE THIS DURATION
    bool initialized = false;

    // --- Cooldown Timer for Slam ---
    Uint32 slamCooldownEndTime = 0; // Stores the SDL_Tick when cooldown ends
    // ---

public:
    // Constructor remains the same
    BossAIComponent(float moveSpeed, float approachDist, float contactDist, int slamDmg, int projDmg, float knockback, Entity* playerEnt)
        : playerEntity(playerEnt),
          approachDistanceThreshold(approachDist),
          contactDistanceThreshold(contactDist),
          speed(moveSpeed),
          slamDamage(slamDmg),
          projectileDamage(projDmg),
          knockbackForce(knockback)
          {}

    void init() override;
    void update() override;

private:
    void updateWalking();
    void updatePreCharge();
    void updateCharging();
    void updateShootingBurst();
    void updateProjectileCooldown();
    void updatePreSlam(); // Can potentially remove this function
    void updateSlamming();
    // void updateSlamCooldown(); // <<< REMOVE THIS FUNCTION DECLARATION

    void shootProjectile();
    void shootSingleBurstProjectile();
    void performSlam();
    void applyKnockback();
    void changeState(BossState newState);
    void updateAttackScaling();
};