#pragma once

#include "ECS.h"
#include "../Vector2D.h"
#include <cmath>
#include <iostream>
#include "../game.h"
#include <set>
#include <algorithm>
#include "Components.h" // Include Components.h for TransformComponent definition

class ProjectileComponent : public Component {
private:
    TransformComponent *transform = nullptr; // Pointer now checked
    Vector2D startPos;
    int damage = 0;
    Vector2D velocity;
    int maxPierce = 1;
    std::set<Entity*> hitEnemies;

    bool initialized = false; // <<< ADD Initialization Flag

public:
    ProjectileComponent( int dmg, Vector2D vel, int pierce = 1)
        :  damage(dmg), velocity(vel), maxPierce(pierce > 0 ? pierce : 1) {}

    ~ProjectileComponent() {}

    void init() override {
        initialized = false; // Reset flag
        if (!entity) { std::cerr << "Error in ProjectileComponent::init: Entity is null!" << std::endl; return; }
        if (!entity->hasComponent<TransformComponent>()) { std::cerr << "Error in ProjectileComponent::init: Missing TransformComponent!" << std::endl; if (entity) entity->destroy(); return; }
        transform = &entity->getComponent<TransformComponent>();

        if (!transform) { std::cerr << "Error in ProjectileComponent::init: Failed to get TransformComponent pointer!" << std::endl; if (entity) entity->destroy(); return; }

        startPos = transform->position;
        initialized = true; // <<< SET Flag on success
    }

    int getDamage() const { return damage; }

    bool hasHit(Entity* enemy) const {
        return hitEnemies.count(enemy) > 0;
    }

    void recordHit(Entity* enemy) {
        hitEnemies.insert(enemy);
    }

    bool shouldDestroy() const {
        return hitEnemies.size() >= static_cast<decltype(hitEnemies.size())>(maxPierce);
    }

    void update() override {
        if (!initialized) return; // <<< CHECK Flag
        if (!transform) { if (entity) entity->destroy(); return; } // Keep internal check

        transform->position.x += velocity.x;
        transform->position.y += velocity.y;

        

        // Check out-of-bounds relative to camera + buffer
        if (transform->position.x > Game::camera.x + Game::camera.w + 100 ||
            transform->position.x < Game::camera.x - 100 ||
            transform->position.y > Game::camera.y + Game::camera.h + 100 ||
            transform->position.y < Game::camera.y - 100)
        {
             if (entity) entity->destroy();
             return;
        }
    }
    
};