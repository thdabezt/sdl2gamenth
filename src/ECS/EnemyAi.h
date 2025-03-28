#pragma once

#include "ECS.h"
#include "../Collision.h"
#include "../Vector2D.h"
#include "../game.h"
#include "Components.h"

class EnemyAIComponent : public Component {
private:
    TransformComponent* transform;
    ColliderComponent* collider;
    SpriteComponent* sprite; // Add reference to sprite component
    SDL_Rect detectionRect;
    int detectionRange;
    float speed;

    Vector2D* playerPosition; // Pointer to the player's position

public:
    EnemyAIComponent(int range, float moveSpeed, Vector2D* playerPos)
        : detectionRange(range), speed(moveSpeed), playerPosition(playerPos) {}

    void init() override {
        if (!entity->hasComponent<TransformComponent>()) {
            entity->addComponent<TransformComponent>();
        }
        if (!entity->hasComponent<ColliderComponent>()) {
            entity->addComponent<ColliderComponent>("enemy");
        }

        transform = &entity->getComponent<TransformComponent>();
        collider = &entity->getComponent<ColliderComponent>();
        
        // Get sprite component if it exists
        if (entity->hasComponent<SpriteComponent>()) {
            sprite = &entity->getComponent<SpriteComponent>();
        } else {
            sprite = nullptr; // No sprite component found
        }

        // Initialize the detection rectangle
        detectionRect.w = collider->collider.w + detectionRange * 2;
        detectionRect.h = collider->collider.h + detectionRange * 2;
    }

    void update() override {
        detectionRect.x = collider->collider.x - (detectionRect.w - collider->collider.w) / 2;
        detectionRect.y = collider->collider.y - (detectionRect.h - collider->collider.h) / 2;

        SDL_Rect playerRect = {static_cast<int>(playerPosition->x), static_cast<int>(playerPosition->y), 32, 32}; // Assuming player size is 32x32
        if (Collision::AABB(playerRect, detectionRect)) {
            Vector2D direction = *playerPosition - transform->position;
            direction = direction.Normalize();

            transform->velocity = direction * speed;
            
            // Update sprite flip based on movement direction (if sprite exists)
            if (sprite) {
                // Flip sprite based on horizontal movement
                if (transform->velocity.x < 0) {
                    sprite->spriteFlip = SDL_FLIP_HORIZONTAL; // Moving left
                } else if (transform->velocity.x > 0) {
                    sprite->spriteFlip = SDL_FLIP_NONE; // Moving right
                }
            }
        } else {
            transform->velocity.Zero();
        }

        transform->position.x += transform->velocity.x;
        transform->position.y += transform->velocity.y;
    }

    void draw() override {
        // Draw the detection rectangle (white)
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
        SDL_Rect debugDetection = {
            detectionRect.x - Game::camera.x,
            detectionRect.y - Game::camera.y,
            detectionRect.w,
            detectionRect.h
        };
        SDL_RenderDrawRect(Game::renderer, &debugDetection);

        // Reset the render color to white
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    }
};