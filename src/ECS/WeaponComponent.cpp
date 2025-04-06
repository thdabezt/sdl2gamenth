#include <algorithm>  
#include <cmath>      
#include <iostream>   

#include "../AssetManager.h"
#include "../game.h"     
#include "Components.h"  

WeaponComponent::WeaponComponent(std::string weaponTag, int dmg, int rate,
                                 float speed, float spread, int count, int size,
                                 std::string texId, int pierce, int burstCount,
                                 int burst_Delay)
    : tag(std::move(weaponTag)),
      damage(dmg),
      fireRate(rate),
      projectileSpeed(speed),
      spreadAngle(spread),
      projectilesPerShot(count),
      projectileSize(size),
      projectileTexture(std::move(texId)),
      projectilePierce(pierce > 0 ? pierce : 1),  
      shotsPerBurst(burstCount > 0 ? burstCount
                                   : 1),  
      burstDelay(burst_Delay) {

}

void WeaponComponent::init() {
    initialized = false;  
    if (!entity) {
        std::cerr << "Error in WeaponComponent::init: Entity is null!"
                  << std::endl;
        return;
    }

    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr
            << "Error in WeaponComponent::init: Missing TransformComponent!"
            << std::endl;
        return;
    }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<ColliderComponent>()) {
        std::cerr
            << "Error in WeaponComponent::init: Missing ColliderComponent!"
            << std::endl;
        return;
    }
    collider = &entity->getComponent<ColliderComponent>();

    if (entity->hasComponent<SoundComponent>()) {
        sound = &entity->getComponent<SoundComponent>();
    } else {
        sound = nullptr;
    }

    if (!transform) {
        std::cerr << "Error in WeaponComponent::init: Failed to get "
                     "TransformComponent!"
                  << std::endl;
        return;
    }
    if (!collider) {
        std::cerr << "Error in WeaponComponent::init: Failed to get "
                     "ColliderComponent!"
                  << std::endl;
        return;
    }

    lastShotTime = SDL_GetTicks();  
    burstShotsRemaining = 0;

    initialized = true;
}

void WeaponComponent::update() {
    if (!initialized || !transform || !collider)
        return;  

    Uint32 currentTime = SDL_GetTicks();

    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            shoot();  
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) {  
                nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }

    else if (currentTime > lastShotTime + fireRate) {
        burstShotsRemaining = shotsPerBurst;  
        lastShotTime = currentTime;           

        if (burstShotsRemaining > 0) {
            shoot();
            burstShotsRemaining--;
            if (burstShotsRemaining >
                0) {  
                nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }

}

void WeaponComponent::shoot() {

    if (!initialized || !transform || !collider) return;

    if (sound) {
        sound->playSoundEffect("shoot");  

    }

    Vector2D projectilePosition;
    projectilePosition.x =
        static_cast<float>(collider->collider.x) + collider->collider.w / 2.0f;
    projectilePosition.y =
        static_cast<float>(collider->collider.y) + collider->collider.h / 2.0f;

    Vector2D targetPosition(static_cast<float>(Game::mouseX),
                            static_cast<float>(Game::mouseY));
    Vector2D direction = targetPosition - projectilePosition;

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction.y = -1.0f;
    }  
    else {
        direction = direction.Normalize();
    }

    Vector2D baseVelocity = direction * projectileSpeed;

    if (projectilesPerShot <= 1) {

        createProjectile(projectilePosition, baseVelocity);
    } else {

        const double PI = acos(-1.0);  
        float spreadRad =
            spreadAngle *
            (PI / 180.0f);  
        float halfSpread = spreadRad / 2.0f;
        float angleStep = (projectilesPerShot > 1)
                              ? spreadRad / (projectilesPerShot - 1)
                              : 0.0f;

        for (int i = 0; i < projectilesPerShot; i++) {
            float currentAngle =
                -halfSpread +
                (i * angleStep);  

            Vector2D rotatedVelocity;
            float cosA = std::cos(currentAngle);
            float sinA = std::sin(currentAngle);
            rotatedVelocity.x = baseVelocity.x * cosA - baseVelocity.y * sinA;
            rotatedVelocity.y = baseVelocity.x * sinA + baseVelocity.y * cosA;

            createProjectile(projectilePosition, rotatedVelocity);
        }
    }
}

void WeaponComponent::increaseDamage(int amount) { damage += amount; }
void WeaponComponent::decreaseFireRate(int amount) {
    fireRate = std::max(10, fireRate - amount);
}  
void WeaponComponent::increaseProjectileSpeed(float amount) {
    projectileSpeed += amount;
}
void WeaponComponent::increaseSpread(float amount) { spreadAngle += amount; }
void WeaponComponent::increaseProjectileCount(int amount) {
    projectilesPerShot += amount;
}
void WeaponComponent::increaseProjectileSize(int amount) {
    projectileSize += amount;
}
void WeaponComponent::increasePierce(int amount) { projectilePierce += amount; }
void WeaponComponent::increaseBurstCount(int amount) {
    shotsPerBurst = std::max(1, shotsPerBurst + amount);
}  

void WeaponComponent::decreaseFireRatePercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    fireRate = static_cast<int>(fireRate * multiplier);
    fireRate = std::max(10, fireRate);  
}

std::string WeaponComponent::getTag() const { return tag; }
int WeaponComponent::getDamage() const { return damage; }
int WeaponComponent::getFireRate() const { return fireRate; }
float WeaponComponent::getProjectileSpeed() const { return projectileSpeed; }
float WeaponComponent::getSpreadAngle() const { return spreadAngle; }
int WeaponComponent::getProjectileCount() const { return projectilesPerShot; }
int WeaponComponent::getProjectileSize() const { return projectileSize; }
int WeaponComponent::getPierce() const { return projectilePierce; }
int WeaponComponent::getBurstCount() const { return shotsPerBurst; }

int WeaponComponent::getLevel() const { return level; }
void WeaponComponent::setLevel(int newLevel) {
    level = std::max(0, newLevel);
}  
void WeaponComponent::incrementLevel() { level++; }

void WeaponComponent::createProjectile(Vector2D position, Vector2D velocity) {
    if (Game::instance && Game::instance->assets) {

        Game::instance->assets->CreateProjectile(
            position, velocity, damage, projectileSize, projectileTexture,
            projectilePierce

        );
    } else {
        std::cerr << "Error in WeaponComponent::createProjectile: Game "
                     "instance or assets pointer is null!"
                  << std::endl;
    }
}