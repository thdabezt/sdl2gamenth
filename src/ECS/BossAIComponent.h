#pragma once

// --- Includes ---
#include "ECS.h"
#include "../Vector2D.h" // Not strictly needed here, but potentially in .cpp
#include <SDL_stdinc.h>  // For Uint32
#include <string>        // Included via ECS.h, but good practice
#include <cstdlib>       // For rand() if used inline (should be in .cpp)

// --- Forward Declarations ---
class TransformComponent;
class SpriteComponent;
class ColliderComponent;

// --- Enums ---
enum class BossState {
    WALKING,            // Moving towards the player
    PRE_CHARGE,         // Short animation before shooting starts
    CHARGING,           // Animation while projectiles are being prepared/aimed (if needed)
    SHOOTING_BURST,     // Actively firing projectiles in a burst
    PROJECTILE_COOLDOWN,// Short pause after shooting before resuming walking
    PRE_SLAM,           // Short animation before slam attack
    SLAMMING            // Slam attack animation and damage application
};

// --- Class Definition ---

// Component managing the AI behavior for a boss enemy.
class BossAIComponent : public Component {
private:
    // --- Private Members ---

    // Component Pointers (set in init)
    TransformComponent* transform = nullptr;
    SpriteComponent* sprite = nullptr;
    ColliderComponent* collider = nullptr;
    Entity* playerEntity = nullptr; // Pointer to the target player entity

    // AI State & Timers
    BossState currentState = BossState::WALKING;
    Uint32 stateTimer = 0;              // Timestamp when the current state began
    Uint32 projectileAttackTimer = 0;   // Timestamp for the next projectile attack sequence
    Uint32 nextBurstShotTime = 0;       // Timestamp for the next shot within a burst
    Uint32 slamCooldownEndTime = 0;     // Timestamp when the slam attack becomes available again

    // AI Parameters & Configuration
    float speed;                        // Base movement speed
    float approachDistanceThreshold;    // Distance at which the boss might start specific actions (unused currently)
    float contactDistanceThreshold;     // Distance within which slam/contact damage occurs
    int slamDamage;
    int projectileDamage;
    float knockbackForce;               // Force applied to player on slam hit

    // Projectile Attack Config
    int baseProjectileCount = 5;        // Base number of projectiles per spread shot
    int baseBurstCount = 2;             // Base number of shots in a burst sequence
    int currentProjectileCount = 5;     // Current (scaled) number of projectiles per shot
    int currentBurstCount = 2;          // Current (scaled) number of shots per burst
    Uint32 projectileAttackInterval = 5000; // Time (ms) between start of projectile attack sequences
    Uint32 burstDelay = 100;            // Time (ms) between shots within a burst

    // State Durations (ms)
    Uint32 preChargeDuration = 1000;
    Uint32 chargeDuration = 300;
    Uint32 projectileCooldownDuration = 1000;
    Uint32 slamDuration = 800;          // Duration of the slam animation/effect

    // Internal State
    int burstShotsRemaining = 0;        // Shots left in the current burst
    bool initialized = false;           // Initialization flag

    // --- Private Methods (Declarations only) ---
    // State-specific update logic
    void updateWalking();
    void updatePreCharge();
    void updateCharging();
    void updateShootingBurst();
    void updateProjectileCooldown();
    void updatePreSlam();
    void updateSlamming();

    // Action helpers
    void shootProjectile();             // Initiates the projectile burst sequence
    void shootSingleBurstProjectile();  // Shoots one projectile as part of a burst
    void performSlam();                 // Applies slam damage and effects
    void applyKnockback();              // Applies knockback force to the player

    // State management
    void changeState(BossState newState); // Handles transitions between states
    void updateAttackScaling();         // Adjusts attack parameters based on player level

public:
    // --- Constructor ---
    // Initializes the AI parameters. Requires a pointer to the player entity.
    BossAIComponent(float moveSpeed, float approachDist, float contactDist,
                    int slamDmg, int projDmg, float knockback, Entity* playerEnt);

    // --- Public Methods ---

    // Component Lifecycle Overrides (Declarations only)
    void init() override;
    void update() override;
    // void draw() override; // No draw logic defined for AI component itself

}; // End BossAIComponent class