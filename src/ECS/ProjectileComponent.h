#pragma once

// --- Includes ---
#include "ECS.h"
#include "../Vector2D.h"
#include "../game.h"
#include "Components.h"
#include <cmath>
#include <iostream>
#include <set>

// --- Forward Declarations ---
class TransformComponent; // Explicit forward declaration

// --- Class Definition ---

class ProjectileComponent : public Component {
private:
    // --- Private Members ---
    TransformComponent *transform = nullptr; // Pointer to entity's transform component
    Vector2D startPos;                       // Initial position (unused currently)
    int damage = 0;                          // Damage dealt on hit
    Vector2D velocity;                       // Movement vector per update
    int maxPierce = 1;                       // Max number of enemies the projectile can hit
    std::set<Entity*> hitEnemies;            // Set of enemies already hit by this projectile
    bool initialized = false;                // Initialization flag

public:
    // --- Public Members ---
    bool isSpinning = false; // Flag to indicate if the projectile sprite should rotate

    // --- Constructor ---
    // Initializes projectile properties. Ensures pierce is at least 1.
    ProjectileComponent(int dmg, Vector2D vel, int pierce = 1)
        : damage(dmg), velocity(vel), maxPierce(pierce > 0 ? pierce : 1) {}

    // --- Destructor ---
    ~ProjectileComponent() override = default; // Default destructor is likely sufficient

    // --- Public Methods ---

    // Component Lifecycle Overrides
    void init() override {
        initialized = false; // Reset flag
        if (!entity) { std::cerr << "Error in ProjectileComponent::init: Entity is null!" << std::endl; return; }
        if (!entity->hasComponent<TransformComponent>()) {
            std::cerr << "Error in ProjectileComponent::init: Missing TransformComponent!" << std::endl;
            // Optionally destroy entity if it's unusable without transform
            // if (entity) entity->destroy();
            return;
        }
        transform = &entity->getComponent<TransformComponent>();
        if (!transform) {
            std::cerr << "Error in ProjectileComponent::init: Failed to get TransformComponent pointer!" << std::endl;
            // if (entity) entity->destroy(); // Optionally destroy
            return;
        }

        startPos = transform->position; // Store starting position (though unused)
        initialized = true;
    }

    void update() override {
        if (!initialized || !transform) {
            // If not initialized or transform is lost, destroy the projectile entity
            if (entity) entity->destroy();
            return;
        }

        // Update position based on velocity
        transform->position += velocity;

        // Check if projectile is way out of camera view and destroy if so
        // Added buffer (100px) around camera bounds
        if (transform->position.x > Game::camera.x + Game::camera.w + 100 ||
            transform->position.x + transform->width * transform->scale < Game::camera.x - 100 || // Check left edge too
            transform->position.y > Game::camera.y + Game::camera.h + 100 ||
            transform->position.y + transform->height * transform->scale < Game::camera.y - 100)  // Check top edge too
        {
             if (entity) entity->destroy();
             return; // Stop further updates if destroyed
        }
        // Note: Rotation logic is handled in SpriteComponent::update based on the isSpinning flag
    }

    // Getters
    int getDamage() const {
        return damage;
    }

    // Hit Tracking & Pierce Logic
    // Checks if this projectile has already hit the specified enemy.
    bool hasHit(Entity* enemy) const {
        return hitEnemies.count(enemy) > 0;
    }

    // Records that this projectile has hit the specified enemy.
    void recordHit(Entity* enemy) {
        if (enemy) { // Basic null check
           hitEnemies.insert(enemy);
        }
    }

    // Determines if the projectile should be destroyed based on pierce count.
    bool shouldDestroy() const {
        // Use static_cast for safe comparison between size_t and int
        return hitEnemies.size() >= static_cast<std::set<Entity*>::size_type>(maxPierce);
    }

}; // End ProjectileComponent class