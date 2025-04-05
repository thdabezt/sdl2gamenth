#pragma once

#include <SDL_keyboard.h>
#include <SDL_scancode.h>

#include <iostream>

#include "../Scene/SceneManager.h"
#include "../constants.h"
#include "../game.h"
#include "Components.h"
#include "ECS.h"

class KeyboardController : public Component {
   private:
    TransformComponent* transform = nullptr;
    SpriteComponent* sprite = nullptr;

    enum Direction { LEFT, RIGHT };
    Direction lastDirection = RIGHT;

    bool initialized = false;

    void updateAnimation(bool isMoving) {
        if (!sprite) return;

        sprite->spriteFlip =
            (lastDirection == LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        sprite->Play(isMoving ? "Walk" : "Idle");
    }

   public:
    KeyboardController() = default;

    void init() override {
        initialized = false;
        if (!entity) {
            std::cerr << "Error in KeyboardController::init: Entity is null!"
                      << std::endl;
            return;
        }
        if (!entity->hasComponent<TransformComponent>()) {
            std::cerr << "Error in KeyboardController::init: Entity missing "
                         "TransformComponent!"
                      << std::endl;
            return;
        }
        if (!entity->hasComponent<SpriteComponent>()) {
            std::cerr << "Error in KeyboardController::init: Entity missing "
                         "SpriteComponent!"
                      << std::endl;
            return;
        }

        transform = &entity->getComponent<TransformComponent>();
        sprite = &entity->getComponent<SpriteComponent>();

        if (!transform) {
            std::cerr << "Error: Failed to get TransformComponent pointer in "
                         "KeyboardController.\n";
            return;
        }
        if (!sprite) {
            std::cerr << "Error: Failed to get SpriteComponent pointer in "
                         "KeyboardController.\n";
            return;
        }

        initialized = true;
    }

    void update() override {
        if (!initialized || !transform) return;

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Vector2D desiredVelocity;

        if (keystate[SDL_SCANCODE_W]) desiredVelocity.y = -1;
        if (keystate[SDL_SCANCODE_S]) desiredVelocity.y = 1;
        if (keystate[SDL_SCANCODE_A]) {
            desiredVelocity.x = -1;
            lastDirection = LEFT;
        }
        if (keystate[SDL_SCANCODE_D]) {
            desiredVelocity.x = 1;
            lastDirection = RIGHT;
        }

        if (desiredVelocity.x != 0.0f || desiredVelocity.y != 0.0f) {
            transform->velocity = desiredVelocity.Normalize() * playerSpeed;
        } else {
            transform->velocity.Zero();
        }

        updateAnimation(transform->velocity.x != 0.0f ||
                        transform->velocity.y != 0.0f);
    }
};