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
    // Weapon properties
    std::string tag;        // Weapon identifier
    int damage;
    int fireRate;          // Milliseconds between shots
    float projectileSpeed;
    int projectileRange;
    float spreadAngle;     // In radians
    int projectilesPerShot;
    int projectileSize;    // Size of projectile (width/height)
    std::string projectileTexture;
    
    Uint32 lastShotTime = 0;
    TransformComponent* transform;
    ColliderComponent* collider;
    
    WeaponComponent(std::string weaponTag, int dmg, int rate, float speed, int range, float spread, 
                   int count, int size, std::string texId) 
        : tag(weaponTag),
          damage(dmg), 
          fireRate(rate), 
          projectileSpeed(speed), 
          projectileRange(range), 
          spreadAngle(spread), 
          projectilesPerShot(count), 
          projectileSize(size),
          projectileTexture(texId) {}
          
    void init() override;
    void update() override;
    void shoot();
    
    // Methods to update weapon properties
    void increaseDamage(int amount) { damage += amount; }
    void decreaseFireRate(int amount) { fireRate = std::max(50, fireRate - amount); } // Faster fire rate
    void increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
    void increaseRange(int amount) { projectileRange += amount; }
    void increaseSpread(float amount) { spreadAngle += amount; }
    void increaseProjectileCount(int amount) { projectilesPerShot += amount; }
    void increaseProjectileSize(int amount) { projectileSize += amount; }
    
    // Enemy defeated upgrade handler
    void onEnemyDefeated();
    
    // Get properties (for UI display)
    std::string getTag() const { return tag; }
    int getDamage() const { return damage; }
    int getFireRate() const { return fireRate; }
    float getProjectileSpeed() const { return projectileSpeed; }
    int getRange() const { return projectileRange; }
    float getSpreadAngle() const { return spreadAngle; }
    int getProjectileCount() const { return projectilesPerShot; }
    int getProjectileSize() const { return projectileSize; }
    
private:
    // Helper function to create projectile without directly calling AssetManager
    void createProjectile(Vector2D position, Vector2D velocity);
};