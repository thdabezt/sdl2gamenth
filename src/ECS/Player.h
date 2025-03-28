#pragma once
#include <string>
#include <SDL.h>
#include "Components.h"

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
    float getExperiencePercentage() const { return static_cast<float>(experience) / experienceToNextLevel; }
    
    // Stat getters
    int getEnemiesDefeated() const { return enemiesDefeated; }
    void incrementEnemiesDefeated() { 
        enemiesDefeated++; 
    }
    
    // Weapon upgrade methods
    void upgradeWeapon();
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
};