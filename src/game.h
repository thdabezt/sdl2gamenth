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
        Manager manager;                // <<< KEEP this declaration (Ensure only one exists)
        AssetManager* assets = nullptr; // <<< KEEP this declaration (Ensure only one exists)
        Player* playerManager = nullptr;
        Entity* playerEntity = nullptr;
        SaveLoadManager* saveLoadManager = nullptr;
        std::vector<Vector2D> spawnPoints;
        std::string currentPlayerName = "Player"; // Default name
        void setPlayerName(const std::string& name) {
            // Set name, ensuring it's not empty (use default if it is)
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
        static int getMusicVolume(); // Removed const as static members are accessed
        static int getSfxVolume();   // Removed const


        enum groupLabels : std::size_t {
            groupMap, groupPlayers, groupColliders, groupProjectiles, groupEnemies, groupExpOrbs
        };
        
        // --- ADD Game Over Resources ---
        SDL_Texture* gameOverTex = nullptr;
        SDL_Texture* gameOverTextTex = nullptr;
        SDL_Rect gameOverRect;
        SDL_Rect gameOverTextRect;
        TTF_Font* gameOverFont = nullptr; // Or reuse pauseFont
        // --- END Game Over Resources ---

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

        // --- Pause Menu UI State & Resources ---
    TTF_Font* pauseFont = nullptr;
    SDL_Texture* pauseBoxTex = nullptr;
    SDL_Texture* buttonBoxTex = nullptr; // Reused texture for buttons
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

    // Volume/Mute state specifically for the pause menu interaction
    // These will reflect the actual game volumes when the menu opens
    // bool isMusicMutedPause = false;
    // bool isSfxMutedPause = false;
    int storedMusicVolumePause = MIX_MAX_VOLUME / 2; // Stores volume before mute
    int storedSfxVolumePause = MIX_MAX_VOLUME / 2;   // Stores volume before mute
    bool isDraggingBgmPause = false;
    bool isDraggingSfxPause = false;
    int sliderDragXPause = 0; // Horizontal dragging offset

    // --- End Pause Menu UI ---
    std::vector<SDL_Rect> getBuffButtonRects();
    // Helper function for pause layout (optional, can do in render/handleEvents)
    void calculatePauseLayout();
    SDL_Texture* renderPauseText(const std::string& text, SDL_Color color); // Helper
    void handlePauseMenuEvents(); // <<< ADD THIS DECLARATION
};

