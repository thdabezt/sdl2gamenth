#include "Player.h"
#include <iostream>
#include "../game.h" // Include game.h for Game::instance access if needed

// --- addExperience Implementation ---
void Player::addExperience(int exp) {
    experience += exp;

    // Check if player has enough experience to level up
    while (experienceToNextLevel > 0 && experience >= experienceToNextLevel) { // Added check for expToNext > 0
        // Level up!
        experience -= experienceToNextLevel;
        levelUp();
    }
     // Ensure experience doesn't somehow become negative after level up subtraction
     if (experience < 0) experience = 0;
}

// --- levelUp Implementation ---
void Player::levelUp() {
    level++;

    // Calculate new experience required for next level (increases with each level)
    experienceToNextLevel = level * 10; // Consider a more scaling formula if desired

    std::cout << "Player Leveled Up to Level " << level << "! Next level requires " << experienceToNextLevel << " EXP." << std::endl; // Feedback


    // Upgrade stats
    if (playerEntity->hasComponent<HealthComponent>()) {
        auto& health = playerEntity->getComponent<HealthComponent>();
        int maxHealthIncrease = level * 5; // 5 health per level
        health.setMaxHealth(health.getMaxHealth() + maxHealthIncrease);
        health.heal(maxHealthIncrease); // Heal by the amount increased
    }

    // --- Trigger Buff Selection ---
    // Trigger every level up for now, adjust frequency if needed (e.g., level % 2 == 0)
    if (Game::instance) { // Check if Game instance exists
         Game::instance->enterBuffSelection(); // Call function in Game class
    } else {
         std::cerr << "Player::levelUp - Game::instance is null, cannot enter buff selection!" << std::endl;
    }
}


// --- Stat Getters (No changes needed) ---
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
         int fireRateMs = playerEntity->getComponent<WeaponComponent>().fireRate;
         return (fireRateMs > 0) ? 1000.0f / fireRateMs : 0.0f; // Avoid division by zero
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
    return Vector2D(); // Return zero vector if no transform
}

// --- Implement Missing Setters ---

void Player::setLevel(int newLevel) {
    level = std::max(1, newLevel); // Ensure level doesn't go below 1
    // Recalculate expToNext based on the new level if needed
     experienceToNextLevel = level * 10; // Or use your scaling formula
     std::cout << "Player level set to: " << level << ", next level EXP: " << experienceToNextLevel << std::endl;
}

void Player::setExperience(int newExp) {
    experience = std::max(0, newExp); // Ensure experience doesn't go below 0
     std::cout << "Player experience set to: " << experience << std::endl;
}

void Player::setExperienceToNextLevel(int newExpToNext) {
    experienceToNextLevel = std::max(1, newExpToNext); // Ensure required EXP is at least 1
     std::cout << "Player EXP to next level set to: " << experienceToNextLevel << std::endl;
}

void Player::setEnemiesDefeated(int count) {
    enemiesDefeated = std::max(0, count); // Ensure count doesn't go below 0
     std::cout << "Player enemies defeated set to: " << enemiesDefeated << std::endl;
}
