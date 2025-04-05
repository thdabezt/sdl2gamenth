#include <iostream>
#include <stdexcept>

#include "../Collision.h"
#include "../game.h"
#include "Components.h"
#include "ExpOrbComponent.h"
#include "Player.h"

void ExpOrbComponent::init() {
    initialized = false;
    if (!entity) {
        std::cerr << "Error in ExpOrbComponent::init: Entity is null!"
                  << std::endl;
        return;
    }

    if (!entity->hasComponent<TransformComponent>()) {
        std::cerr << "Error in ExpOrbComponent::init: Entity missing "
                     "TransformComponent!"
                  << std::endl;
        return;
    }
    transform = &entity->getComponent<TransformComponent>();
    if (!transform) {
        std::cerr << "Error in ExpOrbComponent::init: Failed to get "
                     "TransformComponent!"
                  << std::endl;
        return;
    }

    if (!entity->hasComponent<ColliderComponent>()) {
        std::cerr << "Error in ExpOrbComponent::init: Entity missing "
                     "ColliderComponent!"
                  << std::endl;
        return;
    }
    collider = &entity->getComponent<ColliderComponent>();
    if (!collider) {
        std::cerr << "Error in ExpOrbComponent::init: Failed to get "
                     "ColliderComponent!"
                  << std::endl;
        return;
    }

    entity->addGroup(Game::groupExpOrbs);

    initialized = true;
}

void ExpOrbComponent::update() {
    if (!initialized || collected) return;

    if (!collider || !Game::instance) return;

    Player* playerManager = Game::instance->getPlayerManager();
    if (!playerManager) return;

    Entity* playerEntityPtr = nullptr;
    try {
        playerEntityPtr = &playerManager->getEntity();
    } catch (const std::runtime_error& e) {
        std::cerr << "ExpOrbComponent::update Error: " << e.what() << std::endl;
        return;
    }

    if (playerEntityPtr && playerEntityPtr->isActive() &&
        playerEntityPtr->hasComponent<ColliderComponent>()) {
        auto& playerCollider =
            playerEntityPtr->getComponent<ColliderComponent>();

        if (Collision::AABB(collider->collider, playerCollider.collider)) {
            playerManager->addExperience(experienceAmount);
            collected = true;
            if (entity) {
                entity->destroy();
            }
        }
    }
}