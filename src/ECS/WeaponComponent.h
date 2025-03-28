#pragma once

#include "ECS.h"
#include <string>
#include <math.h>
#include <SDL.h>
#include "../Vector2D.h"

class TransformComponent;
class ColliderComponent;

class WeaponComponent : public Component {
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
        int shotsPerBurst = 1;  // <<< ADDED: Number of shots in one burst (default 1)
        int burstDelay = 50;    // <<< ADDED: Milliseconds between shots WITHIN a burst
    
        // --- State Variables ---
        Uint32 lastShotTime = 0;      // Time the last BURST started
        // --- Burst State ---
        int burstShotsRemaining = 0;  // <<< ADDED: Shots left in the current burst
        Uint32 nextBurstShotTime = 0; // <<< ADDED: Time the next shot in the burst can fire
    
        TransformComponent* transform = nullptr; // Use forward declaration
        ColliderComponent* collider = nullptr; // Use forward declaration
    
        // --- Update Constructor ---
        WeaponComponent(std::string weaponTag, int dmg, int rate, float speed, int range, float spread,
                       int count, int size, std::string texId, int pierce = 1,
                       int burstCount = 1, int burst_Delay = 50) // Add burstCount, burst_Delay params
            : tag(weaponTag),
              damage(dmg),
              fireRate(rate), // Cooldown between bursts
              projectileSpeed(speed),
              projectileRange(range),
              spreadAngle(spread),
              projectilesPerShot(count), // Simultaneous projectiles
              projectileSize(size),
              projectileTexture(texId),
              projectilePierce(pierce),
              shotsPerBurst(burstCount > 0 ? burstCount : 1), // Ensure at least 1 shot
              burstDelay(burst_Delay) // Delay within burst
              {
                  // Initialize state
                  burstShotsRemaining = 0;
                  nextBurstShotTime = 0;
                  lastShotTime = 0; // Init will set this properly
              }
    
        // --- Method Declarations ---
        void init() override;
        void update() override;
        void shoot(); // shoot() now fires one volley of projectilesPerShot
    
    // Methods to update weapon properties
    void increaseDamage(int amount) { damage += amount; }
    void decreaseFireRate(int amount) { fireRate = std::max(50, fireRate - amount); } // Faster fire rate
    void increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
    void increaseRange(int amount) { projectileRange += amount; }
    void increaseSpread(float amount) { spreadAngle += amount; }
    void increaseProjectileCount(int amount) { projectilesPerShot += amount; }
    void increaseProjectileSize(int amount) { projectileSize += amount; }
    void increasePierce(int amount) { projectilePierce += amount; }
    void increaseBurstCount(int amount) { shotsPerBurst += amount; }

    
    // Get properties (for UI display)
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
    // Helper function to create projectile without directly calling AssetManager
    void createProjectile(Vector2D position, Vector2D velocity);
};