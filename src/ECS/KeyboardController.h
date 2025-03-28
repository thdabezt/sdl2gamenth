// #pragma once
// #include "../game.h"
// #include "ECS.h"
// #include "Components.h"

// class KeyboardController : public Component {
// public:
//     TransformComponent* transform;
//     SpriteComponent* sprite;
    
//     // Track last direction
//     enum Direction { LEFT, RIGHT };
//     Direction lastDirection = RIGHT;

//     void init() override {
//         transform = &entity->getComponent<TransformComponent>();
//         sprite = &entity->getComponent<SpriteComponent>();
//     }

//     void update() override {
//         // Store previous velocity to detect changes
//         Vector2D prevVelocity = transform->velocity;

//         // Handle key events
//         handleKeyEvents();
        
//         // Update animation based on current velocity
//         updateAnimation();
//     }

// private:
//     void handleKeyEvents() {
//         if (Game::event.type == SDL_KEYDOWN) {
//             switch (Game::event.key.keysym.sym) {
//             case SDLK_w:
//                 transform->velocity.y = -1;
//                 break;
//             case SDLK_a:
//                 transform->velocity.x = -1;
//                 // Only update direction when explicitly moving left
//                 lastDirection = LEFT;
//                 break;
//             case SDLK_d:
//                 transform->velocity.x = 1;
//                 // Only update direction when explicitly moving right
//                 lastDirection = RIGHT;
//                 break;
//             case SDLK_s:
//                 transform->velocity.y = 1;
//                 break;
//             case SDLK_e:
//                 Game::instance->rezero();
//                 break;
//             default:
//                 break;
//             }
//         }
        
//         if (Game::event.type == SDL_KEYUP) {
//             switch (Game::event.key.keysym.sym) {
//             case SDLK_w:
//                 // Only zero out y velocity if we were moving up
//                 if (transform->velocity.y < 0)
//                     transform->velocity.y = 0;
//                 break;
//             case SDLK_a:
//                 // Only zero out x velocity if we were moving left
//                 if (transform->velocity.x < 0)
//                     transform->velocity.x = 0;
//                 break;
//             case SDLK_d:
//                 // Only zero out x velocity if we were moving right
//                 if (transform->velocity.x > 0)
//                     transform->velocity.x = 0;
//                 break;
//             case SDLK_s:
//                 // Only zero out y velocity if we were moving down
//                 if (transform->velocity.y > 0)
//                     transform->velocity.y = 0;
//                 break;
//             case SDLK_ESCAPE:
//                 Game::isRunning = false;
//                 break;
//             default:
//                 break;
//             }
//         }
//     }

//     void updateAnimation() {
//         // Apply appropriate sprite flip based on last direction
//         if (lastDirection == LEFT) {
//             sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
//         } else {
//             sprite->spriteFlip = SDL_FLIP_NONE;
//         }

//         // Check if player is moving (any non-zero velocity)
//         bool isMoving = transform->velocity.x != 0 || transform->velocity.y != 0;
        
//         // Update animation state based on movement
//         if (isMoving) {
//             sprite->Play("Walk");
//         } else {
//             sprite->Play("Idle");
//         }
//     }
// };

#pragma once
#include "../game.h"
#include "ECS.h"
#include "Components.h"
#include "../Scene/SceneManager.h"
class KeyboardController : public Component {
public:
    TransformComponent* transform;
    SpriteComponent* sprite;
    
    // Track last direction
    enum Direction { LEFT, RIGHT };
    Direction lastDirection = RIGHT;

    void init() override {
        transform = &entity->getComponent<TransformComponent>();
        sprite = &entity->getComponent<SpriteComponent>();
    }

    void update() override {
        // Store previous velocity to detect changes
        Vector2D prevVelocity = transform->velocity;
        
        // Handle key events for one-time actions
        handleKeyEvents();
        
        // ADDED: Check keyboard state for movement (more reliable)
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        
        // Reset velocity each frame
        transform->velocity.x = 0;
        transform->velocity.y = 0;
        
        // Set velocity based on current key state
        if (keystate[SDL_SCANCODE_W]) {
            transform->velocity.y = -1;
        }
        if (keystate[SDL_SCANCODE_A]) {
            transform->velocity.x = -1;
            lastDirection = LEFT;
        }
        if (keystate[SDL_SCANCODE_D]) {
            transform->velocity.x = 1;
            lastDirection = RIGHT;
        }
        if (keystate[SDL_SCANCODE_S]) {
            transform->velocity.y = 1;
        }
        
        // Update animation based on current velocity
        updateAnimation();
    }

private:
    void handleKeyEvents() {
        // Only handle one-time events here
        if (Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
            // case SDLK_e:
            //     Game::instance->rezero();
            //     break;
            case SDLK_ESCAPE:
                // Don't quit game, just signal scene change
                SceneManager::instance->switchToScene(SceneType::Menu);
                break;
            default:
                break;
            }
        }
    }

    void updateAnimation() {
        // Apply appropriate sprite flip based on last direction
        if (lastDirection == LEFT) {
            sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
        } else {
            sprite->spriteFlip = SDL_FLIP_NONE;
        }

        // Check if player is moving (any non-zero velocity)
        bool isMoving = transform->velocity.x != 0 || transform->velocity.y != 0;
        
        // Update animation state based on movement
        if (isMoving) {
            sprite->Play("Walk");
        } else {
            sprite->Play("Idle");
        }
    }
};