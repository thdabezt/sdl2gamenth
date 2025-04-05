// --- Includes ---
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <Windows.h> // For console setup
#include "constants.h"
#include "game.h"
#include "Scene/SceneComponent.h" // Includes SceneManager.h, MenuScene.h, GameScene.h



// --- Globals ---
static SDL_Window* mainWindow = nullptr; // Global pointer for fullscreen toggle

// --- Helper Functions ---

// Initializes or attaches a console window for debugging output.
void setupConsole() {
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        FILE* pCout, * pCerr, * pCin;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCerr, "CONOUT$", "w", stderr);
        freopen_s(&pCin, "CONIN$", "r", stdin);
        std::cout << "Console attached successfully!" << std::endl;
    } else {
        std::cerr << "Failed to setup console." << std::endl;
    }
}

// Toggles the main window between windowed and fullscreen desktop modes.
void toggleFullscreen() {
    if (!mainWindow) {
        std::cerr << "Error: mainWindow pointer is null in toggleFullscreen!" << std::endl;
        return;
    }

    Uint32 currentFlags = SDL_GetWindowFlags(mainWindow);
    bool isFullscreen = (currentFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) || (currentFlags & SDL_WINDOW_FULLSCREEN);

    if (isFullscreen) {
        // Switch to Windowed Mode
        SDL_SetWindowFullscreen(mainWindow, 0);
        SDL_SetWindowSize(mainWindow, WINDOW_WIDTH, WINDOW_HEIGHT); // Use constants
        SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    } else {
        // Switch to Fullscreen Desktop Mode
        SDL_SetWindowFullscreen(mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    // Let SceneManager handle layout recalculation via WINDOWEVENT_RESIZED
}

// --- Main Function ---

int main(int argc, char* argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning

    setupConsole();

    // --- SDL Initialization ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) { // Initialize necessary subsystems
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { // Initialize SDL_image for PNG loading
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) { // Initialize SDL_ttf
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { // Initialize SDL_mixer
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_AllocateChannels(16); // Allocate mixer channels
  
    }

    // --- Window & Renderer ---
    Uint32 windowFlags = SDL_WINDOW_SHOWN | (WINDOW_FULLSCREEN ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    mainWindow = SDL_CreateWindow(
        WINDOW_TITLE,
        WINDOW_POS_X,
        WINDOW_POS_Y,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowFlags
    );

    if (!mainWindow) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(mainWindow);
        Mix_CloseAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit();
        return -1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Enable alpha blending

    // Set the global renderer for the Game class
    Game::renderer = renderer;

    // --- Scene Manager Setup ---
    SceneManager sceneManager;
    sceneManager.addScene(SceneType::Menu, std::make_unique<MenuScene>());
    sceneManager.addScene(SceneType::Game, std::make_unique<GameScene>());
    sceneManager.switchToScene(SceneType::Menu); // Start with the menu

    // --- Game Loop ---
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;
    bool gameIsRunning = true; // Local loop control
    SDL_Event event;

    while (gameIsRunning) {
        frameStart = SDL_GetTicks();

        // --- Event Handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameIsRunning = false;
            }

            // Handle global fullscreen toggle first
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11) {
                toggleFullscreen();
            }

            // Pass event to the Game instance (for components that read it directly)
            if(Game::instance) { // Null check for safety
                Game::event = event;
            }

            // Pass event to the current scene
            sceneManager.handleEvents(event);
        }

        // --- Update ---
        sceneManager.update();

        // --- Render ---
        sceneManager.render(); // Scene manager handles clearing and presenting

        // --- Frame Rate Cap ---
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    // --- Cleanup ---
    std::cout << "Cleaning up application resources..." << std::endl;

    sceneManager.clean(); // Clean up scenes first

    // Clean up SDL subsystems and renderer/window
    if(Game::renderer) { // Use Game::renderer as it holds the pointer
       SDL_DestroyRenderer(Game::renderer);
       Game::renderer = nullptr;
    }
    if(mainWindow) {
       SDL_DestroyWindow(mainWindow);
       mainWindow = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    std::cout << "Application exited cleanly." << std::endl;
    return 0;
}