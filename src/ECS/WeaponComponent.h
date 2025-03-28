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
    int damage;
    int fireRate;          // Milliseconds between shots
    float projectileSpeed;
    int projectileRange;
    float spreadAngle;     // In radians
    int projectilesPerShot;
    std::string projectileTexture;
    
  
    Uint32 lastShotTime = 0;
    TransformComponent* transform;
    ColliderComponent* collider;
    
    WeaponComponent(int dmg, int rate, float speed, int range, float spread, 
                   int count, std::string texId) 
        : damage(dmg), 
          fireRate(rate), 
          projectileSpeed(speed), 
          projectileRange(range), 
          spreadAngle(spread), 
          projectilesPerShot(count), 
          projectileTexture(texId) {}
          
    void init() override;
    void update() override;
    void shoot();
    
private:
    // Helper function to create projectile without directly calling AssetManager
    void createProjectile(Vector2D position, Vector2D velocity);
};