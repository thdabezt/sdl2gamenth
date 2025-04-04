// Suggested changes for Game/src/game.h
#pragma once
#include <SDL.h>
#include <iostream>
#include <vector>
#include "Vector2D.h"
#include "ECS/ECS.h"
#include "UI.h"
#include <SDL_mixer.h>
#include "SaveLoadManager.h"

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
// --- Definitions needed in header ---
enum class BuffType {
    // Fire Spell ("spell" tag) Buffs
    FIRE_SPELL_DAMAGE,
    FIRE_SPELL_COOLDOWN,
    // FIRE_SPELL_DURATION, // Removed as requested? Confirm if duration upgrades are needed
    // FIRE_SPELL_PIERCE, // Removed as requested (set pierce high initially)
    FIRE_SPELL_PROJ_PLUS_1,

    // Star Spell ("star" tag) Buffs
    STAR_SPELL_DAMAGE,
    STAR_SPELL_COOLDOWN,
    STAR_SPELL_PROJ_PLUS_1, // Renamed from PROJ_COUNT for consistency
    // STAR_SPELL_PIERCE, // Removed as requested

    // Weapon Buffs
    WEAPON_DAMAGE_FLAT,       // New: Flat 10% (calculated)
    WEAPON_DAMAGE_RAND_PERC,  // New: Random 1-20% (calculated)
    WEAPON_FIRE_RATE,         // Existing (logic changed to percentage)
    WEAPON_PROJ_PLUS_1,      // Simple +1 proj (distinct from level 10) - Keep restriction?
    WEAPON_PIERCE,           // Existing (+1)
    WEAPON_BURST_COUNT,      // Existing
    WEAPON_PROJ_PLUS_1_DMG_MINUS_30, // Specific Level 10 effect

    // Player Buffs
    PLAYER_HEAL_FLAT,        // New: 100 HP
    PLAYER_HEAL_PERC_MAX,    // New: 30% Max HP
    PLAYER_HEAL_PERC_LOST,   // New: 60% Lost HP
    PLAYER_MAX_HEALTH_FLAT,  // New: 50 HP
    PLAYER_MAX_HEALTH_PERC_MAX, // New: 25% Max HP
    PLAYER_MAX_HEALTH_PERC_CUR, // New: 50% Current Health Added to Max
    PLAYER_LIFESTEAL,        // Existing (+1%)

    // Force Get Spell Buffs (if needed to guarantee offering them)
    // WEAPON_FORCE_PROJ_PLUS_1,
    // FIRE_SPELL_FORCE_PROJ_PLUS_1,
    // STAR_SPELL_FORCE_PROJ_PLUS_1,

    INVALID
};

struct EnemySpawnInfo {
    std::string tag;
    const char* sprite;
    int baseHealth;
    int baseDamage;
    float speed;
    int baseExperience;
    int currentSpawnWeight = 0;
    int minLevel = 1;
    // Add upgrade info if needed (example, adjust based on your actual upgrade structure)
    std::string upgradeTag = "";
    int upgradeLevelRequirement = 0; // Level at which this *becomes* available if it's an upgrade
};

// Buff Info Struct
struct BuffInfo {
    std::string name;
    std::string description;
    BuffType type = BuffType::INVALID;
    float amount = 0.0f;
};
enum class GameState {
    Playing,
    Paused,
    GameOver
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
        Manager manager;
        AssetManager* assets = nullptr;
        Player* playerManager = nullptr;
        Entity* playerEntity = nullptr;
        SaveLoadManager* saveLoadManager = nullptr;
        std::vector<Vector2D> spawnPoints;
        std::string currentPlayerName = "Player"; // Default name
        void setPlayerName(const std::string& name) {
            currentPlayerName = name.empty() ? "Player" : name;
            std::cout << "Player name set to: " << currentPlayerName << std::endl; // Debug log
        }
        std::string getPlayerName() const {
            return currentPlayerName;
        }

        // --- ADD Game State ---
        GameState currentState = GameState::Playing;
        // --- END Game State ---

