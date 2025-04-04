#pragma once

// --- Includes ---
#include "ECS.h"
#include "../Vector2D.h"
#include <string>
#include <cmath>
#include <SDL_stdinc.h> // For Uint32
#include <iostream> // Keep for potential logging in definitions

// --- Forward Declarations ---
class TransformComponent;
class ColliderComponent;
class SoundComponent;

// --- Class Definition ---

class WeaponComponent : public Component {
private:
    // --- Private Members ---
    bool initialized = false;
    int level = 0;

    // Component Pointers
    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    SoundComponent* sound = nullptr;

    // --- Private Methods ---
    void createProjectile(Vector2D position, Vector2D velocity); // Declaration only

public:
    // --- Public Members (Weapon Properties) ---
    std::string tag;
    int damage;
    int fireRate;
    float projectileSpeed;
    float spreadAngle;
    int projectilesPerShot;
    int projectileSize;
    std::string projectileTexture;
    int projectilePierce = 1;

    // Burst Fire Properties
    int shotsPerBurst = 1;
    int burstDelay = 50;

    // State Variables
    Uint32 lastShotTime = 0;
    int burstShotsRemaining = 0;
    Uint32 nextBurstShotTime = 0;

    // --- Constructor ---
    // Declaration only
    WeaponComponent(std::string weaponTag, int dmg, int rate, float speed, float spread,
                   int count, int size, std::string texId, int pierce = 1,
                   int burstCount = 1, int burst_Delay = 50);

    // --- Public Methods ---

    // Component Lifecycle Overrides (Declarations only)
    void init() override;
    void update() override;

    // Core Weapon Action (Declaration only)
    void shoot();

    // Property Modifiers (Declarations only)
    void increaseDamage(int amount);
    void decreaseFireRate(int amount);
    void decreaseFireRatePercentage(float percent); // Keep definition in .cpp
    void increaseProjectileSpeed(float amount);
    void increaseSpread(float amount);
    void increaseProjectileCount(int amount);
    void increaseProjectileSize(int amount);
    void increasePierce(int amount);
    void increaseBurstCount(int amount);

    // Getters (Declarations only)
    std::string getTag() const;
    int getDamage() const;
    int getFireRate() const;
    float getProjectileSpeed() const;
    float getSpreadAngle() const;
    int getProjectileCount() const;
    int getProjectileSize() const;
    int getPierce() const;
    int getBurstCount() const;

    // Level Management (Declarations only)
    int getLevel() const;
    void setLevel(int newLevel);
    void incrementLevel();

}; // End WeaponComponent class