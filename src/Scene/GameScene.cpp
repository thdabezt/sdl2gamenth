// --- Includes ---
#include "GameScene.h"
#include "../constants.h" // Provides window settings if used in init
#include <iostream>  

// --- Constructor & Destructor ---

GameScene::GameScene() : game(nullptr) {
    // Constructor initializes game pointer to null
}

GameScene::~GameScene() {
    // Destructor ensures cleanup is called when the scene is destroyed
    clean();
}

// --- Public Methods (Scene Overrides) ---

void GameScene::init() {
    // If a game instance already exists (e.g., returning from another scene without quitting),
    // just ensure its running state is true.
    if (game) {
        // If game state needs reset beyond just isRunning, do it here.
        Game::isRunning = true; // Use static member
        return;
    }

    // Create and initialize a new Game instance if one doesn't exist.
    game = new Game();
    if (game) {
        // Initialize the game using constants
        game->init();
    } else {
        std::cerr << "Error: Failed to allocate memory for Game instance in GameScene::init!" << std::endl;
        // Handle allocation failure, perhaps by setting an error state or exiting
    }
}

void GameScene::handleEvents(SDL_Event& event) {
    // Pass the event down to the Game instance's handlers
    if (game) {
        game->handleEvents();
    }
    if(event.type == SDL_WINDOWEVENT) {
        //Just for it to not show in the console
    }
}

void GameScene::update() {
    // Update the Game instance if it exists and is running
    if (game && Game::isRunning) { // Use static member
        game->update();
    }
}

void GameScene::render() {
    // Render the Game instance if it exists and is running
    if (game && Game::isRunning) { // Use static member
        game->render();
    }
    
}

void GameScene::clean() {
    // This function is called when switching away from this scene
    // or when the SceneManager cleans up.
    // Deleting the game instance here handles resetting the game state
    // when coming back from the menu or properly cleaning up on exit.
    if (game) {
        delete game; // Calls Game::~Game(), which calls Game::clean()
        game = nullptr;
        Game::isRunning = false; 
    }
}

// --- Helper Methods ---

void GameScene::resetGame() {
    // Explicitly called, usually before init when starting a fresh game session.
    // This ensures any existing game instance is fully cleaned up.
    if (game) {
        delete game; // Calls Game::~Game(), triggering full cleanup
        game = nullptr;
        Game::isRunning = false;
    }
    // A new game instance will be created in the subsequent call to init().
}