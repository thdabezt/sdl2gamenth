#pragma once

#include <algorithm>
#include <iostream>
#include <string>

#include "Components.h"

class HealthComponent : public Component {
   public:
    int health;
    int maxHealth;

    HealthComponent(int h, int maxH)
        : health(h), maxHealth(maxH > 0 ? maxH : 1) {
        health = std::min(health, maxHealth);
        health = std::max(0, health);
    }

    void init() override {}
    void update() override {}

    void takeDamage(int damage) {
        if (damage <= 0) return;

        health -= damage;

        if (health <= 0) {
            health = 0;

            if (entity != nullptr &&
                entity->hasComponent<ColliderComponent>()) {
                const std::string& tag =
                    entity->getComponent<ColliderComponent>().tag;
                if (tag != "player") {
                    entity->destroy();
                }
            } else if (entity != nullptr) {
                entity->destroy();
            }
        }
    }

    void setMaxHealth(int newMax) {
        maxHealth = std::max(1, newMax);
        health = std::min(health, maxHealth);
    }

    void setHealth(int newHealth) { health = newHealth; }

    void heal(int healAmount) {
        if (healAmount <= 0) return;
        health += healAmount;
        health = std::min(health, maxHealth);
    }

    int getMaxHealth() const { return maxHealth; }
    int getHealth() const { return health; }
};