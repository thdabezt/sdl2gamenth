#pragma once

#include <SDL_stdinc.h>  

#include <cmath>  
#include <string>
#include <vector>  

#include "../Vector2D.h"
#include "ECS.h"  

class TransformComponent;
class SoundComponent;

enum class SpellTrajectory {
    RANDOM_DIRECTION,
    SPIRAL

};

class SpellComponent : public Component {
   private:

    bool initialized = false;      
    int level = 0;                 
    int burstShotsRemaining = 0;   
    Uint32 nextBurstShotTime = 0;  

    TransformComponent* transform = nullptr;
    SoundComponent* sound = nullptr;

    void createProjectile(Vector2D position, Vector2D velocity);

    void castSingleProjectile();

    void castSpellFull();  

   public:

    std::string tag;  
    int damage;       
    int cooldown;     
    float projectileSpeed;
    int projectilesPerCast;  

    int projectileSize;
    std::string projectileTexture;  
    int duration;  
    SpellTrajectory trajectoryMode;  
    float spiralGrowthRate;  

    int projectilePierce = 1;  
    int burstDelay = 75;       

    Uint32 lastCastTime = 0;   
    float spiralAngle = 0.0f;  

    SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, SpellTrajectory mode,
                   float growthRate = 5.0f,
                   int pierce = 1);  

    void init() override;
    void update() override;

    void castSpell();  

    void increaseDamage(int amount);
    void decreaseCooldown(int amount);  
    void decreaseCooldownPercentage(
        float percent);  
    void increaseProjectileSpeed(float amount);
    void increaseProjectileCount(
        int amount);  
    void increaseProjectileSize(int amount);
    void increasePierce(int amount);

    std::string getTag() const;
    int getDamage() const;
    int getCooldown() const;
    float getProjectileSpeed() const;
    int getProjectileSize() const;
    int getPierce() const;
    int getProjectileCount() const;
    int getDuration() const;  

    int getLevel() const;
    void setLevel(int newLevel);  
    void incrementLevel();        

};  