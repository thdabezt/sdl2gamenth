#pragma once

#include "ECS.h"
#include "../Collision.h"
#include "../Vector2D.h"
#include "../game.h"      
#include "Components.h"  
#include <iostream>      
#include <SDL_rect.h>    
#include <SDL_timer.h>   

class TransformComponent;
class ColliderComponent;
class SpriteComponent;
class HealthComponent;

class EnemyAIComponent : public Component {
private:

    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    SpriteComponent* sprite = nullptr;

    SDL_Rect detectionRect;
    int detectionRange;
    float speed;

    int contactDamage;
    int expValue;
    Entity* playerEntity = nullptr;
    Vector2D* playerPosition = nullptr; 

    Uint32 lastDamageTime = 0;
    const Uint32 damageInterval = 1000; 
    bool initialized = false;

public:

    EnemyAIComponent(int range, float moveSpeed, Vector2D* playerTransformPosPtr, int damage = 10, int exp = 1, Entity* player = nullptr)
        : detectionRange(range > 0 ? range : 100),
          speed(moveSpeed >= 0.0f ? moveSpeed : 1.0f), 
          contactDamage(damage > 0 ? damage : 1),
          expValue(exp >= 0 ? exp : 0),
          playerEntity(player),
          playerPosition(playerTransformPosPtr)
          {}

    int getExpValue() const { return expValue; }
    void setExpValue(int value) { expValue = std::max(0, value); }

    void init() override {
         initialized = false;
         if (!entity) { std::cerr << "Error in EnemyAIComponent::init: Entity is null!" << std::endl; return; }

        if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in EnemyAIComponent::init: Enemy entity missing TransformComponent!" << std::endl; return; }
        transform = &entity->getComponent<TransformComponent>();

        if (!entity->hasComponent<ColliderComponent>()) { std::cerr << "Error in EnemyAIComponent::init: Enemy entity missing ColliderComponent!" << std::endl; return; }
        collider = &entity->getComponent<ColliderComponent>();

        if (entity->hasComponent<SpriteComponent>()) { sprite = &entity->getComponent<SpriteComponent>(); }
        else { sprite = nullptr; }

        if (!transform) { std::cerr << "Error in EnemyAIComponent::init: Failed to get transform pointer!" << std::endl; return; }
        if (!collider) { std::cerr << "Error in EnemyAIComponent::init: Failed to get collider pointer!" << std::endl; return; }
        if (!playerPosition) { std::cerr << "Error in EnemyAIComponent::init: Player Position pointer is null!" << std::endl; return; }
        if (!playerEntity) { std::cerr << "Error in EnemyAIComponent::init: Player Entity pointer is null!" << std::endl; return; }

        detectionRect.w = collider->collider.w + detectionRange * 2;
        detectionRect.h = collider->collider.h + detectionRange * 2;

        initialized = true;
    }

    void update() override {
        if (!initialized || !transform || !collider || !playerPosition || !playerEntity) return;

        if (!playerEntity->isActive() || !playerEntity->hasComponent<ColliderComponent>() || !playerEntity->hasComponent<HealthComponent>() || !playerEntity->hasComponent<TransformComponent>()) {
            transform->velocity.Zero();
            return;
        }

        Vector2D playerActualPos = playerEntity->getComponent<TransformComponent>().position;
        SDL_Rect playerColRect = playerEntity->getComponent<ColliderComponent>().collider;

        detectionRect.x = collider->collider.x - detectionRange;
        detectionRect.y = collider->collider.y - detectionRange;

        if (Collision::AABB(collider->collider, playerColRect)) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime >= lastDamageTime + damageInterval) {
                playerEntity->getComponent<HealthComponent>().takeDamage(contactDamage);
                lastDamageTime = currentTime;
                 if (playerEntity->hasComponent<SpriteComponent>()) {
                     playerEntity->getComponent<SpriteComponent>().isHit = true;
                     playerEntity->getComponent<SpriteComponent>().hitTime = currentTime;
                 }
            }
        }

        if (Collision::AABB(playerColRect, detectionRect)) {

            Vector2D direction = playerActualPos - transform->position;

            if (direction.x != 0.0f || direction.y != 0.0f) {
                 direction = direction.Normalize();
            } else {
                direction.Zero(); 
            }
            transform->velocity = direction * speed;

            if (sprite) {
                if (transform->velocity.x < -0.01f) {
                    sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
                } else if (transform->velocity.x > 0.01f) {
                    sprite->spriteFlip = SDL_FLIP_NONE;
                }
            }
        } else {

            transform->velocity.Zero();
        }

        transform->position.x += transform->velocity.x;
        transform->position.y += transform->velocity.y;

    }

    void draw() override {

        bool debug_draw = false;
        if (!debug_draw || !initialized || !Game::renderer || !collider) return;

        SDL_SetRenderDrawColor(Game::renderer, 0, 255, 255, 100); 
        SDL_Rect debugDetectionRect = {
            detectionRect.x - Game::camera.x,
            detectionRect.y - Game::camera.y,
            detectionRect.w,
            detectionRect.h
        };
        SDL_RenderDrawRect(Game::renderer, &debugDetectionRect);
    }

    int getContactDamage() const { return contactDamage; }
    void setContactDamage(int damage) { contactDamage = std::max(0, damage); }

}; 