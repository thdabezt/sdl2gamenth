#pragma once

#include <SDL_stdinc.h>  

#include <algorithm>  
#include <stdexcept>  
#include <string>

#include "Components.h"  

class HealthComponent;     
class WeaponComponent;     
class TransformComponent;  

class Player {
   private:

    Entity* playerEntity;  

    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 10;
    int enemiesDefeated = 0;
    float lifestealPercentage = 0.0f;  

   public:

    Player(Entity* entity);  

    void addExperience(int exp);  

    void levelUp();  
    int getLevel() const;
    int getExperience() const;
    int getExperienceToNextLevel() const;
    float getExperiencePercentage() const;

    int getEnemiesDefeated() const;
    void incrementEnemiesDefeated();
    float getLifestealPercentage() const;
    void setLifestealPercentage(float percentage);

    Entity& getEntity();
    const Entity& getEntity() const;

    int getHealth() const;
    int getMaxHealth() const;
    int getDamage() const;
    float getFireRate() const;
    int getProjectileCount() const;
    float getSpeed() const;
    Vector2D getPosition() const;

    void heal(int amount);

    void setLevel(int newLevel);
    void setExperience(int newExp);
    void setExperienceToNextLevel(int newExpToNext);
    void setEnemiesDefeated(int count);

};  