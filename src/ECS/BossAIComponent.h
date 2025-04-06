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

    int baseProjectileCount = 5;
    int baseBurstCount = 2;
    int currentProjectileCount = 5;
    int currentBurstCount = 2;
    int baseSlamDamage;       
    int baseProjectileDamage; 
    int currentSlamDamage;    
    int currentProjectileDamage; 
    Uint32 projectileAttackInterval = 5000;
    Uint32 burstDelay = 100;

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