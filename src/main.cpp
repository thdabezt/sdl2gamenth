// #include <SDL.h>
// #include <iostream>
// #include <SDL_image.h>
// #include "game.h"
// #define SDL_MAIN_HANDLED
// #include "constants.h"
// #include <Windows.h>

// Game *game = nullptr;

// int main(int argc, char* argv[]) {
//     //console
//     if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
//         freopen("CONOUT$", "w", stdout);
//         freopen("CONOUT$", "w", stderr);
//         freopen("CONIN$", "r", stdin);
//     }
//     //console end

//     const int frameDelay = 1000 / FPS;
//     Uint32 frameStart;
//     int frameTime;
//     game = new Game();

//     game->init(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FULLSCREEN);
    
//     std::cout << "Console attached successfully!" << std::endl;
//     if (!game->running()) {
//         std::cerr << "Failed to initialize the game." << std::endl;
//         return -1;
//     }
    

//     while(game->running()){
//         frameStart = SDL_GetTicks();
//         game->handleEvents();
//         game->update();
//         game->render();

//         frameTime = SDL_GetTicks() - frameStart;

//         if(frameDelay > frameTime){
//             SDL_Delay(frameDelay - frameTime);
//         }
//     }

//     game->clean();
    
//     return 0;

// }

#include <SDL.h>
#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "constants.h"
#include "game.h"
#include "Scene/SceneComponent.h"
#include <Windows.h>
#define SDL_MAIN_HANDLED

int main(int argc, char* argv[]) {
    // Setup console
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
    }
    std::cout << "Console attached successfully!" << std::endl;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }
    
    // Create window and renderer directly
    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE, 
        WINDOW_POS_X, 
        WINDOW_POS_Y, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        WINDOW_FULLSCREEN
    );
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        return -1;
    }
    
    // Set the global renderer in Game class
    Game::renderer = renderer;
    
    // Create scene manager
    SceneManager sceneManager;
    
    // Add scenes
    sceneManager.addScene(SceneType::Menu, std::make_unique<MenuScene>());
    sceneManager.addScene(SceneType::Game, std::make_unique<GameScene>());
    sceneManager.addScene(SceneType::Win, std::make_unique<WinScene>());
    sceneManager.addScene(SceneType::Lose, std::make_unique<LoseScene>());
    
    // Start with the menu scene
    sceneManager.switchToScene(SceneType::Menu);
    
    // Game loop variables
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;
    bool running = true;
    SDL_Event event;
    
    // Main game loop

while (running) {
    frameStart = SDL_GetTicks();
    
    // Handle events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        // Copy the event to the static Game event
        Game::event = event;
        
        // Process events in the scene manager
        sceneManager.handleEvents(event);
    }
    
    // Update and render through the scene manager
    sceneManager.update();
    sceneManager.render();
    
    // Cap the frame rate
    frameTime = SDL_GetTicks() - frameStart;
    if (frameDelay > frameTime) {
        SDL_Delay(frameDelay - frameTime);
    }
}

// Clean up resources in the correct order
std::cout << "Cleaning up all resources..." << std::endl;

// First clean up scenes which might still reference the renderer
sceneManager.clean();

// Clean up the shared renderer and window last
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
Game::renderer = nullptr;

TTF_Quit();
SDL_Quit();

return 0;
}