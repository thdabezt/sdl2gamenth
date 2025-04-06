// --- Includes ---
#include "Player.h"       // Corresponding header
#include "Components.h"   // Access to component definitions (Health, Weapon, Transform)
#include "../game.h"      // Access to Game::instance for triggering updates/spawns
#include "../constants.h" // Access to constants like playerSpeed
#include <iostream>      // For std::cerr error logging
#include <algorithm>     // For std::max
#include <stdexcept>     // For std::runtime_error

// --- Constructor ---

Player::Player(Entity* entity) : playerEntity(entity) {
    if (!playerEntity) {
        // This is a critical error, the Player manager cannot function without its entity.
        throw std::runtime_error("FATAL ERROR: Player created with null entity!");
    }
    // Initialize stats (redundant if using default member initializers in C++11+, but safe)
    level = 1;
    experience = 0;
    experienceToNextLevel = 10; // Initial requirement for level 2
    enemiesDefeated = 0;
    lifestealPercentage = 0.0f;
}

// --- Method Definitions ---

// --- Level System ---

void Player::addExperience(int exp) {
    if (exp <= 0 || level >= 999) return; // Ignore non-positive exp or if at max level

    experience += exp;
    // Handle multiple level ups if enough experience is gained at once
    while (experienceToNextLevel > 0 && experience >= experienceToNextLevel) {
        experience -= experienceToNextLevel; // Subtract cost of current level up
        levelUp();                           // Perform level up logic
        if (level >= 9999) { // Check level cap after potential level up
            experience = 0; // Cap experience at max level
            break;
        }
    }
    // Ensure experience doesn't somehow become negative after subtraction
    if (experience < 0) experience = 0;
}

void Player::levelUp() {
    level++;
    // Example formula: Experience needed increases linearly
    experienceToNextLevel = level * 10 + 5; // Adjust formula as desired

    // std::cout << "Player Leveled Up to Level " << level << "! Next: " << experienceToNextLevel << " EXP." << std::endl; // Removed log

    // Trigger spawn pool update in the Game instance
    if (Game::instance) {
        Game::instance->updateSpawnPoolAndWeights();
    } else {
        std::cerr << "Warning: Game::instance is null in Player::levelUp. Cannot update spawn pool." << std::endl;
    }

    // Trigger buff selection every 2 levels
    if (level % 2 == 0) {
        if (Game::instance) {
             Game::instance->enterBuffSelection();
        } else {
             std::cerr << "Warning: Player::levelUp - Game::instance is null, cannot enter buff selection!" << std::endl;
        }
    }

    // Trigger boss spawn every 10 levels
    if (level % 10 == 0) {
        if (Game::instance) {
            Game::instance->spawnBoss(); // Use the standard boss spawn function
        } else {
            std::cerr << "Warning: Game::instance is null in Player::levelUp. Cannot spawn boss." << std::endl;
        }
    }
}

int Player::getLevel() const { return level; }
int Player::getExperience() const { return experience; }
int Player::getExperienceToNextLevel() const { return experienceToNextLevel; }

float Player::getExperiencePercentage() const {
    // Avoid division by zero if somehow expToNextLevel is 0 or less
    return (experienceToNextLevel > 0) ? static_cast<float>(experience) / experienceToNextLevel : 0.0f;
}

// --- Stats ---

int Player::getEnemiesDefeated() const { return enemiesDefeated; }
void Player::incrementEnemiesDefeated() { enemiesDefeated++; }
float Player::getLifestealPercentage() const { return lifestealPercentage; }

void Player::setLifestealPercentage(float percentage) {
    lifestealPercentage = std::max(0.0f, percentage); // Ensure lifesteal isn't negative
}

// --- Entity Access ---

Entity& Player::getEntity() {
    if (!playerEntity) {
        throw std::runtime_error("Attempted to get null player entity in Player::getEntity()!");
    }
    return *playerEntity;
}

const Entity& Player::getEntity() const {
    if (!playerEntity) {
        throw std::runtime_error("Attempted to get null player entity in Player::getEntity() const!");
    }
    return *playerEntity;
}

// --- Component Shortcuts (Getters) ---

int Player::getHealth() const {
    if (playerEntity && playerEntity->hasComponent<HealthComponent>()) {
        return playerEntity->getComponent<HealthComponent>().getHealth();
    }
    return 0; // Return 0 or throw exception if component missing
}

int Player::getMaxHealth() const {
    if (playerEntity && playerEntity->hasComponent<HealthComponent>()) {
        return playerEntity->getComponent<HealthComponent>().getMaxHealth();
    }
    return 0; // Return 0 or throw exception if component missing
}

int Player::getDamage() const {
    if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) {
        return playerEntity->getComponent<WeaponComponent>().getDamage();
    }
    return 0;
}

float Player::getFireRate() const {
    if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) {
        int fr_ms = playerEntity->getComponent<WeaponComponent>().getFireRate();
        return (fr_ms > 0) ? 1000.0f / fr_ms : 0.0f; // Calculate shots per second
    }
    return 0.0f;
}

int Player::getProjectileCount() const {
    if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) {
        return playerEntity->getComponent<WeaponComponent>().getProjectileCount();
    }
    return 0;
}

float Player::getSpeed() const {
    return playerSpeed;
}

Vector2D Player::getPosition() const {
    if (playerEntity && playerEntity->hasComponent<TransformComponent>()) {
        return playerEntity->getComponent<TransformComponent>().position;
    }
    return Vector2D(); // Return zero vector if missing
}

// --- Actions ---

void Player::heal(int amount) {
    if (amount <= 0) return; // Ignore non-positive healing
    if (playerEntity && playerEntity->hasComponent<HealthComponent>()) {
        playerEntity->getComponent<HealthComponent>().heal(amount);
    } else {
        std::cerr << "Warning: Player::heal called but playerEntity or HealthComponent is missing!" << std::endl;
    }
}

// --- Setters (For Loading) ---

void Player::setLevel(int newLevel) {
    level = std::max(1, newLevel); // Ensure level is at least 1
    // Recalculate exp needed for next level based on the loaded level
    experienceToNextLevel = level * 10 + 5; // Use the same formula as levelUp
}

void Player::setExperience(int newExp) {
    experience = std::max(0, newExp); // Ensure experience is not negative
}

void Player::setExperienceToNextLevel(int newExpToNext) {
    // Only allow setting if it's positive, otherwise rely on level calculation
    if (newExpToNext > 0) {
        experienceToNextLevel = newExpToNext;
    } else {
        // Recalculate based on current level if loaded value is invalid
        experienceToNextLevel = level * 10 + 5;
    }
}

void Player::setEnemiesDefeated(int count) {
    enemiesDefeated = std::max(0, count);
}