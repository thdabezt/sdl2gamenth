#pragma once
#include <string>
#include <SDL.h>
#include "Components.h" // Includes Entity, Vector2D etc. indirectly
#include <stdexcept> // For runtime_error
#include <algorithm> // For std::max

// Forward declare Entity if Components.h doesn't guarantee it before Player use
// class Entity;

class Player {
private:
    Entity* playerEntity; // Pointer to the entity this manager wraps

    // Player stats
    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 10;
    int enemiesDefeated = 0;
    float lifestealPercentage = 0.0f; // Percentage (e.g., 5.0 = 5%)

public:
    // --- Constructor DECLARATION ONLY ---
    Player(Entity* entity); // NO BODY {} here

    // Level system methods
    void addExperience(int exp);
    void levelUp();
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNextLevel() const { return experienceToNextLevel; }
    float getExperiencePercentage() const { return (experienceToNextLevel > 0) ? static_cast<float>(experience) / experienceToNextLevel : 0.0f; }

    // Stat getters
    int getEnemiesDefeated() const { return enemiesDefeated; }
    void incrementEnemiesDefeated() { enemiesDefeated++; }

    // Entity access methods
    Entity& getEntity() { if (!playerEntity) throw std::runtime_error("Player entity is null!"); return *playerEntity; }
    const Entity& getEntity() const { if (!playerEntity) throw std::runtime_error("Player entity is null!"); return *playerEntity; }

    // Health component shortcuts
    int getHealth() const;
    int getMaxHealth() const;
    void heal(int amount); // Heal method DECLARATION

    // Weapon component shortcuts
    int getDamage() const;
    float getFireRate() const;
    int getProjectileCount() const;

    // TransformComponent shortcuts
    float getSpeed() const;
    Vector2D getPosition() const;

    // Setters needed for loading
    void setLevel(int newLevel);
    void setExperience(int newExp);
    void setExperienceToNextLevel(int newExpToNext);
    void setEnemiesDefeated(int count);

    // LIFESTEAL GETTER/SETTER
    float getLifestealPercentage() const { return lifestealPercentage; }
    void setLifestealPercentage(float percentage) { lifestealPercentage = std::max(0.0f, percentage); }
};