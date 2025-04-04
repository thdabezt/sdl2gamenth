// --- Includes ---
#include "Components.h" // Includes SpellComponent.h indirectly
#include "../AssetManager.h"
#include "../game.h"     // For Game::instance access
#include <iostream>    // For std::cerr error logging
#include <cmath>       // For std::cos, std::sin, acos, std::max
#include <cstdlib>     // For rand(), RAND_MAX
// #include <ctime>    // Not strictly needed if seeded globally

// --- Constructor ---

SpellComponent::SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                               int count, int size, std::string texId, SpellTrajectory mode,
                               float growthRate, int pierce)
    : tag(std::move(spellTag)), damage(dmg), cooldown(cool), projectileSpeed(speed),
      projectilesPerCast(count), projectileSize(size), projectileTexture(std::move(texId)),
      trajectoryMode(mode), spiralGrowthRate(growthRate),
      projectilePierce(pierce > 0 ? pierce : 1) // Ensure pierce is at least 1
{
    // Pointers (transform, sound) are initialized to nullptr in the header.
    // State variables (lastCastTime, spiralAngle, level, etc.) are initialized in the header or init().
    // Global random seed should be handled elsewhere (e.g., Game constructor or main).
}

// --- Method Definitions ---

// --- Component Lifecycle ---

void SpellComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in SpellComponent::init: Entity is null!" << std::endl; return; }

    // Get required TransformComponent
    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in SpellComponent::init (" << tag << "): Entity missing TransformComponent!" << std::endl; return; }
    transform = &entity->getComponent<TransformComponent>();
    if (!transform) { std::cerr << "Error in SpellComponent::init (" << tag << "): Failed to get TransformComponent pointer!" << std::endl; return; }

    // Get optional SoundComponent
    if (entity->hasComponent<SoundComponent>()) { sound = &entity->getComponent<SoundComponent>(); }
    else { sound = nullptr; }

    lastCastTime = SDL_GetTicks(); // Initialize cooldown timer
    initialized = true;
}

void SpellComponent::update() {
    if (!initialized || !transform) return; // Check initialization and required pointers

    Uint32 currentTime = SDL_GetTicks();

    // Burst fire logic (if shots remaining)
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            castSingleProjectile(); // Cast one projectile of the burst
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) { // Schedule next if more remain
               nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }
    // Cooldown check (only if not bursting)
    else if (currentTime > lastCastTime + cooldown) {
        // Determine if this spell uses burst logic
        bool useBurst = (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION || trajectoryMode == SpellTrajectory::SPIRAL); // Example condition

        if (useBurst) {
            burstShotsRemaining = projectilesPerCast; // Start the burst sequence
            nextBurstShotTime = currentTime;          // First shot is immediate in the sequence
            lastCastTime = currentTime;               // Reset main cooldown timer

            // Play sound once at the start of the burst
            if (sound && projectilesPerCast > 0) {
                if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
                     sound->playSoundEffect("star_cast");
                 } else if (trajectoryMode == SpellTrajectory::SPIRAL) {
                     sound->playSoundEffect("fire_cast");
                 }
                 // Add other sounds for different spell tags if needed
            }

            // Cast the very first shot of the burst now
            if (burstShotsRemaining > 0) {
               castSingleProjectile();
               burstShotsRemaining--;
                if (burstShotsRemaining > 0) { // Set timer only if more shots remain after the first
                   nextBurstShotTime = currentTime + burstDelay;
               }
            }
        } else {
             // Handle non-burst spells (if any) - Currently, both main types use burst
             // castSpellFull(); // Call the full cast logic for non-burst
             lastCastTime = currentTime;
             // Play sound for non-burst spells if needed
        }
    }

    // Update spiral angle regardless of cooldown (for visual effect or next cast)
    if (trajectoryMode == SpellTrajectory::SPIRAL) {
         spiralAngle += 0.1f; // Adjust rotation speed as needed
         const double PI = acos(-1.0);
         if (spiralAngle > 2.0 * PI) {
             spiralAngle -= 2.0 * PI;
         }
    }
}

// --- Casting Logic ---

// Public castSpell function - currently delegates based on update logic, could be simplified
void SpellComponent::castSpell() {
    // This function might be redundant if all casting logic is handled within update()
    // For now, it can just call the internal helper for non-burst spells if needed.
    if (trajectoryMode != SpellTrajectory::RANDOM_DIRECTION && trajectoryMode != SpellTrajectory::SPIRAL) {
         // castSpellFull(); // Example for a non-burst spell
    }
    // Burst spells are initiated and handled within update()
}

