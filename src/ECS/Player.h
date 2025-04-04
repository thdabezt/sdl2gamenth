#pragma once

// --- Includes ---
#include <string>
#include <SDL_stdinc.h> // For Uint32 if needed, though not directly used here
#include <stdexcept>    // For std::runtime_error
#include <algorithm>    // For std::max
#include "Components.h" // Includes Entity.h, Vector2D.h, and component headers indirectly

// --- Forward Declarations ---
class HealthComponent;     // Needed for getter return types/checks
class WeaponComponent;     // Needed for getter return types/checks
class TransformComponent;  // Needed for getter return types/checks

// --- Class Definition ---

// Manages player-specific stats and provides convenience methods
// to access data stored in the associated player Entity's components.
class Player {
private:
    // --- Private Members ---
    Entity* playerEntity; // Pointer to the entity this manager wraps

    // Player stats
    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 10;
    int enemiesDefeated = 0;
    float lifestealPercentage = 0.0f; // Percentage (e.g., 5.0 = 5%)

public:
    // --- Constructor ---
    // Constructor requires a valid pointer to the player's Entity.
    Player(Entity* entity); // Definition in Player.cpp

    // --- Public Methods ---

    // Level System
    void addExperience(int exp); // Adds experience and handles level ups. Definition in Player.cpp
    void levelUp();              // Handles leveling up logic. Definition in Player.cpp
    int getLevel() const;       
    int getExperience() const;  
    int getExperienceToNextLevel() const;
    float getExperiencePercentage() const;

    // Stats
    int getEnemiesDefeated() const;
    void incrementEnemiesDefeated();
    float getLifestealPercentage() const;
    void setLifestealPercentage(float percentage);

    // Entity Access
    Entity& getEntity();      
    const Entity& getEntity() const;

    // Component Shortcuts (Getters)
    int getHealth() const;       
    int getMaxHealth() const;    
    int getDamage() const;       
    float getFireRate() const;   
    int getProjectileCount() const;
    float getSpeed() const;      
    Vector2D getPosition() const;

    // Actions
    void heal(int amount);

    // Setters (Mainly for loading game state)
    void setLevel(int newLevel);            
    void setExperience(int newExp);         
    void setExperienceToNextLevel(int newExpToNext);
    void setEnemiesDefeated(int count);     

}; // End Player class