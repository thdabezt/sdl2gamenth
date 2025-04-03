#include "Player.h" // Include the header file
#include <iostream>
#include <algorithm> // For std::max
#include "../game.h" // Include game.h for Game::instance if needed
#include "Components.h" // Include Components.h for component access

// --- Constructor Definition ---
Player::Player(Entity* entity) : playerEntity(entity) { // Definition starts here
    if (!playerEntity) {
        std::cerr << "FATAL ERROR: Player created with null entity!" << std::endl;
        // Consider throwing an exception
    }
    // Initialize stats
    level = 1;
    experience = 0;
    experienceToNextLevel = 10;
    enemiesDefeated = 0;
    lifestealPercentage = 0.0f;
} // Definition ends here

// --- addExperience ---
void Player::addExperience(int exp) {
    if (exp <= 0) return;
    experience += exp;
    while (experienceToNextLevel > 0 && experience >= experienceToNextLevel) {
        experience -= experienceToNextLevel;
        levelUp();
        // if (level >= 999) break; // Optional level cap
    }
     if (experience < 0) experience = 0;
}

// --- levelUp ---
void Player::levelUp() { // <<<< START OF FUNCTION BODY {
    level++;
    experienceToNextLevel = level * 10; // Or your formula

    std::cout << "Player Leveled Up to Level " << level << "! Next level requires " << experienceToNextLevel << " EXP." << std::endl;

    // --- Trigger Spawn Pool Update ---  <<<< MAKE SURE THIS IS INSIDE THE FUNCTION
    if (Game::instance) {
        Game::instance->updateSpawnPoolAndWeights(); // This call should be fine here
    } else {
        std::cerr << "Warning: Game::instance is null in Player::levelUp. Cannot update spawn pool." << std::endl;
    }
    // --- End Trigger ---

    // Remove or comment out automatic health increase if desired
    // if (playerEntity && playerEntity->hasComponent<HealthComponent>()) { ... }

    // Buff selection logic
    if (level % 2 == 0) {
        if (Game::instance) {
            //  std::cout << "  >> Triggering Buff Selection Screen (Level " << level << ")" << std::endl;
             Game::instance->enterBuffSelection(); // Trigger the choice screen
        } else {
            //  std::cerr << "Player::levelUp - Game::instance is null, cannot enter buff selection!" << std::endl;
        }
    } else {
        // std::cout << "  >> Skipping Buff Selection Screen (Level " << level << ")" << std::endl;
    }

}


// --- Getters ---
// (Implementations for getHealth, getMaxHealth, etc. using playerEntity->getComponent...)
int Player::getHealth() const { if (playerEntity && playerEntity->hasComponent<HealthComponent>()) return playerEntity->getComponent<HealthComponent>().getHealth(); return 0; }
int Player::getMaxHealth() const { if (playerEntity && playerEntity->hasComponent<HealthComponent>()) return playerEntity->getComponent<HealthComponent>().getMaxHealth(); return 0; }
int Player::getDamage() const { if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) return playerEntity->getComponent<WeaponComponent>().getDamage(); return 0; }
float Player::getFireRate() const { if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) { int fr = playerEntity->getComponent<WeaponComponent>().getFireRate(); return (fr > 0) ? 1000.0f / fr : 0.0f; } return 0.0f; }
int Player::getProjectileCount() const { if (playerEntity && playerEntity->hasComponent<WeaponComponent>()) return playerEntity->getComponent<WeaponComponent>().getProjectileCount(); return 0; }
float Player::getSpeed() const { if (playerEntity && playerEntity->hasComponent<TransformComponent>()) return playerSpeed; /* Use playerSpeed constant from constants.h */ return 0.0f; } // Assuming playerSpeed is defined elsewhere
Vector2D Player::getPosition() const { if (playerEntity && playerEntity->hasComponent<TransformComponent>()) return playerEntity->getComponent<TransformComponent>().position; return Vector2D(); }

// --- heal method Definition ---
void Player::heal(int amount) {
    if (amount <= 0) return; // Ignore non-positive healing
    // Check if the entity and component exist before using them
    if (playerEntity && playerEntity->hasComponent<HealthComponent>()) {
        playerEntity->getComponent<HealthComponent>().heal(amount); // Call the HealthComponent's heal method
    } else {
        std::cerr << "Warning: Player::heal called but playerEntity or HealthComponent is missing!" << std::endl;
    }
}


// --- Setters ---
// (Implementations for setLevel, setExperience, etc.)
void Player::setLevel(int newLevel) { level = std::max(1, newLevel); experienceToNextLevel = level * 10; }
void Player::setExperience(int newExp) { experience = std::max(0, newExp); }
void Player::setExperienceToNextLevel(int newExpToNext) { experienceToNextLevel = std::max(1, newExpToNext); }
void Player::setEnemiesDefeated(int count) { enemiesDefeated = std::max(0, count); }