// --- Includes ---
#include "ExpOrbComponent.h"
#include "Components.h" // Includes TransformComponent.h, ColliderComponent.h etc.
#include "Player.h"     // Needed for Player class definition
#include "../game.h"    // For Game::instance access
#include "../Collision.h" // For Collision::AABB
#include <stdexcept>   // For std::runtime_error
#include <iostream>    // For std::cerr error logging

// --- Method Definitions ---

// --- init() ---
void ExpOrbComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in ExpOrbComponent::init: Entity is null!" << std::endl; return; }

    // Get required TransformComponent
    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr << "Error in ExpOrbComponent::init: Entity missing TransformComponent!" << std::endl;
        return;
    }
    transform = &entity->getComponent<TransformComponent>();
    if (!transform) {
        std::cerr << "Error in ExpOrbComponent::init: Failed to get TransformComponent!" << std::endl;
        return;
    }

    // Get required ColliderComponent
    if (!entity->hasComponent<ColliderComponent>()) {
        std::cerr << "Error in ExpOrbComponent::init: Entity missing ColliderComponent!" << std::endl;
        return;
    }
    collider = &entity->getComponent<ColliderComponent>();
    if (!collider) {
        std::cerr << "Error in ExpOrbComponent::init: Failed to get ColliderComponent!" << std::endl;
        return;
    }

    // Add this entity to the experience orb group for efficient processing
    entity->addGroup(Game::groupExpOrbs);

    initialized = true; // Set flag on successful initialization
}


// --- update() ---
void ExpOrbComponent::update() {
    if (!initialized || collected) return; // Don't update if not initialized or already collected

    // Need collider and Game instance to check for player collision
    if (!collider || !Game::instance) return;

    // Get player manager and entity
    Player* playerManager = Game::instance->getPlayerManager();
    if (!playerManager) return; // Cannot check collision without player manager

    Entity* playerEntityPtr = nullptr;
    try {
        playerEntityPtr = &playerManager->getEntity(); // Get player entity via manager
    } catch (const std::runtime_error& e) {
        // Handle case where player entity might be null (e.g., during scene transition)
        std::cerr << "ExpOrbComponent::update Error: " << e.what() << std::endl;
        return;
    }

    // Check player validity and required components for collision
    if (playerEntityPtr && playerEntityPtr->isActive() && playerEntityPtr->hasComponent<ColliderComponent>()) {
        auto& playerCollider = playerEntityPtr->getComponent<ColliderComponent>();

        // Check for collision between orb's collider and player's collider
        if (Collision::AABB(collider->collider, playerCollider.collider)) {
            // Collision detected: Grant experience and destroy orb
            playerManager->addExperience(experienceAmount);
            collected = true; // Mark as collected
            if (entity) {
                entity->destroy(); // Mark the orb entity for removal
            }
        }
    }
}