        Game();
        ~Game();
        void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
        void handleEvents();
        void update();
        void render();
        void clean();
        bool running() { return isRunning; }
        void setRunning(bool running) { isRunning = running; }

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

        // --- Make volume members static ---
        static int musicVolume; // Default value set in .cpp
        static int sfxVolume;   // Default value set in .cpp

        // --- Make accessors/modifiers static ---
        static void setMusicVolume(int volume);
        static void setSfxVolume(int volume);
        static int getMusicVolume();
        static int getSfxVolume();

        enum groupLabels : std::size_t {
            groupMap, groupPlayers, groupColliders, groupProjectiles, groupEnemies, groupExpOrbs
        };

        // --- ADD Game Over Resources ---
        SDL_Texture* gameOverTex = nullptr;
        SDL_Texture* gameOverTextTex = nullptr;
        SDL_Rect gameOverRect;
        SDL_Rect gameOverTextRect;
        TTF_Font* gameOverFont = nullptr;
        // --- END Game Over Resources ---

        void initializeEnemyDatabase();
        void updateSpawnPoolAndWeights();
        EnemySpawnInfo* selectEnemyBasedOnWeight();
        void spawnBossNearPlayer(); // <<< ADD THIS DECLARATION
        void spawnBoss(); // <<< RENAMED Declaration
    private:
        // Private methods
        void changeMusicVolume(int delta);
        void changeSfxVolume(int delta);
        void generateBuffOptions();

        // Private members
        UIManager* ui = nullptr;
        Map* map = nullptr;
        Uint32 lastEnemySpawnTime = 0;
        Uint32 lastShotTime = 0;
        bool isInBuffSelection = false;
        std::vector<BuffInfo> currentBuffOptions;

        // --- Pause Menu UI State & Resources ---
        TTF_Font* pauseFont = nullptr;
        SDL_Texture* pauseBoxTex = nullptr;
        SDL_Texture* buttonBoxTex = nullptr;
        SDL_Texture* soundOnTex = nullptr;
        SDL_Texture* soundOffTex = nullptr;
        SDL_Texture* sliderTrackTex = nullptr;
        SDL_Texture* sliderButtonTex = nullptr;

        // Button Text Textures
        SDL_Texture* continueTextTex = nullptr;
        SDL_Texture* saveTextTex = nullptr;
        SDL_Texture* returnTextTex = nullptr;

        // Rects for layout
        SDL_Rect pauseBoxRect;
        SDL_Rect continueButtonRect;
        SDL_Rect continueTextRect;
        SDL_Rect bgmIconRectPause;
        SDL_Rect bgmSliderTrackRectPause;
        SDL_Rect bgmSliderButtonRectPause;
        SDL_Rect sfxIconRectPause;
        SDL_Rect sfxSliderTrackRectPause;
        SDL_Rect sfxSliderButtonRectPause;
        SDL_Rect saveButtonRect;
        SDL_Rect saveTextRect;
        SDL_Rect returnButtonRect;
        SDL_Rect returnTextRect;

        int storedMusicVolumePause = MIX_MAX_VOLUME / 2;
        int storedSfxVolumePause = MIX_MAX_VOLUME / 2;
        bool isDraggingBgmPause = false;
        bool isDraggingSfxPause = false;
        int sliderDragXPause = 0;

        // --- End Pause Menu UI ---
        std::vector<SDL_Rect> getBuffButtonRects();
        void calculatePauseLayout();
        SDL_Texture* renderPauseText(const std::string& text, SDL_Color color);
        void handlePauseMenuEvents();


         // --- Add Enemy Spawning Members ---
    std::vector<EnemySpawnInfo> allEnemyDatabase; // Stores data for ALL enemies
    std::vector<EnemySpawnInfo*> currentSpawnPool; // Pointers to currently active enemies
    int currentTotalSpawnWeight = 0; // Store the calculated total weight

    
    // --- End Enemy Spawning ---
};