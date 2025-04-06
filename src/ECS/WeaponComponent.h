#pragma once

#include <SDL_stdinc.h>

#include <cmath>
#include <iostream>
#include <string>

#include "../Vector2D.h"
#include "ECS.h"

class TransformComponent;
class ColliderComponent;
class SoundComponent;

class WeaponComponent : public Component {
   private:
    bool initialized = false;
    int level = 0;

    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    SoundComponent* sound = nullptr;

    void createProjectile(Vector2D position, Vector2D velocity);

   public:
    std::string tag;
    int damage;
    int fireRate;
    float projectileSpeed;
    float spreadAngle;
    int projectilesPerShot;
    int projectileSize;
    std::string projectileTexture;
    int projectilePierce = 1;

    int shotsPerBurst = 1;
    int burstDelay = 50;

    Uint32 lastShotTime = 0;
    int burstShotsRemaining = 0;
    Uint32 nextBurstShotTime = 0;

    WeaponComponent(std::string weaponTag, int dmg, int rate, float speed,
                    float spread, int count, int size, std::string texId,
                    int pierce = 1, int burstCount = 1, int burst_Delay = 50);

    void init() override;
    void update() override;

    void shoot();

    void increaseDamage(int amount);
    void decreaseFireRate(int amount);
    void decreaseFireRatePercentage(float percent);
    void increaseProjectileSpeed(float amount);
    void increaseSpread(float amount);
    void increaseProjectileCount(int amount);
    void increaseProjectileSize(int amount);
    void increasePierce(int amount);
    void increaseBurstCount(int amount);

    std::string getTag() const;
    int getDamage() const;
    int getFireRate() const;
    float getProjectileSpeed() const;
    float getSpreadAngle() const;
    int getProjectileCount() const;
    int getProjectileSize() const;
    int getPierce() const;
    int getBurstCount() const;

    int getLevel() const;
    void setLevel(int newLevel);
    void incrementLevel();
};