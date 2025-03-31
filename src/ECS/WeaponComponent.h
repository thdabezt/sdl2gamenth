#pragma once

#include "ECS.h"
#include <string>
#include <cmath>
#include <SDL.h>
#include "../Vector2D.h"
#include <iostream> // For logging

// Forward declarations
class TransformComponent;
class ColliderComponent;
class SoundComponent;

class WeaponComponent : public Component {
private:
    bool initialized = false; // <<< ADD Initialization Flag

public:
    // --- Weapon properties ---
    std::string tag;
    int damage;
    int fireRate;           // Milliseconds BETWEEN BURSTS
    float projectileSpeed;
    int projectileRange;
    float spreadAngle;
    int projectilesPerShot; // Projectiles fired simultaneously per shot
    int projectileSize;
    std::string projectileTexture;
    int projectilePierce = 1;
    // --- Burst Properties ---
    int shotsPerBurst = 1;
    int burstDelay = 50;

    // --- State Variables ---
    Uint32 lastShotTime = 0;
    int burstShotsRemaining = 0;
    Uint32 nextBurstShotTime = 0;

    // --- Component Pointers (Initialize to nullptr) ---
    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    SoundComponent* sound = nullptr;


    WeaponComponent(std::string weaponTag, int dmg, int rate, float speed, int range, float spread,
                   int count, int size, std::string texId, int pierce = 1,
                   int burstCount = 1, int burst_Delay = 50)
        : tag(std::move(weaponTag)), damage(dmg), fireRate(rate), projectileSpeed(speed),
          projectileRange(range), spreadAngle(spread), projectilesPerShot(count),
          projectileSize(size), projectileTexture(std::move(texId)), projectilePierce(pierce),
          shotsPerBurst(burstCount > 0 ? burstCount : 1), burstDelay(burst_Delay) {}

    // --- Method Declarations ---
    // Init/Update declarations only, definitions in .cpp
    void init() override;
    void update() override;
    void shoot();

    // Methods to update weapon properties
    void increaseDamage(int amount) { damage += amount; }
    void decreaseFireRate(int amount) { fireRate = std::max(50, fireRate - amount); }
    void increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
    void increaseRange(int amount) { projectileRange += amount; }
    void increaseSpread(float amount) { spreadAngle += amount; }
    void increaseProjectileCount(int amount) { projectilesPerShot += amount; }
    void increaseProjectileSize(int amount) { projectileSize += amount; }
    void increasePierce(int amount) { projectilePierce += amount; }
    void increaseBurstCount(int amount) { shotsPerBurst += amount; }


    // Get properties
    std::string getTag() const { return tag; }
    int getDamage() const { return damage; }
    int getFireRate() const { return fireRate; }
    float getProjectileSpeed() const { return projectileSpeed; }
    int getRange() const { return projectileRange; }
    float getSpreadAngle() const { return spreadAngle; }
    int getProjectileCount() const { return projectilesPerShot; }
    int getProjectileSize() const { return projectileSize; }
    int getPierce() const { return projectilePierce; }
    int getBurstCount() const { return shotsPerBurst; }

private:
    void createProjectile(Vector2D position, Vector2D velocity);
};