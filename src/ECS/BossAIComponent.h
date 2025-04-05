#pragma once

#include <SDL_stdinc.h>

#include <cstdlib>
#include <string>

#include "../Vector2D.h"
#include "ECS.h"

class TransformComponent;
class SpriteComponent;
class ColliderComponent;

enum class BossState {
    WALKING,
    PRE_CHARGE,
    CHARGING,
    SHOOTING_BURST,
    PROJECTILE_COOLDOWN,
    PRE_SLAM,
    SLAMMING
};

class BossAIComponent : public Component {
   private:
    TransformComponent* transform = nullptr;
    SpriteComponent* sprite = nullptr;
    ColliderComponent* collider = nullptr;
    Entity* playerEntity = nullptr;

    BossState currentState = BossState::WALKING;
    Uint32 stateTimer = 0;
    Uint32 projectileAttackTimer = 0;
    Uint32 nextBurstShotTime = 0;
    Uint32 slamCooldownEndTime = 0;

    float speed;
    float approachDistanceThreshold;
    float contactDistanceThreshold;
    int slamDamage;
    int projectileDamage;
    float knockbackForce;

<<<<<<< HEAD
    int baseProjectileCount = 5;
    int baseBurstCount = 2;
    int currentProjectileCount = 5;
    int currentBurstCount = 2;
    Uint32 projectileAttackInterval = 5000;
    Uint32 burstDelay = 100;
=======

    // Projectile Attack Config
    int baseProjectileCount = 5;        // Base number of projectiles per spread shot
    int baseBurstCount = 2;             // Base number of shots in a burst sequence
    int currentProjectileCount = 5;     // Current (scaled) number of projectiles per shot
    int currentBurstCount = 2;          // Current (scaled) number of shots per burst
    Uint32 projectileAttackInterval = 5000; // Time (ms) between start of projectile attack sequences
    Uint32 burstDelay = 100;            // Time (ms) between shots within a burst
>>>>>>> 48aebd591664aaebcc837f2de6b6a7394e56c0f2

    Uint32 preChargeDuration = 1000;
    Uint32 chargeDuration = 300;
    Uint32 projectileCooldownDuration = 1000;
    Uint32 slamDuration = 800;

    int burstShotsRemaining = 0;
    bool initialized = false;

    void updateWalking();
    void updatePreCharge();
    void updateCharging();
    void updateShootingBurst();
    void updateProjectileCooldown();
    void updatePreSlam();
    void updateSlamming();

    void shootProjectile();
    void shootSingleBurstProjectile();
    void performSlam();
    void applyKnockback();

    void changeState(BossState newState);
    void updateAttackScaling();

   public:
    BossAIComponent(float moveSpeed, float approachDist, float contactDist,
                    int slamDmg, int projDmg, float knockback,
                    Entity* playerEnt);

    void init() override;
    void update() override;
};