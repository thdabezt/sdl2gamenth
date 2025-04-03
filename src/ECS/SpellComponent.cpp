#include "Components.h" // Includes SpellComponent.h
#include "../AssetManager.h"
#include "../game.h"
#include <iostream>
// #define _USE_MATH_DEFINES // Not needed if using acos
#include <cmath>
#include <cstdlib>
#include <ctime>

        SpellComponent::SpellComponent(std::string spellTag, int dmg, int cool, float speed,
            int count, int size, std::string texId, /* REMOVE int dur, */ SpellTrajectory mode,
            float growthRate, int pierce)
        : tag(std::move(spellTag)), damage(dmg), cooldown(cool), projectileSpeed(speed),
        projectilesPerCast(count), projectileSize(size), projectileTexture(std::move(texId)),
        trajectoryMode(mode), spiralGrowthRate(growthRate),
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
    if (!initialized || !transform) return; // Check initialization and transform

    Uint32 currentTime = SDL_GetTicks();

    // Burst fire logic for spells that use it (e.g., modified Star)
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            castSingleProjectile(); // Cast one projectile of the burst
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) { // Don't set timer if it was the last shot
               nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }
    // Check main cooldown ONLY if not currently bursting
    else if (currentTime > lastCastTime + cooldown) {
        
        if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION || trajectoryMode == SpellTrajectory::SPIRAL ) { // Apply burst logic only to Star (or others if desired)
            burstShotsRemaining = projectilesPerCast; // Start the burst sequence
            nextBurstShotTime = currentTime; // First shot immediately
            lastCastTime = currentTime;      // Reset main cooldown timer

            if (sound && projectilesPerCast > 0) {
                // --- ADD DEBUG LOGGING ---
                // std::cout << "DEBUG: Spell '" << tag << "' attempting to play sound." << std::endl;
                // --- END DEBUG LOGGING ---
                if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
                     sound->playSoundEffect("star_cast");
                 } else if (trajectoryMode == SpellTrajectory::SPIRAL) {
                     sound->playSoundEffect("fire_cast");
                 }
            }
            
            // Cast the first shot of the burst immediately
            if (burstShotsRemaining > 0) {
               castSingleProjectile();
               burstShotsRemaining--;
                if (burstShotsRemaining > 0) { // Set timer only if more shots remain
                   nextBurstShotTime = currentTime + burstDelay;
               }
            }
        } else { // Handle non-burst spells (like the Fire spiral)
             castSpell(); // Use the original cast logic
             lastCastTime = currentTime;
        }
    }

    // Existing spiral angle update (only relevant if trajectoryMode is SPIRAL)
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
    if (Game::instance && Game::instance->assets) {
        Game::instance->assets->CreateProjectile(position, velocity, damage, projectileSize, projectileTexture, projectilePierce); // Remove duration argument
    } else {
         std::cerr << "Error in SpellComponent::createProjectile: Game instance or assets is null!" << std::endl;
    }
}

// New method for casting one projectile (used by burst logic)
void SpellComponent::castSingleProjectile() {
    if (!initialized || !transform) return; // Already checked in update()

    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;
    const double PI = acos(-1.0);

    // Logic specific to the spell type
    if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
        // --- Sound for Star (optional: only on first shot?) ---
        

        // --- End Sound ---
        float randomAngle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * PI)));
        Vector2D velocity;
        velocity.x = std::cos(randomAngle) * projectileSpeed;
        velocity.y = std::sin(randomAngle) * projectileSpeed;
        createProjectile(spawnCenter, velocity);

    } else if (trajectoryMode == SpellTrajectory::SPIRAL) { // <<< ADDED SPIRAL LOGIC HERE
        // --- Sound for Fire (optional: only on first shot?) ---
         
        // --- End Sound ---

        // Calculate current spiral position and velocity for *this* shot
        float currentRadius = spiralAngle * spiralGrowthRate;
        Vector2D spiralSpawnPosition;
        spiralSpawnPosition.x = spawnCenter.x + std::cos(spiralAngle) * currentRadius;
        spiralSpawnPosition.y = spawnCenter.y + std::sin(spiralAngle) * currentRadius;

        Vector2D velocity;
        velocity.x = std::cos(spiralAngle) * projectileSpeed;
        velocity.y = std::sin(spiralAngle) * projectileSpeed;

        // Create just one projectile for this step in the burst
        createProjectile(spiralSpawnPosition, velocity);

        // --- Increment spiralAngle AFTER casting this projectile ---
        // This ensures the *next* projectile in the burst appears at the next spiral point.
        // Adjust the increment amount (0.1f) as needed for visual spacing.
        spiralAngle += 0.1f;
        if (spiralAngle > 2.0 * PI) {
            spiralAngle -= 2.0 * PI;
        }
        // --- End Increment ---
    }
    // Add other trajectory logic here if needed for bursting other spell types
}
// Rename original castSpell
void SpellComponent::castSpellFull() { // Renamed from castSpell
    if (!initialized || !transform ) return;

    

    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;
    const double PI = acos(-1.0);

    switch (trajectoryMode) {
        // RANDOM_DIRECTION is now handled by burst logic in update()
        // case SpellTrajectory::RANDOM_DIRECTION:
            // ... (Original logic removed or commented out) ...
            // break;

        case SpellTrajectory::SPIRAL: // Keep spiral logic for Fire spell
             if (projectilesPerCast > 0) {
                 float currentRadius = spiralAngle * spiralGrowthRate;
                 Vector2D spiralSpawnPosition;
                 spiralSpawnPosition.x = spawnCenter.x + std::cos(spiralAngle) * currentRadius;
                 spiralSpawnPosition.y = spawnCenter.y + std::sin(spiralAngle) * currentRadius;

                 // Calculate velocity ONCE for the spiral direction
                 Vector2D baseVelocity;
                 baseVelocity.x = std::cos(spiralAngle) * projectileSpeed;
                 baseVelocity.y = std::sin(spiralAngle) * projectileSpeed;

                 // Create all projectiles for the spiral at once
                 for(int i = 0; i < projectilesPerCast; i++) {
                     // Slightly offset spawn position for visual effect? Optional.
                     // Vector2D offsetPos = spiralSpawnPosition + Vector2D(i*2, i*2); // Example offset
                     createProjectile(spiralSpawnPosition, baseVelocity);
                 }
             }
            break;
         // Default case or handle other trajectories
         default:
              // Fallback: Cast one projectile randomly if mode not handled
              castSingleProjectile();
              break;
    }
}
void SpellComponent::decreaseCooldownPercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    cooldown = static_cast<int>(cooldown * multiplier);
    cooldown = std::max(50, cooldown); // Ensure a minimum cooldown (e.g., 50ms)
}