#include "Components.h"
#include "../AssetManager.h"

void WeaponComponent::init() {
    // Get component pointers (Requires TransformComponent.h/ColliderComponent.h included via Components.h)
    if (entity->hasComponent<TransformComponent>()) {
        transform = &entity->getComponent<TransformComponent>();
    } else { std::cerr << "WeaponComponent missing TransformComponent!" << std::endl; }

    if (entity->hasComponent<ColliderComponent>()) {
        collider = &entity->getComponent<ColliderComponent>();
    } else { std::cerr << "WeaponComponent missing ColliderComponent!" << std::endl; }

    lastShotTime = SDL_GetTicks(); // Initialize timer correctly
    burstShotsRemaining = 0; // Ensure starting state
}

// --- Rewritten Update Logic for Burst Fire ---
void WeaponComponent::update() {
    Uint32 currentTime = SDL_GetTicks();

    // 1. Check if currently in a burst sequence
    if (burstShotsRemaining > 0) {
        // Check if it's time for the next shot in the burst
        if (currentTime >= nextBurstShotTime) {
            if (transform && collider) { // Ensure components are valid before shooting
                 shoot(); // Fire one volley (projectilesPerShot simultaneous projectiles)
                 burstShotsRemaining--; // Decrement remaining shots in burst
                 // Set time for the *next* shot in the burst
                 nextBurstShotTime = currentTime + burstDelay;
            } else {
                // Missing components, cancel burst
                 burstShotsRemaining = 0;
            }
        }
    }
    // 2. Else, check if ready to start a NEW burst (main cooldown)
    else if (currentTime > lastShotTime + fireRate) {
        // Start a new burst
        burstShotsRemaining = shotsPerBurst;
        // Schedule the first shot immediately (or add burstDelay if you want a delay before the first shot too)
        nextBurstShotTime = currentTime;
        // Record the time this burst sequence started (for the main cooldown)
        lastShotTime = currentTime;

        // We trigger the first shot immediately by letting the logic loop back to step 1
        // OR call shoot() directly here if preferred, adjusting burstShotsRemaining and nextBurstShotTime accordingly.
        // Let's trigger it immediately for simplicity here:
        if(burstShotsRemaining > 0 && transform && collider) {
            shoot();
            burstShotsRemaining--;
            nextBurstShotTime = currentTime + burstDelay; // Schedule the *next* one
        }
    }

    // Update mouse position regardless of firing state (for aiming)
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Game::mouseX = mouseX + Game::camera.x;
    Game::mouseY = mouseY + Game::camera.y;
}


// --- shoot() method remains the same ---
// It fires one volley of 'projectilesPerShot' projectiles with spread
void WeaponComponent::shoot() {
    // Check required components again before dereferencing
    if (!transform || !collider) {
        // std::cerr << "WeaponComponent::shoot() called without valid transform or collider!" << std::endl;
        return;
    }
    if (entity->hasComponent<SoundComponent>()) {
        // Call playSoundEffect using the internal name "shoot"
        entity->getComponent<SoundComponent>().playSoundEffect("shoot");
    }
    Vector2D projectilePosition;
    projectilePosition.x = static_cast<float>(collider->collider.x) + collider->collider.w / 2.0f; // Center start pos
    projectilePosition.y = static_cast<float>(collider->collider.y) + collider->collider.h / 2.0f;

    Vector2D targetPosition(static_cast<float>(Game::mouseX), static_cast<float>(Game::mouseY));
    Vector2D direction = targetPosition - projectilePosition;

    // Prevent division by zero if direction is zero vector
    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction.y = -1.0f; // Default direction (e.g., straight up)
    }

    Vector2D baseVelocity = direction.Normalize() * projectileSpeed;

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

// --- createProjectile() method remains the same ---
void WeaponComponent::createProjectile(Vector2D position, Vector2D velocity) {
    Game::assets->CreateProjectile(position, velocity, projectileRange, damage, projectileSize, projectileTexture, projectilePierce);
}