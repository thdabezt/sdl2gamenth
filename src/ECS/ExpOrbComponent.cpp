#include "ExpOrbComponent.h"

// Include FULL definitions needed for the implementation
#include "Components.h" // Includes TransformComponent.h, ColliderComponent.h etc.
#include "Player.h"
#include "../game.h"
#include "../Collision.h"
#include <stdexcept>
#include <iostream>

// --- REMOVED Constructor Definition (it's inline in .h now) ---

// --- init() Definition ---
void ExpOrbComponent::init() {
    initialized = false; // Reset flag
    if (!entity) { std::cerr << "Error in ExpOrbComponent::init: Entity is null!" << std::endl; return; }

    if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in ExpOrbComponent::init: Entity missing TransformComponent!" << std::endl; return; }
    transform = &entity->getComponent<TransformComponent>();
    if (!transform) { std::cerr << "Error in ExpOrbComponent::init: Failed to get TransformComponent!" << std::endl; return; }

    // ColliderComponent is added in game.cpp before this component now
    if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "Error in ExpOrbComponent::init: Entity missing ColliderComponent!" << std::endl; return; }
    collider = &entity->getComponent<ColliderComponent>();
     if (!collider) { std::cerr << "Error in ExpOrbComponent::init: Failed to get ColliderComponent!" << std::endl; return; }

    entity->addGroup(Game::groupExpOrbs); // Add to group

    initialized = true; // Set flag on success
}


// --- update() Definition ---
void ExpOrbComponent::update() {
    if (!initialized) return; // <<< CHECK Flag
    if (collected) return;
    // Keep internal checks too
    if (!collider || !Game::instance) return;

    Entity* playerEntityPtr = nullptr;
    Player* playerManager = Game::instance->getPlayerManager();
    if (!playerManager) return;

    try {
        playerEntityPtr = &playerManager->getEntity();
    } catch (const std::runtime_error& e) {
        std::cerr << "ExpOrbComponent: Error getting player entity: " << e.what() << std::endl;
        return;
    }

    // Check player components *after* getting the pointer
    if (playerEntityPtr && playerEntityPtr->isActive() && playerEntityPtr->hasComponent<ColliderComponent>()) {
        auto& playerCollider = playerEntityPtr->getComponent<ColliderComponent>();

        if (Collision::AABB(collider->collider, playerCollider.collider)) {
            // Collision detected
            playerManager->addExperience(experienceAmount);
            collected = true;
             if (entity) { // Destroy the orb entity
                 entity->destroy();
             }
        }
    }
}

// void ExpOrbComponent::draw() { // Optional draw method definition }