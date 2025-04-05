#include <cmath>     
#include <cstdlib>   
#include <iostream>  

#include "../AssetManager.h"
#include "../game.h"     
#include "Components.h"  

SpellComponent::SpellComponent(std::string spellTag, int dmg, int cool,
                               float speed, int count, int size,
                               std::string texId, SpellTrajectory mode,
                               float growthRate, int pierce)
    : tag(std::move(spellTag)),
      damage(dmg),
      cooldown(cool),
      projectileSpeed(speed),
      projectilesPerCast(count),
      projectileSize(size),
      projectileTexture(std::move(texId)),
      trajectoryMode(mode),
      spiralGrowthRate(growthRate),
      projectilePierce(pierce > 0 ? pierce : 1)  
{

}

void SpellComponent::init() {
    initialized = false;  
    if (!entity) {
        std::cerr << "Error in SpellComponent::init: Entity is null!"
                  << std::endl;
        return;
    }

    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr << "Error in SpellComponent::init (" << tag
                  << "): Entity missing TransformComponent!" << std::endl;
        return;
    }
    transform = &entity->getComponent<TransformComponent>();
    if (!transform) {
        std::cerr << "Error in SpellComponent::init (" << tag
                  << "): Failed to get TransformComponent pointer!"
                  << std::endl;
        return;
    }

    if (entity->hasComponent<SoundComponent>()) {
        sound = &entity->getComponent<SoundComponent>();
    } else {
        sound = nullptr;
    }

    lastCastTime = SDL_GetTicks();  
    initialized = true;
}

void SpellComponent::update() {
    if (!initialized || !transform)
        return;  

    Uint32 currentTime = SDL_GetTicks();

    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            castSingleProjectile();  
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) {  
                nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }

    else if (currentTime > lastCastTime + cooldown) {

        bool useBurst =
            (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION ||
             trajectoryMode == SpellTrajectory::SPIRAL);  

        if (useBurst) {
            burstShotsRemaining =
                projectilesPerCast;  
            nextBurstShotTime =
                currentTime;  
            lastCastTime = currentTime;  

            if (sound && projectilesPerCast > 0) {
                if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
                    sound->playSoundEffect("star_cast");
                } else if (trajectoryMode == SpellTrajectory::SPIRAL) {
                    sound->playSoundEffect("fire_cast");
                }

            }

            if (burstShotsRemaining > 0) {
                castSingleProjectile();
                burstShotsRemaining--;
                if (burstShotsRemaining >
                    0) {  
                    nextBurstShotTime = currentTime + burstDelay;
                }
            }
        } else {

            lastCastTime = currentTime;

        }
    }

    if (trajectoryMode == SpellTrajectory::SPIRAL) {
        spiralAngle += 0.1f;  
        const double PI = acos(-1.0);
        if (spiralAngle > 2.0 * PI) {
            spiralAngle -= 2.0 * PI;
        }
    }
}

void SpellComponent::castSpell() {

    if (trajectoryMode != SpellTrajectory::RANDOM_DIRECTION &&
        trajectoryMode != SpellTrajectory::SPIRAL) {

    }

}

void SpellComponent::castSingleProjectile() {
    if (!initialized || !transform) return;  

    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;

    const double PI = acos(-1.0);
    Vector2D velocity;

    if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
        float randomAngle = static_cast<float>(rand()) /
                            (static_cast<float>(RAND_MAX / (2.0 * PI)));
        velocity.x = std::cos(randomAngle) * projectileSpeed;
        velocity.y = std::sin(randomAngle) * projectileSpeed;
        createProjectile(spawnCenter, velocity);
    } else if (trajectoryMode == SpellTrajectory::SPIRAL) {
        float currentRadius = spiralAngle * spiralGrowthRate;
        Vector2D spiralSpawnPosition;
        spiralSpawnPosition.x =
            spawnCenter.x + std::cos(spiralAngle) * currentRadius;
        spiralSpawnPosition.y =
            spawnCenter.y + std::sin(spiralAngle) * currentRadius;

        velocity.x = std::cos(spiralAngle) * projectileSpeed;
        velocity.y = std::sin(spiralAngle) * projectileSpeed;

        createProjectile(spiralSpawnPosition, velocity);

        spiralAngle += 0.1f;  
        if (spiralAngle > 2.0 * PI) {
            spiralAngle -= 2.0 * PI;
        }
    }
}

void SpellComponent::castSpellFull() {
    if (!initialized || !transform) return;

    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;

    const double PI = acos(-1.0);

    switch (trajectoryMode) {

        default:  
            for (int i = 0; i < projectilesPerCast; ++i) {

                float randomAngle = static_cast<float>(rand()) /
                                    (static_cast<float>(RAND_MAX / (2.0 * PI)));
                Vector2D defaultVel;
                defaultVel.x = std::cos(randomAngle) * projectileSpeed;
                defaultVel.y = std::sin(randomAngle) * projectileSpeed;
                createProjectile(spawnCenter, defaultVel);
            }
            break;
    }
}

void SpellComponent::createProjectile(Vector2D position, Vector2D velocity) {
    if (Game::instance && Game::instance->assets) {
        Game::instance->assets->CreateProjectile(
            position, velocity, damage, projectileSize, projectileTexture,
            projectilePierce);
    } else {
        std::cerr << "Error in SpellComponent::createProjectile: Game instance "
                     "or assets is null!"
                  << std::endl;
    }
}

void SpellComponent::increaseDamage(int amount) {
    damage = std::max(0, damage + amount);
}  
void SpellComponent::decreaseCooldown(int amount) {
    cooldown = std::max(50, cooldown - amount);
}  
void SpellComponent::increaseProjectileSpeed(float amount) {
    projectileSpeed += amount;
}
void SpellComponent::increaseProjectileCount(int amount) {
    projectilesPerCast = std::max(1, projectilesPerCast + amount);
}  
void SpellComponent::increaseProjectileSize(int amount) {
    projectileSize = std::max(1, projectileSize + amount);
}  
void SpellComponent::increasePierce(int amount) { projectilePierce += amount; }

void SpellComponent::decreaseCooldownPercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    cooldown = static_cast<int>(cooldown * multiplier);
    cooldown =
        std::max(50, cooldown);  
}

std::string SpellComponent::getTag() const { return tag; }
int SpellComponent::getDamage() const { return damage; }
int SpellComponent::getCooldown() const { return cooldown; }
float SpellComponent::getProjectileSpeed() const { return projectileSpeed; }
int SpellComponent::getProjectileSize() const { return projectileSize; }
int SpellComponent::getPierce() const { return projectilePierce; }
int SpellComponent::getProjectileCount() const { return projectilesPerCast; }
int SpellComponent::getDuration() const { return duration; }

int SpellComponent::getLevel() const { return level; }
void SpellComponent::setLevel(int newLevel) {
    level = std::max(0, newLevel);
}  
void SpellComponent::incrementLevel() { level++; }