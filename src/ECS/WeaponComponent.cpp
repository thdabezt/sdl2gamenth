// --- Includes ---
#include "Components.h" // Should include WeaponComponent.h indirectly
#include "../AssetManager.h"
#include "../game.h" // Include game.h for Game::instance and statics
#include <iostream> // Keep for std::cerr error logging
#include <cmath>    // For std::cos, std::sin, acos, std::max
#include <algorithm> // For std::max

// --- Method Definitions ---

// --- Constructor ---
WeaponComponent::WeaponComponent(std::string weaponTag, int dmg, int rate, float speed, float spread,
                               int count, int size, std::string texId, int pierce,
                               int burstCount, int burst_Delay)
    : tag(std::move(weaponTag)), damage(dmg), fireRate(rate), projectileSpeed(speed),
      spreadAngle(spread), projectilesPerShot(count),
      projectileSize(size), projectileTexture(std::move(texId)),
      projectilePierce(pierce > 0 ? pierce : 1), // Ensure pierce is at least 1
      shotsPerBurst(burstCount > 0 ? burstCount : 1), // Ensure burst count is at least 1
      burstDelay(burst_Delay)
{
    // Initializations of pointers and state variables are done in the header or init()
}


// --- Initialization ---
void WeaponComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in WeaponComponent::init: Entity is null!" << std::endl; return; }

    // Get component pointers with checks
    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in WeaponComponent::init: Missing TransformComponent!" << std::endl; return; }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "Error in WeaponComponent::init: Missing ColliderComponent!" << std::endl; return; }
    collider = &entity->getComponent<ColliderComponent>();

    // Get SoundComponent (optional)
    if (entity->hasComponent<SoundComponent>()) { sound = &entity->getComponent<SoundComponent>(); }
    else { sound = nullptr; }

    // Final null checks on required pointers
    if (!transform) { std::cerr << "Error in WeaponComponent::init: Failed to get TransformComponent!" << std::endl; return; }
    if (!collider) { std::cerr << "Error in WeaponComponent::init: Failed to get ColliderComponent!" << std::endl; return; }

    lastShotTime = SDL_GetTicks(); // Initialize timer
    burstShotsRemaining = 0;

    initialized = true;
}

// --- Update Logic ---
void WeaponComponent::update() {
    if (!initialized || !transform || !collider) return; // Check required components

    Uint32 currentTime = SDL_GetTicks();

    // Handle burst firing first
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            shoot(); // Fire one shot of the burst
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) { // If more shots left, schedule next
                nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }
    // If not bursting, check if main cooldown is ready
    else if (currentTime > lastShotTime + fireRate) {
        burstShotsRemaining = shotsPerBurst; // Start a new burst sequence
        lastShotTime = currentTime;         // Reset main cooldown timer

        // Fire the first shot of the burst immediately
        if(burstShotsRemaining > 0) {
            shoot();
            burstShotsRemaining--;
            if (burstShotsRemaining > 0) { // Schedule next if burst has more shots
                 nextBurstShotTime = currentTime + burstDelay;
            }
        }
    }
    // Mouse position update is handled globally in Game::handleEvents and stored in Game statics
}

