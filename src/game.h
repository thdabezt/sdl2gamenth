#pragma once

#include <SDL.h>
#include <iostream>
#include <vector>
#include "Vector2D.h"
#include "ECS/ECS.h"
#include "UI.h"


class AssetManager;
class Entity;
class Map;
class ColliderComponent;

class Game {
public:
    static Game* instance;
    Game();

    ~Game();
    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);

    void handleEvents();
    void update();
    void render();
    void clean();

    bool running() {
        return isRunning;
    }
    void rezero();
    void setRunning(bool running) { isRunning = running; }
    
    void spawnEnemy();
    static SDL_Renderer *renderer;
    static SDL_Event event;
    Entity& getPlayer();
    Player* getPlayerManager() { return playerManager; }  // Add new getter
    static bool isRunning;
    // Add these methods
    static void cleanupRenderer();
    static SDL_Rect camera;
    static AssetManager* assets;
    static int mouseX;
    static int mouseY;
    bool getPaused() const { return isPaused; }
    void togglePause() { isPaused = !isPaused;
        std::cout << "togglePause called, new state: " << isPaused << std::endl; }
    enum groupLabels : std::size_t{
        groupMap,
        groupPlayers,
        groupColliders,
        groupProjectiles,
        groupEnemies,
    };
    void renderHealthBar(Entity& entity, Vector2D position);
    
private:
    int count = 0;
    Uint32 lastShotTime = 0;       // Timer for projectiles
    Uint32 lastEnemySpawnTime = 0; // Timer for enemy spawning
    // SDL_Window *window;
    UIManager* ui = nullptr;
    Player* playerManager = nullptr;  // Add Player instance
    bool isPaused = false;
    Entity* playerEntity = nullptr;
};