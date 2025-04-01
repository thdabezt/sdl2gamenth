#pragma once

#include "Components.h"

class HealthComponent : public Component {
public:
    int health;
    int maxHealth;

    HealthComponent(int h, int maxH) : health(h), maxHealth(maxH) {}

    void init() override {}

    void update() override {}
    void renderHealthBar(Entity& entity, Vector2D position);
    void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) { // Check if health dropped to 0 or below
            health = 0;

            // Check if this entity should be destroyed (i.e., it's NOT the player)
            if (entity != nullptr && entity->hasComponent<ColliderComponent>()) {
                const std::string& tag = entity->getComponent<ColliderComponent>().tag;
                if (tag != "player") {
                    // It's not the player, so destroy it immediately
                    entity->destroy();
                    // std::cout << tag << " destroyed by HealthComponent." << std::endl; // Optional debug
                }
            } 
        }
    }
    void setMaxHealth(int newMax) {
        maxHealth = newMax;
        // Make sure current health doesn't exceed maximum
        if (health > maxHealth) {
            health = maxHealth;
        }
    }
    void setHealth(int Health) {
        health = Health;
        // Make sure current health doesn't exceed maximum
        // if (health > maxHealth) {
        //     health = maxHealth;
        // }
    }
    int getMaxHealth() const {
        return maxHealth;
    }
    int getHealth() const{
        return health;
    }
    void heal(int healAmount) {
        health += healAmount;
        if (health > maxHealth) {
            health = maxHealth;
        }
    }
};