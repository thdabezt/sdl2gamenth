#include "ExpOrbComponent.h"

// Include FULL definitions needed for the implementation
#include "Components.h"
#include "Player.h"           // Include Player.h for the Player class definition
#include "../game.h"          // Include game.h for Game instance and PlayerManager access
#include "../Collision.h"     // Include Collision.h for Collision::AABB implementation
#include <stdexcept>         // For std::runtime_error

// Constructor Definition
ExpOrbComponent::ExpOrbComponent(int exp) : experienceAmount(exp) {}

// init() Definition (can also be moved here, or kept inline in .h if simple)
void ExpOrbComponent::init() {
    if (!entity->hasComponent<TransformComponent>()) {
        entity->addComponent<TransformComponent>();
    }
    transform = &entity->getComponent<TransformComponent>();

    if (!entity->hasComponent<ColliderComponent>()) {
        entity->addComponent<ColliderComponent>("exp_orb", 32, 32);
    }
    collider = &entity->getComponent<ColliderComponent>();

    // Ensure group label is accessible (it's in Game class)
    entity->addGroup(Game::groupExpOrbs);
}


// update() Definition
void ExpOrbComponent::update() {
    if (collected) return;

    // Ensure collider is initialized (init should have run)
    if (!collider || !Game::instance) return;

    Entity* playerEntityPtr = nullptr;
    try {
        // Make sure Game::getPlayer() is accessible and returns a valid entity
        playerEntityPtr = &Game::instance->getPlayer();
    } catch (const std::runtime_error& e) {
        std::cerr << "ExpOrbComponent: Error getting player entity: " << e.what() << std::endl;
        return;
    }

    // Ensure player entity and its collider are valid
    if (playerEntityPtr && playerEntityPtr->isActive() && playerEntityPtr->hasComponent<ColliderComponent>()) {
        auto& playerCollider = playerEntityPtr->getComponent<ColliderComponent>();

        // --- Collision Check ---
        // Now, when this .cpp file is compiled, the full definitions of
        // ColliderComponent (from ColliderComponent.h) and Collision::AABB
        // (from Collision.cpp linked later) will be available.
        if (Collision::AABB(collider->collider, playerCollider.collider)) {
            std::cout << "Player collided with EXP orb! Granting " << experienceAmount << " EXP." << std::endl; // Debug

            Player* playerManager = Game::instance->getPlayerManager();
            if (playerManager) {
                playerManager->addExperience(experienceAmount);
            } else {
                std::cerr << "ExpOrbComponent: PlayerManager not found!" << std::endl;
            }

            collected = true;
            entity->destroy();
        }
    }
    
}
// void ExpOrbComponent::draw() {
//     // Only draw if the collider exists and we have a renderer
//     if (!collider || !Game::renderer) return;

//     // Get the collider rectangle
//     SDL_Rect debugRect = collider->collider;

//     // Adjust position based on camera
//     debugRect.x -= Game::camera.x;
//     debugRect.y -= Game::camera.y;

//     // Set draw color (e.g., yellow for EXP orbs)
//     SDL_SetRenderDrawColor(Game::renderer, 255, 255, 0, 255); // Yellow, fully opaque

//     // Draw the rectangle outline
//     SDL_RenderDrawRect(Game::renderer, &debugRect);

//     // Optional: Reset draw color back to default (e.g., white) if needed elsewhere
//     // SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
// }