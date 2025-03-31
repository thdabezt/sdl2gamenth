#include "Components.h" // Includes SpellComponent.h
#include "../AssetManager.h"
#include "../game.h"
#include <iostream>
// #define _USE_MATH_DEFINES // Not needed if using acos
#include <cmath>
#include <cstdlib>
#include <ctime>

SpellComponent::SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, int dur, SpellTrajectory mode,
                   float growthRate, int pierce)
        : tag(std::move(spellTag)), damage(dmg), cooldown(cool), projectileSpeed(speed),
          projectilesPerCast(count), projectileSize(size), projectileTexture(std::move(texId)),
          duration(dur), trajectoryMode(mode), spiralGrowthRate(growthRate),
          projectilePierce(pierce > 0 ? pierce : 1)
          {
              // Pointers initialized to nullptr in header
              // State variables initialized in header
              if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
                   // Seeding random here might cause issues if multiple spells are created quickly.
                   // Consider seeding once globally (e.g., in Game constructor or main).
                   // std::srand(static_cast<unsigned int>(std::time(nullptr)));
              }
          }

void SpellComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in SpellComponent::init: Entity is null!" << std::endl; return; }

    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in SpellComponent::init (" << tag << "): Entity missing TransformComponent!" << std::endl; return; }
    transform = &entity->getComponent<TransformComponent>();

    // Get SoundComponent (optional)
    if (entity->hasComponent<SoundComponent>()) { sound = &entity->getComponent<SoundComponent>(); }
    else { sound = nullptr; }

    if (!transform) { std::cerr << "Error in SpellComponent::init (" << tag << "): Failed to get TransformComponent pointer!" << std::endl; return; }

    lastCastTime = SDL_GetTicks();
    initialized = true; // <<< SET Flag on success
}

void SpellComponent::update() {
    if (!initialized) return; // <<< CHECK Flag
    // Keep internal checks
    if (!transform) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastCastTime + cooldown) {
        castSpell();
        lastCastTime = currentTime;
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
    if (!initialized) return; // <<< CHECK Flag
    // Keep internal checks
    if (!transform) return;

    if (sound) {
        if (this->tag == "spell") { sound->playSoundEffect("fire_cast"); }
        else if (this->tag == "star") { sound->playSoundEffect("star_cast"); }
    }

    Vector2D spawnCenter = transform->position;
    // Center spawn relative to transform size
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;

    const double PI = acos(-1.0);

    switch (trajectoryMode) {
        case SpellTrajectory::RANDOM_DIRECTION:
            for (int i = 0; i < projectilesPerCast; ++i) {
                float randomAngle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * PI)));
                Vector2D velocity;
                velocity.x = std::cos(randomAngle) * projectileSpeed;
                velocity.y = std::sin(randomAngle) * projectileSpeed;
                createProjectile(spawnCenter, velocity);
            }
            break;

        case SpellTrajectory::SPIRAL:
             if (projectilesPerCast > 0) {
                  float currentRadius = spiralAngle * spiralGrowthRate;
                  Vector2D spiralSpawnPosition;
                  spiralSpawnPosition.x = spawnCenter.x + std::cos(spiralAngle) * currentRadius;
                  spiralSpawnPosition.y = spawnCenter.y + std::sin(spiralAngle) * currentRadius;
                  Vector2D velocity;
                  velocity.x = std::cos(spiralAngle) * projectileSpeed;
                  velocity.y = std::sin(spiralAngle) * projectileSpeed;
                  for(int i = 0;i<projectilesPerCast;i++){
                      createProjectile(spiralSpawnPosition, velocity);
                      
                  }
             }
            break;
    }
}

void SpellComponent::createProjectile(Vector2D position, Vector2D velocity) {
    // if (!initialized) return; // Internal helper, castSpell checks initialized
     if (Game::instance && Game::instance->assets) {
         Game::instance->assets->CreateProjectile(position, velocity, duration, damage, projectileSize, projectileTexture, projectilePierce);
     } else {
          std::cerr << "Error in SpellComponent::createProjectile: Game instance or assets is null!" << std::endl;
     }
}