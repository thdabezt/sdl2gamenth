// In Game/src/ECS/SpellComponent.cpp


// --- Include FULL headers needed for implementation ---
#include "../AssetManager.h"      // For Game::assets->CreateProjectile
#include "../game.h"            // For Game::assets
#include "Components.h"
// ----------------------------------------------------

#include <iostream> // For std::cerr, std::cout (optional debugging)
#define _USE_MATH_DEFINES // Define BEFORE including cmath
#include <cmath>    // For M_PI, cos, sin (include AFTER define)
#include <cstdlib>  // For rand()
#include <ctime>    // For time()


// --- Update Constructor Implementation (Add pierce) ---
SpellComponent::SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, int dur, SpellTrajectory mode,
                   float growthRate, int pierce) // <<< ADD pierce parameter
        : tag(spellTag),
          damage(dmg),
          cooldown(cool),
          projectileSpeed(speed),
          projectilesPerCast(count),
          projectileSize(size),
          projectileTexture(texId),
          duration(dur),         // Storing duration separately
          trajectoryMode(mode),
          spiralGrowthRate(growthRate),
          projectilePierce(pierce > 0 ? pierce : 1) // <<< INITIALIZE pierce (ensure >= 1)
          {
              transform = nullptr;
              lastCastTime = 0;
              spiralAngle = 0.0f;
              if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
                   std::srand(static_cast<unsigned int>(std::time(nullptr)));
              }
          }

// --- Init Implementation (No change needed) ---
void SpellComponent::init() {
    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr << "SpellComponent Error: Entity missing TransformComponent during init!" << std::endl;
        return;
    }
    transform = &entity->getComponent<TransformComponent>();
    lastCastTime = SDL_GetTicks();
}

// --- Update Implementation (Using acos(-1.0) for pi) ---
void SpellComponent::update() {
    if (!transform) {
         if (entity->hasComponent<TransformComponent>()) {
             transform = &entity->getComponent<TransformComponent>();
         } else {
             return;
         }
    }

    Uint32 currentTime = SDL_GetTicks();

    if (currentTime > lastCastTime + cooldown) {
        castSpell();
        lastCastTime = currentTime;
    }

    if (trajectoryMode == SpellTrajectory::SPIRAL) {
         spiralAngle += 0.1f;
         const double PI = acos(-1.0); // Use acos for pi
         if (spiralAngle > 2.0 * PI) {
             spiralAngle -= 2.0 * PI;
         }
    }
}

// --- CastSpell Implementation (Using acos(-1.0) for pi) ---
void SpellComponent::castSpell() {
    if (!transform) return;

    Vector2D playerCenter = transform->position;
    if (entity->hasComponent<ColliderComponent>()) {
         auto& collider = entity->getComponent<ColliderComponent>();
         playerCenter.x = static_cast<float>(collider.collider.x) + collider.collider.w / 2.0f;
         playerCenter.y = static_cast<float>(collider.collider.y) + collider.collider.h / 2.0f;
    }

    const double PI = acos(-1.0);

    switch (trajectoryMode) {
        case SpellTrajectory::RANDOM_DIRECTION:
            // Loop correctly handles projectilesPerCast == 0
            for (int i = 0; i < projectilesPerCast; ++i) {
                float randomAngle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * PI)));
                Vector2D velocity;
                velocity.x = cos(randomAngle) * projectileSpeed;
                velocity.y = sin(randomAngle) * projectileSpeed;
                createProjectile(playerCenter, velocity);
            }
            break;

        case SpellTrajectory::SPIRAL:
             // <<< FIX: Check projectilesPerCast before firing >>>
             if (projectilesPerCast > 0) {
                  // Only execute if we are supposed to cast at least one projectile
                  float currentRadius = spiralAngle * spiralGrowthRate;
                  Vector2D spiralSpawnPosition;
                  spiralSpawnPosition.x = playerCenter.x + cos(spiralAngle) * currentRadius;
                  spiralSpawnPosition.y = playerCenter.y + sin(spiralAngle) * currentRadius;
                  Vector2D velocity;
                  velocity.x = cos(spiralAngle) * projectileSpeed;
                  velocity.y = sin(spiralAngle) * projectileSpeed;
                  createProjectile(spiralSpawnPosition, velocity);
             }
             // <<< END FIX >>>
            break;
    }
}


// --- Update CreateProjectile Implementation (Pass pierce) ---
void SpellComponent::createProjectile(Vector2D position, Vector2D velocity) {
    // Pass the component's 'projectilePierce' value.
    // Still passing 'duration' as the 'range' argument based on previous steps.
    // Ensure AssetManager::CreateProjectile expects this.
    Game::assets->CreateProjectile(position, velocity, duration, damage, projectileSize, projectileTexture, projectilePierce); // Pass pierce here
}