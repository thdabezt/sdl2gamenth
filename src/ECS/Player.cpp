#include "Player.h"
#include <iostream>

void Player::addExperience(int exp) {
    experience += exp;
    
    // Check if player has enough experience to level up
    while (experience >= experienceToNextLevel) {
        // Level up!
        experience -= experienceToNextLevel;
        levelUp();
    }
}

void Player::levelUp() {
    level++;
    
    // Calculate new experience required for next level (increases with each level)
    experienceToNextLevel = level * 10;
    
    // Upgrade stats
    if (playerEntity->hasComponent<HealthComponent>()) {
        auto& health = playerEntity->getComponent<HealthComponent>();
        int maxHealthIncrease = level * 5; // 5 health per level
        health.setMaxHealth(health.getMaxHealth() + maxHealthIncrease);
        health.heal(maxHealthIncrease); // Heal by the amount increased
    }
    
    // Log level up message
    std::cout << "LEVEL UP! Now level " << level 
              << ", +" << level * 5 << " Max Health" << std::endl;
}



int Player::getHealth() const {
    if (playerEntity->hasComponent<HealthComponent>()) {
        return playerEntity->getComponent<HealthComponent>().getHealth();
    }
    return 0;
}

int Player::getMaxHealth() const {
    if (playerEntity->hasComponent<HealthComponent>()) {
        return playerEntity->getComponent<HealthComponent>().getMaxHealth();
    }
    return 0;
}

int Player::getDamage() const {
    if (playerEntity->hasComponent<WeaponComponent>()) {
        return playerEntity->getComponent<WeaponComponent>().damage;
    }
    return 0;
}

float Player::getFireRate() const {
    if (playerEntity->hasComponent<WeaponComponent>()) {
        // Convert from delay (ms) to shots per second
        return 1000.0f / playerEntity->getComponent<WeaponComponent>().fireRate;
    }
    return 0.0f;
}

int Player::getProjectileCount() const {
    if (playerEntity->hasComponent<WeaponComponent>()) {
        return playerEntity->getComponent<WeaponComponent>().projectilesPerShot;
    }
    return 0;
}

float Player::getSpeed() const {
    if (playerEntity->hasComponent<TransformComponent>()) {
        return playerEntity->getComponent<TransformComponent>().speed;
    }
    return 0.0f;
}

Vector2D Player::getPosition() const {
    if (playerEntity->hasComponent<TransformComponent>()) {
        return playerEntity->getComponent<TransformComponent>().position;
    }
    return Vector2D();
}