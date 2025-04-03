#pragma once
#include "ECS.h"
#include "../Collision.h"
#include "../Vector2D.h"
#include "../game.h"
#include "Components.h"
#include <iostream> // For error logging

class EnemyAIComponent : public Component {
private:
    TransformComponent* transform = nullptr; // Enemy's transform
    ColliderComponent* collider = nullptr;   // Enemy's collider
    SpriteComponent* sprite = nullptr;       // Enemy's sprite (optional)
    SDL_Rect detectionRect;
    int detectionRange;
    float speed;

    // Contact damage properties
    int contactDamage;
    Uint32 lastDamageTime = 0;
    const Uint32 damageInterval = 1000;
    int expValue;

    // --- Player Data Pointers ---
    Vector2D* playerPosition = nullptr; // Pointer to the player's COLLIDER position
    Entity* playerEntity = nullptr;     // Pointer to the player entity (for damage/checks)

    bool initialized = false; // <<< ADD Initialization Flag

public:
    EnemyAIComponent(int range, float moveSpeed, Vector2D* playerColliderPosPtr, int damage = 10, int exp = 1, Entity* player = nullptr)
        : detectionRange(range),
          speed(moveSpeed),
          playerPosition(playerColliderPosPtr),
          contactDamage(damage),
          expValue(exp),
          playerEntity(player) {}

    int getExpValue() const { return expValue; }
    void setExpValue(int value) { expValue = value; }

    void init() override {
         initialized = false; // Reset flag
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

        initialized = true; // <<< SET Flag on success
    }

    void update() override {
        if (!initialized) return; // <<< CHECK Flag
        // Keep internal checks too
        if (!transform || !collider || !playerPosition || !playerEntity ) return;
        if (!playerEntity->isActive() || !playerEntity->hasComponent<ColliderComponent>() || !playerEntity->hasComponent<HealthComponent>()) return;


        // --- Get Player's ACTUAL Transform position (if available) ---
        Vector2D playerActualPos;
        if(playerEntity && playerEntity->hasComponent<TransformComponent>()) {
            playerActualPos = playerEntity->getComponent<TransformComponent>().position;
            // Optional: Adjust to player center if needed
            // playerActualPos.x += playerEntity->getComponent<TransformComponent>().width * playerEntity->getComponent<TransformComponent>().scale / 2.0f;
            // playerActualPos.y += playerEntity->getComponent<TransformComponent>().height * playerEntity->getComponent<TransformComponent>().scale / 2.0f;
        } else {
            // Fallback or error handling if player transform is missing
            playerActualPos = *playerPosition; // Use the old pointer as fallback? Risky.
            // Or maybe stop movement: transform->velocity.Zero(); return;
        }
        // --- End Get Player Position ---

        // Update detection rect position
        detectionRect.x = collider->collider.x - (detectionRect.w - collider->collider.w) / 2;
        detectionRect.y = collider->collider.y - (detectionRect.h - collider->collider.h) / 2;

        // --- Apply Contact Damage ---
        SDL_Rect playerColRect = playerEntity->getComponent<ColliderComponent>().collider;
        if (Collision::AABB(collider->collider, playerColRect)) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime > lastDamageTime + damageInterval) {
                playerEntity->getComponent<HealthComponent>().takeDamage(contactDamage);
                lastDamageTime = currentTime;
                 if (playerEntity->hasComponent<SpriteComponent>()) {
                     playerEntity->getComponent<SpriteComponent>().isHit = true;
                     playerEntity->getComponent<SpriteComponent>().hitTime = currentTime;
                 }
            }
        }

        // --- Movement AI ---
        SDL_Rect playerRect = {static_cast<int>(playerPosition->x), static_cast<int>(playerPosition->y), 32, 32};
        if (Collision::AABB(playerRect, detectionRect)) {
            Vector2D direction = playerActualPos - transform->position;
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
        // if (!initialized) return; // Optional check
        bool debug = false;
        if (!debug || !Game::renderer || !collider) return;

        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
        SDL_Rect debugDetection = { detectionRect.x - Game::camera.x, detectionRect.y - Game::camera.y, detectionRect.w, detectionRect.h };
        SDL_RenderDrawRect(Game::renderer, &debugDetection);
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    }

    int getContactDamage() const { return contactDamage; }
    void setContactDamage(int damage) { contactDamage = damage; }
};