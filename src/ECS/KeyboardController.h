#pragma once

// --- Includes ---
#include "../game.h"          // Provides Game::event access
#include "ECS.h"            // Provides Component base class, Entity
#include "Components.h"     // Provides TransformComponent, SpriteComponent definitions
#include "../constants.h"     // Provides playerSpeed
#include <iostream>          // For std::cerr error logging
#include <SDL_scancode.h>    // For SDL_SCANCODE_* definitions
#include <SDL_keyboard.h>    // For SDL_GetKeyboardState
#include "../Scene/SceneManager.h" // Must Include

// --- Class Definition ---

// Handles player input for movement and updates animations accordingly.
class KeyboardController : public Component {
private:
    // --- Private Members ---
    TransformComponent* transform = nullptr; // Pointer to entity's transform
    SpriteComponent* sprite = nullptr;       // Pointer to entity's sprite

    // Enum to track horizontal facing direction for sprite flipping
    enum Direction { LEFT, RIGHT };
    Direction lastDirection = RIGHT; // Default facing direction

    bool initialized = false;          // Initialization flag

    // --- Private Methods ---
    // Updates the sprite's animation and flip based on movement state.
    void updateAnimation(bool isMoving) {
        if (!sprite) return; // Need sprite component to update animation

        // Flip sprite based on the last horizontal movement direction
        sprite->spriteFlip = (lastDirection == LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // Play "Walk" or "Idle" animation
        sprite->Play(isMoving ? "Walk" : "Idle");
    }

public:
    // --- Constructor ---
    KeyboardController() = default; // Use default constructor

    // --- Public Methods ---

    // Component Lifecycle Overrides
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

        // Double-check pointers after assignment
        if (!transform) { std::cerr << "Error: Failed to get TransformComponent pointer in KeyboardController.\n"; return; }
        if (!sprite) { std::cerr << "Error: Failed to get SpriteComponent pointer in KeyboardController.\n"; return; }

        initialized = true;
    }

    void update() override {
        if (!initialized || !transform) return; // Check initialization and required transform

        // --- Movement Input Handling ---
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Vector2D desiredVelocity; // Store intended movement direction

        // Read WASD keys
        if (keystate[SDL_SCANCODE_W]) desiredVelocity.y = -1;
        if (keystate[SDL_SCANCODE_S]) desiredVelocity.y = 1;
        if (keystate[SDL_SCANCODE_A]) { desiredVelocity.x = -1; lastDirection = LEFT; }
        if (keystate[SDL_SCANCODE_D]) { desiredVelocity.x = 1; lastDirection = RIGHT; }

        // Apply velocity if movement keys are pressed
        if (desiredVelocity.x != 0.0f || desiredVelocity.y != 0.0f) {
            // Normalize to prevent faster diagonal movement and apply speed
            transform->velocity = desiredVelocity.Normalize() * playerSpeed;
        } else {
            transform->velocity.Zero(); // Stop movement if no keys are pressed
        }

        // --- Animation Update ---
        // Update animation based on whether the entity has non-zero velocity
        updateAnimation(transform->velocity.x != 0.0f || transform->velocity.y != 0.0f);
    }

    // void draw() override; // No drawing logic needed for this component

}; // End KeyboardController class