// Casts a single projectile (used by burst logic or potentially single-shot spells)
void SpellComponent::castSingleProjectile() {
    if (!initialized || !transform) return; // Basic checks

    // Calculate spawn center based on entity's transform
    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;

    const double PI = acos(-1.0);
    Vector2D velocity;

    // Calculate velocity based on trajectory mode
    if (trajectoryMode == SpellTrajectory::RANDOM_DIRECTION) {
        float randomAngle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * PI)));
        velocity.x = std::cos(randomAngle) * projectileSpeed;
        velocity.y = std::sin(randomAngle) * projectileSpeed;
        createProjectile(spawnCenter, velocity);
    } else if (trajectoryMode == SpellTrajectory::SPIRAL) {
        float currentRadius = spiralAngle * spiralGrowthRate;
        Vector2D spiralSpawnPosition;
        spiralSpawnPosition.x = spawnCenter.x + std::cos(spiralAngle) * currentRadius;
        spiralSpawnPosition.y = spawnCenter.y + std::sin(spiralAngle) * currentRadius;

        velocity.x = std::cos(spiralAngle) * projectileSpeed;
        velocity.y = std::sin(spiralAngle) * projectileSpeed;

        createProjectile(spiralSpawnPosition, velocity);

        // Increment spiral angle AFTER casting for the next step in the burst/sequence
        spiralAngle += 0.1f; // Adjust increment for visual spacing
        if (spiralAngle > 2.0 * PI) {
            spiralAngle -= 2.0 * PI;
        }
    }
}

// Casts all projectiles at once (original logic, potentially for non-burst spells)
void SpellComponent::castSpellFull() {
    if (!initialized || !transform ) return;

    Vector2D spawnCenter = transform->position;
    spawnCenter.x += (transform->width * transform->scale) / 2.0f;
    spawnCenter.y += (transform->height * transform->scale) / 2.0f;

    const double PI = acos(-1.0);

    switch (trajectoryMode) {
        // RANDOM_DIRECTION is handled by burst logic in update() via castSingleProjectile
        // SPIRAL is handled by burst logic in update() via castSingleProjectile

        default: // Fallback or handle other non-burst modes
             for (int i = 0; i < projectilesPerCast; ++i) {
                 // Example: Default random cast if mode isn't burst
                 float randomAngle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * PI)));
                 Vector2D defaultVel;
                 defaultVel.x = std::cos(randomAngle) * projectileSpeed;
                 defaultVel.y = std::sin(randomAngle) * projectileSpeed;
                 createProjectile(spawnCenter, defaultVel);
             }
            break;
    }
}

// Helper to create the actual projectile entity
void SpellComponent::createProjectile(Vector2D position, Vector2D velocity) {
    if (Game::instance && Game::instance->assets) {
        Game::instance->assets->CreateProjectile(position, velocity, damage, projectileSize, projectileTexture, projectilePierce);
    } else {
         std::cerr << "Error in SpellComponent::createProjectile: Game instance or assets is null!" << std::endl;
    }
}

// --- Property Modifiers ---
void SpellComponent::increaseDamage(int amount) { damage = std::max(0, damage + amount); } // Prevent negative damage
void SpellComponent::decreaseCooldown(int amount) { cooldown = std::max(50, cooldown - amount); } // Minimum cooldown 50ms
void SpellComponent::increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
void SpellComponent::increaseProjectileCount(int amount) { projectilesPerCast = std::max(1, projectilesPerCast + amount); } // Ensure at least 1
void SpellComponent::increaseProjectileSize(int amount) { projectileSize = std::max(1, projectileSize + amount); } // Ensure at least 1x1
void SpellComponent::increasePierce(int amount) { projectilePierce += amount; }

void SpellComponent::decreaseCooldownPercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    cooldown = static_cast<int>(cooldown * multiplier);
    cooldown = std::max(50, cooldown); // Ensure a minimum cooldown (e.g., 50ms)
}

// --- Getters ---
std::string SpellComponent::getTag() const { return tag; }
int SpellComponent::getDamage() const { return damage; }
int SpellComponent::getCooldown() const { return cooldown; }
float SpellComponent::getProjectileSpeed() const { return projectileSpeed; }
int SpellComponent::getProjectileSize() const { return projectileSize; }
int SpellComponent::getPierce() const { return projectilePierce; }
int SpellComponent::getProjectileCount() const { return projectilesPerCast; }
int SpellComponent::getDuration() const { return duration; }

// --- Level Management ---
int SpellComponent::getLevel() const { return level; }
void SpellComponent::setLevel(int newLevel) { level = std::max(0, newLevel); } // Ensure level >= 0
void SpellComponent::incrementLevel() { level++; }