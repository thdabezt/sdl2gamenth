#pragma once

#include <cmath>
#include <iostream>
#include <set>

#include "../Vector2D.h"
#include "../game.h"
#include "Components.h"
#include "ECS.h"

class TransformComponent;

class ProjectileComponent : public Component {
   private:
    TransformComponent* transform = nullptr;
    Vector2D startPos;
    int damage = 0;
    Vector2D velocity;
    int maxPierce = 1;
    std::set<Entity*> hitEnemies;
    bool initialized = false;

   public:
    bool isSpinning = false;

    ProjectileComponent(int dmg, Vector2D vel, int pierce = 1)
        : damage(dmg), velocity(vel), maxPierce(pierce > 0 ? pierce : 1) {}

    ~ProjectileComponent() override = default;

    void init() override {
        initialized = false;
        if (!entity) {
            std::cerr << "Error in ProjectileComponent::init: Entity is null!"
                      << std::endl;
            return;
        }
        if (!entity->hasComponent<TransformComponent>()) {
            std::cerr << "Error in ProjectileComponent::init: Missing "
                         "TransformComponent!"
                      << std::endl;

            return;
        }
        transform = &entity->getComponent<TransformComponent>();
        if (!transform) {
            std::cerr << "Error in ProjectileComponent::init: Failed to get "
                         "TransformComponent pointer!"
                      << std::endl;

            return;
        }

        startPos = transform->position;
        initialized = true;
    }

    void update() override {
        if (!initialized || !transform) {
            if (entity) entity->destroy();
            return;
        }

        transform->position += velocity;

        if (transform->position.x > Game::camera.x + Game::camera.w + 100 ||
            transform->position.x + transform->width * transform->scale <
                Game::camera.x - 100 ||
            transform->position.y > Game::camera.y + Game::camera.h + 100 ||
            transform->position.y + transform->height * transform->scale <
                Game::camera.y - 100) {
            if (entity) entity->destroy();
            return;
        }
    }

    int getDamage() const { return damage; }

    bool hasHit(Entity* enemy) const { return hitEnemies.count(enemy) > 0; }

    void recordHit(Entity* enemy) {
        if (enemy) {
            hitEnemies.insert(enemy);
        }
    }

    bool shouldDestroy() const {
        return hitEnemies.size() >=
               static_cast<std::set<Entity*>::size_type>(maxPierce);
    }
};