// --- Core Action ---
void WeaponComponent::shoot() {
    // Check required components again before shooting
    if (!initialized || !transform || !collider) return;

    // Play sound effect if SoundComponent exists
    if (sound) {
        sound->playSoundEffect("shoot"); // Assumes "shoot" is the internal name mapped in SoundComponent
    }

    // Calculate projectile spawn position (center of the collider)
    Vector2D projectilePosition;
    projectilePosition.x = static_cast<float>(collider->collider.x) + collider->collider.w / 2.0f;
    projectilePosition.y = static_cast<float>(collider->collider.y) + collider->collider.h / 2.0f;

    // Calculate direction towards mouse cursor (using global Game coordinates)
    Vector2D targetPosition(static_cast<float>(Game::mouseX), static_cast<float>(Game::mouseY));
    Vector2D direction = targetPosition - projectilePosition;

    // Normalize direction vector (handle zero vector case)
    if (direction.x == 0.0f && direction.y == 0.0f) { direction.y = -1.0f; } // Default upwards
    else { direction = direction.Normalize(); }

    // Calculate base velocity
    Vector2D baseVelocity = direction * projectileSpeed;

    // Handle single vs multiple projectiles (spread)
    if (projectilesPerShot <= 1) {
        // Fire a single projectile
        createProjectile(projectilePosition, baseVelocity);
    } else {
        // Fire multiple projectiles with spread
        const double PI = acos(-1.0); // Define PI locally if needed elsewhere
        float spreadRad = spreadAngle * (PI / 180.0f); // Convert spreadAngle (degrees assumed) to radians
        float halfSpread = spreadRad / 2.0f;
        float angleStep = (projectilesPerShot > 1) ? spreadRad / (projectilesPerShot - 1) : 0.0f;

        for (int i = 0; i < projectilesPerShot; i++) {
            float currentAngle = -halfSpread + (i * angleStep); // Angle offset from base direction

            // Rotate the base velocity vector
            Vector2D rotatedVelocity;
            float cosA = std::cos(currentAngle);
            float sinA = std::sin(currentAngle);
            rotatedVelocity.x = baseVelocity.x * cosA - baseVelocity.y * sinA;
            rotatedVelocity.y = baseVelocity.x * sinA + baseVelocity.y * cosA;

            createProjectile(projectilePosition, rotatedVelocity);
        }
    }
}

// --- Property Modifiers ---
void WeaponComponent::increaseDamage(int amount) { damage += amount; }
void WeaponComponent::decreaseFireRate(int amount) { fireRate = std::max(10, fireRate - amount); } // Added minimum check
void WeaponComponent::increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
void WeaponComponent::increaseSpread(float amount) { spreadAngle += amount; }
void WeaponComponent::increaseProjectileCount(int amount) { projectilesPerShot += amount; }
void WeaponComponent::increaseProjectileSize(int amount) { projectileSize += amount; }
void WeaponComponent::increasePierce(int amount) { projectilePierce += amount; }
void WeaponComponent::increaseBurstCount(int amount) { shotsPerBurst = std::max(1, shotsPerBurst + amount); } // Ensure at least 1

void WeaponComponent::decreaseFireRatePercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    fireRate = static_cast<int>(fireRate * multiplier);
    fireRate = std::max(10, fireRate); // Ensure a minimum fire rate
}

// --- Getters ---
std::string WeaponComponent::getTag() const { return tag; }
int WeaponComponent::getDamage() const { return damage; }
int WeaponComponent::getFireRate() const { return fireRate; }
float WeaponComponent::getProjectileSpeed() const { return projectileSpeed; }
float WeaponComponent::getSpreadAngle() const { return spreadAngle; }
int WeaponComponent::getProjectileCount() const { return projectilesPerShot; }
int WeaponComponent::getProjectileSize() const { return projectileSize; }
int WeaponComponent::getPierce() const { return projectilePierce; }
int WeaponComponent::getBurstCount() const { return shotsPerBurst; }

// --- Level Management ---
int WeaponComponent::getLevel() const { return level; }
void WeaponComponent::setLevel(int newLevel) { level = std::max(0, newLevel); } // Ensure level >= 0
void WeaponComponent::incrementLevel() { level++; }

// --- Private Helper ---
void WeaponComponent::createProjectile(Vector2D position, Vector2D velocity) {
     if (Game::instance && Game::instance->assets) {
         // Call the AssetManager to create the projectile entity
         Game::instance->assets->CreateProjectile(
             position,
             velocity,
             damage,
             projectileSize,
             projectileTexture,
             projectilePierce
             // Burst count/delay are handled by WeaponComponent
         );
     } else {
          std::cerr << "Error in WeaponComponent::createProjectile: Game instance or assets pointer is null!" << std::endl;
     }
}