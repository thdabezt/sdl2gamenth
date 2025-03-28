#pragma once

#include "ECS.h"       // <<< Include ECS.h for Entity type
#include "../Vector2D.h"
#include <cmath>
#include <iostream>
#include "../game.h"
#include <set>         // <<< Include set
#include <algorithm>   // <<< Include algorithm (might be needed for set operations later)

// Forward declare TransformComponent
class TransformComponent;

class ProjectileComponent : public Component {
public:
    ProjectileComponent(int rng, int dmg, Vector2D vel, int pierce = 1)
        : range(rng), damage(dmg), velocity(vel), maxPierce(pierce > 0 ? pierce : 1)
    {
        transform = nullptr;
        // enemiesHit count removed, use hitEnemies.size()
    }

    ~ProjectileComponent() {}

    void init() override {
        if (!entity->hasComponent<TransformComponent>()) {
            //  std::cerr << "ProjectileComponent Error: Missing TransformComponent during init!" << std::endl;
             entity->destroy();
             return;
        }
        transform = &entity->getComponent<TransformComponent>();
        startPos = transform->position;
    }

    int getDamage() const { return damage; }

    // --- Check if a specific enemy has been hit by this projectile ---
    bool hasHit(Entity* enemy) const {
        return hitEnemies.count(enemy) > 0; // Check if enemy pointer exists in the set
    }

    // --- Record that a specific enemy has been hit ---
    void recordHit(Entity* enemy) {
        hitEnemies.insert(enemy); // Add enemy pointer to the set
        // std::cout << "Projectile recorded hit on enemy: " << enemy << ". Total unique hits: " << hitEnemies.size() << "/" << maxPierce << std::endl; // Debug
    }

    // --- Check if projectile should be destroyed based on unique hits ---
    bool shouldDestroy() const {
        // Destroy if the number of unique enemies hit reaches the pierce limit
        return hitEnemies.size() >= static_cast<decltype(hitEnemies.size())>(maxPierce);
    }

    void update() override {
        // ... (update logic for movement, range, bounds remains the same as previous step) ...
        if (!transform) return;

        transform->position.x += velocity.x;
        transform->position.y += velocity.y;

        if (range > 0) {
            float distSq = (transform->position.x - startPos.x) * (transform->position.x - startPos.x) +
                           (transform->position.y - startPos.y) * (transform->position.y - startPos.y);
            if (distSq > (static_cast<float>(range) * range)) {
                entity->destroy();
                return;
            }
        }

        if (transform->position.x > Game::camera.x + Game::camera.w + 50 ||
            transform->position.x < Game::camera.x - 50 ||
            transform->position.y > Game::camera.y + Game::camera.h + 50 ||
            transform->position.y < Game::camera.y - 50) {
            entity->destroy();
            return;
        }
    }

private:
    TransformComponent *transform;
    Vector2D startPos;
    int range = 0;
    int damage = 0;
    Vector2D velocity;
    int maxPierce = 1;
    // int enemiesHit = 0; // Removed, use hitEnemies.size()
    std::set<Entity*> hitEnemies; // <<< ADDED: Set to store pointers to unique enemies hit
};