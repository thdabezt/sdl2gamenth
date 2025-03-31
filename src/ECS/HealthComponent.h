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
        if (health <= 0) {
            health = 0;
            // Handle entity death here (e.g., destroy the entity)
            entity->destroy();
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