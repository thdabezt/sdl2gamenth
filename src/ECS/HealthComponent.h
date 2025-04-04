#pragma once

// --- Includes ---
#include "Components.h" // Includes ECS.h indirectly
#include <algorithm>    // For std::max, std::min
#include <string>       // For std::string used in takeDamage logic
#include <iostream>     // For std::cerr if error logging needed


// --- Class Definition ---

// Manages the health points of an entity, allowing damage taking and healing.
class HealthComponent : public Component {
public:
    // --- Public Members ---
    int health;    // Current health points
    int maxHealth; // Maximum health points

    // --- Constructor ---
    // Initializes health and maximum health.
    HealthComponent(int h, int maxH) : health(h), maxHealth(maxH > 0 ? maxH : 1) { // Ensure maxHealth is positive
         // Ensure current health doesn't exceed max health initially
         health = std::min(health, maxHealth);
         health = std::max(0, health); // Ensure health isn't initially negative
    }

    // --- Public Methods ---

    // Component Lifecycle Overrides (Currently empty)
    void init() override {}
    void update() override {}

    // Applies damage to the entity's health.
    // If health drops to 0 or below, it marks the entity for destruction
    // unless the entity has the "player" tag.
    void takeDamage(int damage) {
        if (damage <= 0) return; // Ignore zero or negative damage

        health -= damage;

        if (health <= 0) {
            health = 0; // Clamp health at 0

            // Check if the owner entity should be destroyed
            if (entity != nullptr && entity->hasComponent<ColliderComponent>()) {
                const std::string& tag = entity->getComponent<ColliderComponent>().tag;
                if (tag != "player") { // Don't destroy the player entity here
                    entity->destroy(); // Mark non-player entities for removal
                }
            } else if (entity != nullptr) {
                 // If entity exists but has no collider, assume it's not the player?
                 // Or log a warning? Let's assume destroy for now if not player.
                 // Consider adding specific tags/components for entities that shouldn't auto-destroy.
                 entity->destroy();
            }
        }
    }

    // Sets the maximum health, ensuring the new value is positive.
    // Also clamps current health if it exceeds the new maximum.
    void setMaxHealth(int newMax) {
        maxHealth = std::max(1, newMax); // Ensure max health is at least 1
        health = std::min(health, maxHealth); // Clamp current health
    }

    // Sets the current health directly, clamping between 0 and maxHealth.
    void setHealth(int newHealth) {
        health = newHealth;
    }

    // Restores health points, clamping at maxHealth.
    void heal(int healAmount) {
        if (healAmount <= 0) return; // Ignore zero or negative healing
        health += healAmount;
        health = std::min(health, maxHealth); // Clamp at maximum
    }

    // Getters
    int getMaxHealth() const {
        return maxHealth;
    }
    int getHealth() const {
        return health;
    }

}; // End HealthComponent class