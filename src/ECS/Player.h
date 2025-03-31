#pragma once
#include <string>
#include <SDL.h>
#include "Components.h" // Make sure this includes necessary component headers

class Player {
private:
    // Reference to the player entity
    Entity* playerEntity;

    // Player stats
    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 10;
    int enemiesDefeated = 0;


public:
    Player(Entity* entity) : playerEntity(entity) {}

    // Level system methods
    void addExperience(int exp);
    void levelUp();
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNextLevel() const { return experienceToNextLevel; }
    float getExperiencePercentage() const { return (experienceToNextLevel > 0) ? static_cast<float>(experience) / experienceToNextLevel : 0.0f; } // Added check for division by zero

    // Stat getters
    int getEnemiesDefeated() const { return enemiesDefeated; }
    void incrementEnemiesDefeated() {
        enemiesDefeated++;
    }

    // Weapon upgrade methods (Example, implement if needed)
    // void upgradeWeapon();
    // int getWeaponUpgrades() const { return weaponUpgrades; }

    // Entity access methods
    Entity& getEntity() { return *playerEntity; }

    // Health component shortcuts
    int getHealth() const;
    int getMaxHealth() const;

    // Weapon component shortcuts
    int getDamage() const;
    float getFireRate() const;
    int getProjectileCount() const;

    // TransformComponent shortcuts
    float getSpeed() const;
    Vector2D getPosition() const;

    // --- Add Setters needed for loading ---
    void setLevel(int newLevel);
    void setExperience(int newExp);
    void setExperienceToNextLevel(int newExpToNext);
    void setEnemiesDefeated(int count);

};