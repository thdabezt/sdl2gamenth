#pragma once

#define _USE_MATH_DEFINES // Keep for M_PI/cmath if needed, place before include
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL.h>

#include "ECS.h"
#include "../Vector2D.h"

// --- Forward Declarations ---
class TransformComponent;
class ColliderComponent;
// -----------------------------

enum class SpellTrajectory {
    RANDOM_DIRECTION,
    SPIRAL
};

class SpellComponent : public Component {
public:
    // --- Properties ---
    std::string tag;
    int damage;
    int cooldown;
    float projectileSpeed;
    int projectilesPerCast;
    int projectileSize;
    std::string projectileTexture;
    int duration;         // Used instead of range for time-based projectiles
    SpellTrajectory trajectoryMode;
    float spiralGrowthRate;
    int projectilePierce = 1; // Pierce value for this spell's projectiles

    // --- State / Pointers (Declare ONLY ONCE) ---
    TransformComponent* transform = nullptr;
    Uint32 lastCastTime = 0;
    float spiralAngle = 0.0f;
    // *** Removed duplicate declarations from here ***

    // --- Method Declarations ---
    SpellComponent(std::string spellTag, int dmg, int cool, float speed,
                   int count, int size, std::string texId, int dur, SpellTrajectory mode,
                   float growthRate = 5.0f, int pierce = 1); // Ensure pierce is in constructor

    // --- Update Method Declarations ---
    void increaseDamage(int amount) { damage += amount; }
    void decreaseCooldown(int amount) { cooldown = std::max(50, cooldown - amount); } // Faster fire rate
    void increaseProjectileSpeed(float amount) { projectileSpeed += amount; }
    void increaseProjectileCount(int amount) { projectilesPerCast += amount; }
    void increaseProjectileSize(int amount) { projectileSize += amount; }
    void increasePierce(int amount) { projectilePierce += amount; }

    // Get properties (for UI display)
    std::string getTag() const { return tag; }
    int getDamage() const { return damage; }
    int getCooldown() const { return cooldown; }
    float getProjectileSpeed() const { return projectileSpeed; }
    int getProjectileSize() const { return projectileSize; }
    int getPierce() const { return projectilePierce; }

    


    void init() override;
    void update() override;
    void castSpell();

private:
    void createProjectile(Vector2D position, Vector2D velocity);
};