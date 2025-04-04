#pragma once

// --- Includes ---
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h> // Required for font members
#include <iostream>
#include <vector>
#include <string>
#include <map> // Required for AssetManager members if declared inline
#include "Vector2D.h"
#include "ECS/ECS.h"
#include "UI.h"
#include "SaveLoadManager.h"

// --- Forward Declarations ---
class AssetManager;
class Entity;
class Map;
class ColliderComponent;
class TransformComponent;   // <--- ADDED
class ProjectileComponent; // <--- ADDED
class HealthComponent;     // <--- ADDED
class UIManager;
class Player;
class SpellComponent;
class WeaponComponent;


// --- Enums & Structs ---

enum class GameState {
    Playing,
    Paused,
    GameOver
};

enum class BuffType {
    // Fire Spell ("spell" tag) Buffs
    FIRE_SPELL_DAMAGE,
    FIRE_SPELL_COOLDOWN,
    FIRE_SPELL_PROJ_PLUS_1,

    // Star Spell ("star" tag) Buffs
    STAR_SPELL_DAMAGE,
    STAR_SPELL_COOLDOWN,
    STAR_SPELL_PROJ_PLUS_1,

    // Weapon Buffs
    WEAPON_DAMAGE_FLAT,
    WEAPON_DAMAGE_RAND_PERC,
    WEAPON_FIRE_RATE,
    WEAPON_PROJ_PLUS_1,
    WEAPON_PIERCE,
    WEAPON_BURST_COUNT,
    WEAPON_PROJ_PLUS_1_DMG_MINUS_30,

    // Player Buffs
    PLAYER_HEAL_FLAT,
    PLAYER_HEAL_PERC_MAX,
    PLAYER_HEAL_PERC_LOST,
    PLAYER_MAX_HEALTH_FLAT,
    PLAYER_MAX_HEALTH_PERC_MAX,
    PLAYER_MAX_HEALTH_PERC_CUR,
    PLAYER_LIFESTEAL,

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
    std::string upgradeTag = "";
    int upgradeLevelRequirement = 0;
};

struct BuffInfo {
    std::string name;
    std::string description;
    BuffType type = BuffType::INVALID;
    float amount = 0.0f;
};

// --- Class Definition ---

class Game {
public:
    // --- Static Members ---
    static Game* instance;
    static SDL_Renderer* renderer;
    static SDL_Event event;
    static SDL_Rect camera;
    static bool isRunning;
    static int mouseX;
    static int mouseY;
    static int musicVolume; // Default value set in .cpp
    static int sfxVolume;   // Default value set in .cpp

    // --- ECS Group Labels ---
    enum groupLabels : std::size_t {
        groupMap, groupPlayers, groupColliders, groupProjectiles, groupEnemies, groupExpOrbs
    };

    // --- Public Members ---
    Manager manager;
    AssetManager* assets = nullptr;
    Player* playerManager = nullptr;
    Entity* playerEntity = nullptr;
    SaveLoadManager* saveLoadManager = nullptr;
    std::vector<Vector2D> spawnPoints;
    std::string currentPlayerName = "Player";
    GameState currentState = GameState::Playing;
    const int VOLUME_STEP = 10; // Constant for volume adjustment steps

    // --- Constructor & Destructor ---
    Game();
    ~Game();

    // --- Public Methods ---
    // Initialization & Cleanup
    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
    void clean();

    // Game Loop & State
    void handleEvents();
    void update();
    void render();
    bool running() { return isRunning; }
    void setRunning(bool running) { isRunning = running; }
    void togglePause() ;

    // Spawning
    void spawnEnemy();
    void initializeEnemyDatabase();
    void updateSpawnPoolAndWeights();
    EnemySpawnInfo* selectEnemyBasedOnWeight();
    void spawnBossNearPlayer(); // Debug spawn
    void spawnBoss();           // Level-triggered spawn

    // Buff System
    void enterBuffSelection();
    void exitBuffSelection();
    void applySelectedBuff(int index);

    // Audio
    static void setMusicVolume(int volume);
    static void setSfxVolume(int volume);
    static int getMusicVolume();
    static int getSfxVolume();

