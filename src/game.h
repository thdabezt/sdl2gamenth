#pragma once

#include <SDL.h>
#include <iostream>
#include <vector>
// #include <fstream>   // Removed - Moved to SaveLoadManager
// #include <string>    // Removed - Moved to SaveLoadManager
// #include <chrono>    // Removed - Moved to SaveLoadManager
// #include <iomanip>   // Removed - Moved to SaveLoadManager
#include "Vector2D.h"
#include "ECS/ECS.h"
#include "UI.h"
#include <SDL_mixer.h>
#include "SaveLoadManager.h" // Include the new header

// Forward declarations
class AssetManager;
class Entity;
class Map;
class ColliderComponent;
class UIManager;
class Player;
class SpellComponent;
class WeaponComponent;
// class SaveLoadManager; // No longer needed if included above



// Buff Type Enum
enum class BuffType {
    // --- Fire Spell ("spell" tag) Buffs ---
    FIRE_SPELL_DAMAGE,
    FIRE_SPELL_COOLDOWN,
    // FIRE_SPELL_PROJ_SPEED, // Uncomment or add if needed
    // FIRE_SPELL_PROJ_COUNT, // Note: Spiral spell doesn't use count currently
    FIRE_SPELL_DURATION, // Example: Increase spiral duration
    FIRE_SPELL_PIERCE,

    // --- Star Spell ("star" tag) Buffs ---
    STAR_SPELL_DAMAGE,
    STAR_SPELL_COOLDOWN,
    // STAR_SPELL_PROJ_SPEED, // Uncomment or add if needed
    STAR_SPELL_PROJ_COUNT, // Star spell uses random direction, count might be relevant
    STAR_SPELL_PIERCE,

    // --- Weapon Buffs ---
    WEAPON_DAMAGE,
    WEAPON_FIRE_RATE,
    WEAPON_PROJ_COUNT,
    WEAPON_PIERCE,
    WEAPON_BURST_COUNT,

    // --- Player Buffs ---
    PLAYER_HEAL,

    INVALID // Keep for error handling
};

// Buff Info Struct
struct BuffInfo {
    std::string name;
    std::string description;
    BuffType type = BuffType::INVALID;
    float amount = 0.0f;
};

class Game {
    public:
        // Static members
        static Game* instance;
        static SDL_Renderer* renderer;
        static SDL_Event event;
        static SDL_Rect camera;
        static bool isRunning;
        static int mouseX;
        static int mouseY;
    
        // Public Members (Declare ONCE)
        Manager manager;                // <<< KEEP this declaration (Ensure only one exists)
        AssetManager* assets = nullptr; // <<< KEEP this declaration (Ensure only one exists)
        Player* playerManager = nullptr;
        Entity* playerEntity = nullptr;
        int musicVolume = MIX_MAX_VOLUME / 2;
        int sfxVolume = MIX_MAX_VOLUME / 2;
        SaveLoadManager* saveLoadManager = nullptr;
        std::vector<Vector2D> spawnPoints;
        // --- REMOVE DUPLICATE DECLARATIONS ---
        // Remove lines like these if they exist elsewhere in the class definition:
        // AssetManager* assets = nullptr; // REMOVE if duplicate
        // Manager manager;             // REMOVE if duplicate
        // --- END REMOVAL ---
    
    
        Game();
        ~Game();
        void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
        void handleEvents();
        void update();
        void render();
        void clean();
        bool running() { return isRunning; }
        void setRunning(bool running) { isRunning = running; }
        bool getPaused() const { return isPaused; }
        void togglePause() ;
        void rezero();
        Entity& getPlayer();
        Player* getPlayerManager() { return playerManager; }
        void spawnEnemy();
        void renderHealthBar(Entity& entity, Vector2D position);
        void enterBuffSelection();
        void exitBuffSelection();
        void applySelectedBuff(int index);
        
        const int VOLUME_STEP = 10;
        int getMusicVolume() const { return musicVolume; }
        int getSfxVolume() const { return sfxVolume; }
    
        enum groupLabels : std::size_t {
            groupMap, groupPlayers, groupColliders, groupProjectiles, groupEnemies, groupExpOrbs
        };
    
    private:
        // Private methods
        void changeMusicVolume(int delta);
        void changeSfxVolume(int delta);
        void generateBuffOptions();
    
        // Private members
        UIManager* ui = nullptr;
        Map* map = nullptr;
        bool isPaused = false;
        Uint32 lastEnemySpawnTime = 0;
        Uint32 lastShotTime = 0;
        bool isInBuffSelection = false;
        std::vector<BuffInfo> currentBuffOptions;
    
        // Buff constants (can stay private or move to constants.h)
        // const int BUFF_DAMAGE_AMOUNT = 5;
        const int BUFF_DAMAGE_AMOUNT = 5;
        const int BUFF_COOLDOWN_AMOUNT = 50;
        const float BUFF_PROJ_SPEED_AMOUNT = 0.5f;
        const int BUFF_PROJ_COUNT_AMOUNT = 1;
        const int BUFF_PROJ_SIZE_AMOUNT = 2;
        const int BUFF_PIERCE_AMOUNT = 1;
        const int BUFF_FIRE_RATE_AMOUNT = 50;
        const int BUFF_BURST_COUNT_AMOUNT = 1;
    };
