#include "Components.h" // Should include WeaponComponent.h
#include "../AssetManager.h"
#include "../game.h" // Include game.h for Game::instance and Game::camera/mouseX/mouseY
#include <iostream> // For logging
#include <cmath>    // For std::cos, std::sin, acos

void WeaponComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in WeaponComponent::init: Entity is null!" << std::endl; return; }

    // Get component pointers
    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in WeaponComponent::init: Missing TransformComponent!" << std::endl; return; }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "Error in WeaponComponent::init: Missing ColliderComponent!" << std::endl; return; }
    collider = &entity->getComponent<ColliderComponent>();

     // Get SoundComponent (optional)
     if (entity->hasComponent<SoundComponent>()) {
         sound = &entity->getComponent<SoundComponent>();
     } else {
         sound = nullptr;
     }

     // --- Null Check after getting pointers ---
     if (!transform) { std::cerr << "Error in WeaponComponent::init: Failed to get TransformComponent!" << std::endl; return; }
     if (!collider) { std::cerr << "Error in WeaponComponent::init: Failed to get ColliderComponent!" << std::endl; return; }
     // Sound is optional

    lastShotTime = SDL_GetTicks(); // Initialize timer correctly
    burstShotsRemaining = 0; // Ensure starting state

    initialized = true; // <<< SET Flag on success
}

void WeaponComponent::update() {
    if (!initialized) return; // <<< CHECK Flag
    // Keep internal checks
     if (!transform || !collider) return;

    Uint32 currentTime = SDL_GetTicks();

    // Burst fire logic
    if (burstShotsRemaining > 0) {
        if (currentTime >= nextBurstShotTime) {
            if (transform && collider) { // Check components before shooting
                 shoot();
                 burstShotsRemaining--;
                 nextBurstShotTime = currentTime + burstDelay;
            } else { burstShotsRemaining = 0; } // Cancel burst if components invalid
        }
    }
    else if (currentTime > lastShotTime + fireRate) { // Check main cooldown
        burstShotsRemaining = shotsPerBurst;
        nextBurstShotTime = currentTime; // First shot immediately
        lastShotTime = currentTime; // Reset main cooldown timer

        if(burstShotsRemaining > 0 && transform && collider) { // Check components before shooting
            shoot();
            burstShotsRemaining--;
            nextBurstShotTime = currentTime + burstDelay;
        }
    }
    // Mouse position update is handled globally in Game::handleEvents
}

void WeaponComponent::shoot() {
    if (!initialized) return; // <<< CHECK Flag
    // Keep internal checks
    if (!transform || !collider) return;

    if (sound) {
        sound->playSoundEffect("shoot");
    }

    Vector2D projectilePosition;
    projectilePosition.x = static_cast<float>(collider->collider.x) + collider->collider.w / 2.0f;
    projectilePosition.y = static_cast<float>(collider->collider.y) + collider->collider.h / 2.0f;

    Vector2D targetPosition(static_cast<float>(Game::mouseX), static_cast<float>(Game::mouseY));
    Vector2D direction = targetPosition - projectilePosition;

    if (direction.x == 0.0f && direction.y == 0.0f) { direction.y = -1.0f; }
    else { direction = direction.Normalize(); }

    Vector2D baseVelocity = direction * projectileSpeed;

    if (projectilesPerShot <= 1) {
        createProjectile(projectilePosition, baseVelocity);
    } else {
        float halfSpread = spreadAngle / 2.0f;
        float angleStep = (projectilesPerShot > 1) ? spreadAngle / (projectilesPerShot - 1) : 0.0f;

        for (int i = 0; i < projectilesPerShot; i++) {
            float currentAngle = -halfSpread + (i * angleStep); // Centered spread

            Vector2D rotatedVelocity;
            float cosA = cos(currentAngle);
            float sinA = sin(currentAngle);
            rotatedVelocity.x = baseVelocity.x * cosA - baseVelocity.y * sinA;
            rotatedVelocity.y = baseVelocity.x * sinA + baseVelocity.y * cosA;
            // No need to re-normalize if baseVelocity has correct speed and rotation preserves length
            // rotatedVelocity = rotatedVelocity.Normalize() * projectileSpeed; // Re-normalize only if necessary

            createProjectile(projectilePosition, rotatedVelocity);
        }
    }
}
// In WeaponComponent.cpp
void WeaponComponent::decreaseFireRatePercentage(float percent) {
    if (percent <= 0) return;
    float multiplier = 1.0f - (percent / 100.0f);
    fireRate = static_cast<int>(fireRate * multiplier);
    fireRate = std::max(10, fireRate); // Ensure a minimum fire rate (e.g., 10ms delay)
}

void WeaponComponent::createProjectile(Vector2D position, Vector2D velocity) {
    // if (!initialized) return; // CreateProjectile is internal helper, shoot checks initialized
     if (Game::instance && Game::instance->assets) {
         Game::instance->assets->CreateProjectile(position, velocity, damage, projectileSize, projectileTexture, projectilePierce);
     } else {
          std::cerr << "Error in WeaponComponent::createProjectile: Game instance or assets pointer is null!" << std::endl;
     }
}