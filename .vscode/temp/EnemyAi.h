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
    
    // Contact damage properties
    int contactDamage;
    Uint32 lastDamageTime = 0;
    const Uint32 damageInterval = 1000; // Damage cooldown in ms (1 second)
    int expValue;
    Vector2D* playerPosition; // Pointer to the player's position
    Entity* playerEntity; // Reference to player entity for damage

public:
    EnemyAIComponent(int range, float moveSpeed, Vector2D* playerPos, int damage = 10, int exp = 1, Entity* player = nullptr)
        : detectionRange(range), 
          speed(moveSpeed), 
          playerPosition(playerPos),
          contactDamage(damage),
          expValue(exp),
          playerEntity(player) {}
    int getExpValue() const { return expValue; }
    void setExpValue(int value) { expValue = value; }
          
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

        // Check for player collision to apply damage
        if (playerEntity && playerEntity->hasComponent<ColliderComponent>() && 
            playerEntity->hasComponent<HealthComponent>()) {
            
            SDL_Rect playerCol = playerEntity->getComponent<ColliderComponent>().collider;
            
            // If enemy is colliding with player, apply damage on interval
            if (Collision::AABB(collider->collider, playerCol)) {
                Uint32 currentTime = SDL_GetTicks();
                if (currentTime > lastDamageTime + damageInterval) {
                    // Apply damage to player
                    playerEntity->getComponent<HealthComponent>().takeDamage(contactDamage);
                    
                    // // Flash the player sprite if it has a SpriteComponent
                    // if (playerEntity->hasComponent<SpriteComponent>()) {
                    //     playerEntity->getComponent<SpriteComponent>().isHit = true;
                    //     playerEntity->getComponent<SpriteComponent>().hitTime = currentTime;
                    // }
                    
                    // // Apply knockback to the player
                    // if (playerEntity->hasComponent<TransformComponent>()) {
                    //     // Calculate direction from enemy to player
                    //     Vector2D knockbackDir = playerEntity->getComponent<TransformComponent>().position - transform->position;
                    //     knockbackDir = knockbackDir.Normalize();
                        
                    //     // Apply knockback
                    //     float knockbackForce = 20.0f;
                    //     playerEntity->getComponent<TransformComponent>().position.x += knockbackDir.x * knockbackForce;
                    //     playerEntity->getComponent<TransformComponent>().position.y += knockbackDir.y * knockbackForce;
                    // }
                    
                    // Output damage info
                    // std::cout << "Player took " << contactDamage << " damage! Health: " 
                    //           << playerEntity->getComponent<HealthComponent>().getHealth() << "/"
                    //           << playerEntity->getComponent<HealthComponent>().getMaxHealth() << std::endl;
                              
                    lastDamageTime = currentTime;
                }
            }
        }

        // Regular enemy movement AI
        SDL_Rect playerRect = {static_cast<int>(playerPosition->x), static_cast<int>(playerPosition->y), 32, 32};
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
    
    // Getter/setter for contact damage
    int getContactDamage() const { return contactDamage; }
    void setContactDamage(int damage) { contactDamage = damage; }
};