    // Getters
    Entity& getPlayer();
    Player* getPlayerManager() { return playerManager; }
    std::string getPlayerName() const { return currentPlayerName; }

    // Setters
    void setPlayerName(const std::string& name) {
        currentPlayerName = name.empty() ? "Player" : name;
    }

    // Rendering Helpers
    void renderHealthBar(Entity& entity, Vector2D position);

    // Misc

private:
    // --- Private Members ---
    // Managers & Core Components
    UIManager* ui = nullptr;
    Map* map = nullptr;

    // Timers & Flags
    Uint32 lastEnemySpawnTime = 0;
    Uint32 lastShotTime = 0; // Might be specific to weapon/spell, check usage
    bool isInBuffSelection = false;

    // Buff System Data
    std::vector<BuffInfo> currentBuffOptions;

    // Pause Menu UI Resources
    TTF_Font* pauseFont = nullptr;
    SDL_Texture* pauseBoxTex = nullptr;
    SDL_Texture* buttonBoxTex = nullptr;
    SDL_Texture* soundOnTex = nullptr;
    SDL_Texture* soundOffTex = nullptr;
    SDL_Texture* sliderTrackTex = nullptr;
    SDL_Texture* sliderButtonTex = nullptr;
    SDL_Texture* continueTextTex = nullptr;
    SDL_Texture* saveTextTex = nullptr;
    SDL_Texture* returnTextTex = nullptr;

    // Pause Menu State
    SDL_Rect pauseBoxRect;
    SDL_Rect continueButtonRect, continueTextRect;
    SDL_Rect saveButtonRect, saveTextRect;
    SDL_Rect returnButtonRect, returnTextRect;
    SDL_Rect bgmIconRectPause, bgmSliderTrackRectPause, bgmSliderButtonRectPause;
    SDL_Rect sfxIconRectPause, sfxSliderTrackRectPause, sfxSliderButtonRectPause;
    int storedMusicVolumePause = MIX_MAX_VOLUME / 2;
    int storedSfxVolumePause = MIX_MAX_VOLUME / 2;
    bool isDraggingBgmPause = false;
    bool isDraggingSfxPause = false;
    int sliderDragXPause = 0;

    // Game Over Resources
    SDL_Texture* gameOverTex = nullptr;
    SDL_Texture* gameOverTextTex = nullptr;
    SDL_Rect gameOverRect;
    SDL_Rect gameOverTextRect;
    TTF_Font* gameOverFont = nullptr;

    // Enemy Spawning Data
    std::vector<EnemySpawnInfo> allEnemyDatabase;
    std::vector<EnemySpawnInfo*> currentSpawnPool;
    int currentTotalSpawnWeight = 0;

    // --- Private Methods ---
    // Internal Logic
    void handleTerrainCollision(ColliderComponent& playerCollider, TransformComponent& playerTransform, SDL_Rect& playerColRect);
    void handleProjectileCollisions(Uint32 currentTime);
    void handleProjectileHitEnemy(Entity* projectile, Entity* enemy, ProjectileComponent& projComp, Uint32 currentTime);
    void handleEnemyDeath(Entity* enemy, int maxHp);
    void handleBossProjectileHitPlayer(Entity* projectile, Uint32 currentTime);
    void handleEnemySpawning(Uint32 currentTime);
    void updateCamera(TransformComponent& playerTransform);
    void checkPlayerDeath(HealthComponent& playerHealth, Uint32 currentTime);
    void spawnBossAt(Vector2D spawnPos); // Helper for boss spawning

    // Event Handling Helpers
    void handlePauseMenuEvents();
    void handleBuffSelectionEvents();
    void handleSliderDrag(int mouseX_Screen, int mouseY_Screen);

    // Rendering Helpers
    void renderPausedState();
    void renderPauseMenuUI();
    void renderGameOverState();
    SDL_Texture* renderPauseText(const std::string& text, SDL_Color color);

    // Audio Helpers
    void toggleMute(bool isMusic); // Helper for pause menu mute toggle

    // Buff System Helpers
    void generateBuffOptions();
    std::vector<SDL_Rect> getBuffButtonRects();

    // Pause Menu Helpers
    void calculatePauseLayout();

}; // End Game class