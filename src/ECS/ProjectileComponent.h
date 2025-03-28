#pragma once

#include "ECS.h"
#include "Components.h"
#include "../Vector2D.h"

class ProjectileComponent : public Component {
public:
    ProjectileComponent(int rng, int dmg, Vector2D vel) 
        : range(rng), damage(dmg), velocity(vel) {}
    
    ~ProjectileComponent() {}

    void init() override {
        transform = &entity->getComponent<TransformComponent>();
        transform->velocity = velocity;
        std::cout << transform->position << std::endl;
    }
    
    // Add getter for damage
    int getDamage() const { return damage; }
    
    void update() override {
        // Calculate the distance traveled in this frame
        float frameDistance = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        distance += frameDistance;
    
        // Check if the projectile has exceeded its range
        if (distance > range) {
            entity->destroy();
            return;
        }
    
        // Check if the projectile is out of bounds
        if (transform->position.x > Game::camera.x + Game::camera.w ||
            transform->position.x < Game::camera.x ||
            transform->position.y > Game::camera.y + Game::camera.h ||
            transform->position.y < Game::camera.y) {
            entity->destroy();
            return;
        }
    
        // Update the projectile's position
        transform->position.x += velocity.x;
        transform->position.y += velocity.y;
    }

private:
    TransformComponent *transform;
    int range = 0;
    int damage = 0;
    int distance = 0;
    Vector2D velocity;
};