#pragma once
#include "../game.h"
#include "ECS.h"
#include "Components.h"
#include "../Scene/SceneManager.h" // Included for scene switching
#include "../constants.h"
#include <iostream> // For error logging

class KeyboardController : public Component {
private:
    bool initialized = false; // <<< ADD Initialization Flag

public:
    TransformComponent* transform = nullptr; // Initialize to nullptr
    SpriteComponent* sprite = nullptr;       // Initialize to nullptr

    // Track last direction
    enum Direction { LEFT, RIGHT };
    Direction lastDirection = RIGHT;

    void init() override {
        initialized = false; // Reset flag
        if (!entity) {
             std::cerr << "Error in KeyboardController::init: Entity is null!" << std::endl;
             return;
        }
        if (!entity->hasComponent<TransformComponent>()) {
            std::cerr << "Error in KeyboardController::init: Entity missing TransformComponent!" << std::endl;
            return;
        }
        if (!entity->hasComponent<SpriteComponent>()) {
             std::cerr << "Error in KeyboardController::init: Entity missing SpriteComponent!" << std::endl;
             return;
        }

        transform = &entity->getComponent<TransformComponent>();
        sprite = &entity->getComponent<SpriteComponent>();

        if (!transform) {
            std::cerr << "Error in KeyboardController::init: Failed to get TransformComponent pointer!" << std::endl;
            return; // Fail init if transform is missing
        }
        if (!sprite) {
             std::cerr << "Error in KeyboardController::init: Failed to get SpriteComponent pointer!" << std::endl;
             return; // Fail init if sprite is missing
        }
        initialized = true; // <<< SET Flag on success
    }

    void update() override {
        if (!initialized) return; // <<< CHECK Flag
        // Keep internal checks as fallback safety
        if (!transform || !sprite) return;

        Vector2D prevVelocity = transform->velocity; // Now safer

        handleKeyEvents(); // Handles non-movement keys

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        transform->velocity.x = 0;
        transform->velocity.y = 0;

        if (keystate[SDL_SCANCODE_W]) transform->velocity.y = -1;
        if (keystate[SDL_SCANCODE_A]) { transform->velocity.x = -1; lastDirection = LEFT; }
        if (keystate[SDL_SCANCODE_D]) { transform->velocity.x = 1; lastDirection = RIGHT; }
        if (keystate[SDL_SCANCODE_S]) transform->velocity.y = 1;

        if (transform->velocity.x != 0 || transform->velocity.y != 0) {
            if (transform->velocity.x != 0 && transform->velocity.y != 0) {
                transform->velocity = transform->velocity.Normalize();
            }
            transform->velocity = transform->velocity * playerSpeed;
        }
        updateAnimation(transform->velocity.x != 0 || transform->velocity.y != 0);
    }

private:
    void handleKeyEvents() {
        // Check only required if init didn't succeed, but redundant if update checks initialized flag
        // if (!initialized) return;
        if (Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
            // case SDLK_ESCAPE:
            //      if (Game::instance && SceneManager::instance) {
            //          SceneManager::instance->switchToScene(SceneType::Menu);
            //      }
            //     break;
            default: break;
            }
        }
    }

    void updateAnimation(bool isMoving) {
         if (!initialized || !sprite) return; // Check flag and sprite pointer

        sprite->spriteFlip = (lastDirection == LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        sprite->Play(isMoving ? "Walk" : "Idle");
    }
};