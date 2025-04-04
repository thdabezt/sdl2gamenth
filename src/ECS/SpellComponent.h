#pragma once

// --- Includes ---
#include "ECS.h" // Includes Component base class
#include "../Vector2D.h"
#include <string>
#include <vector>   // Often needed for ECS headers
#include <cmath>    // For math constants/functions if used inline (like PI)
#include <SDL_stdinc.h> // For Uint32

// --- Forward Declarations ---
class TransformComponent;
class SoundComponent;

// --- Enums ---
enum class SpellTrajectory {
    RANDOM_DIRECTION,
    SPIRAL
    // Add other trajectories here if needed
};

// --- Class Definition ---

class SpellComponent : public Component {
private:
    // --- Private Members ---
    bool initialized = false;     // Initialization flag
    int level = 0;                // Current upgrade level of the spell
    int burstShotsRemaining = 0; // Shots left in the current burst
    Uint32 nextBurstShotTime = 0; // Time the next shot in burst can fire

    // Pointers to other components (set in init)
    TransformComponent* transform = nullptr;
    SoundComponent* sound = nullptr;

    // --- Private Methods ---
    // Internal helper to create projectile entities via AssetManager
    void createProjectile(Vector2D position, Vector2D velocity);
    // Internal helper to cast a single projectile (used by burst logic)
    void castSingleProjectile();
    // Internal helper for casting all projectiles at once (e.g., for non-burst modes)
    void castSpellFull(); // Renamed original cast method


public:
    // --- Public Members (Spell Properties) ---
    std::string tag;             // Identifier (e.g., "fire_vortex", "starfall")
    int damage;                  // Damage per projectile tick/hit
    int cooldown;                // Milliseconds between casts/burst starts
    float projectileSpeed;
    int projectilesPerCast;    // Number of projectiles created per cast action/burst sequence
    int projectileSize;
    std::string projectileTexture; // Asset ID for the projectile's texture
    int duration;                // Duration of the projectile/effect (if applicable)
    SpellTrajectory trajectoryMode; // How the projectile(s) move
    float spiralGrowthRate;      // Rate at which spiral radius increases (if SPIRAL mode)
    int projectilePierce = 1;    // How many enemies a projectile can pass through
    int burstDelay = 75;         // Milliseconds between shots within a burst

    // State Variables
    Uint32 lastCastTime = 0;       // Timestamp of the last cast/burst start
    float spiralAngle = 0.0f;      // Current angle for SPIRAL trajectory

    // --- Constructor ---
    // Creates a spell component with specified properties.
    SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, SpellTrajectory mode,
                   float growthRate = 5.0f, int pierce = 1); // Default args for growth/pierce

    // --- Public Methods ---

    // Component Lifecycle Overrides (Declarations only)
    void init() override;
    void update() override;
    // void draw() override; // No draw method defined for SpellComponent currently

    // Core Spell Action (Declaration only)
    void castSpell(); // This might trigger a burst or a full cast depending on logic in update

    // Property Modifiers (Declarations only)
    void increaseDamage(int amount);
    void decreaseCooldown(int amount); // Reduces cooldown time by fixed amount
    void decreaseCooldownPercentage(float percent); // Reduces cooldown time by percentage
    void increaseProjectileSpeed(float amount);
    void increaseProjectileCount(int amount); // Increases projectiles per cast/burst
    void increaseProjectileSize(int amount);
    void increasePierce(int amount);


    // Getters (Declarations only)
    std::string getTag() const;
    int getDamage() const;
    int getCooldown() const;
    float getProjectileSpeed() const;
    int getProjectileSize() const;
    int getPierce() const;
    int getProjectileCount() const;
    int getDuration() const; // Add if duration is used

    // Level Management (Declarations only)
    int getLevel() const;
    void setLevel(int newLevel); // Setter mainly for loading state
    void incrementLevel();       // Call when spell is upgraded

}; // End SpellComponent class