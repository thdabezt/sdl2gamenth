#pragma once

// #define _USE_MATH_DEFINES // Not needed if using acos
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL.h>

#include "ECS.h"
#include "../Vector2D.h"
#include <iostream> // For logging

// --- Forward Declarations ---
class TransformComponent;
class SoundComponent;
// -----------------------------

enum class SpellTrajectory {
    RANDOM_DIRECTION,
    SPIRAL
};

class SpellComponent : public Component {
private:
    bool initialized = false; // <<< ADD Initialization Flag

public:
    // --- Properties ---
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

    // --- State / Pointers (Initialize) ---
    TransformComponent* transform = nullptr;
    SoundComponent* sound = nullptr;
    Uint32 lastCastTime = 0;
    float spiralAngle = 0.0f;


    SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, int dur, SpellTrajectory mode,
                   float growthRate = 5.0f, int pierce = 1);

    // --- Update Method Declarations ---
    void increaseDamage(int amount) { damage += amount; }
    void decreaseCooldown(int amount) { cooldown = std::max(50, cooldown - amount); }
    void increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
    void increaseProjectileCount(int amount) { projectilesPerCast += amount; }
    void increaseProjectileSize(int amount) { projectileSize += amount; }
    void increasePierce(int amount) { projectilePierce += amount; }

    // Get properties
    std::string getTag() const { return tag; }
    int getDamage() const { return damage; }
    int getCooldown() const { return cooldown; }
    float getProjectileSpeed() const { return projectileSpeed; }
    int getProjectileSize() const { return projectileSize; }
    int getPierce() const { return projectilePierce; }

    // Init/Update/Cast declarations, definitions in .cpp
    void init() override;
    void update() override;
    void castSpell();

private:
    void createProjectile(Vector2D position, Vector2D velocity);
};