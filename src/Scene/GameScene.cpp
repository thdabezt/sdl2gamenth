#include "GameScene.h"
#include "SceneManager.h"
#include <iostream>
#include "../constants.h"

GameScene::GameScene() : game(nullptr) {
}

GameScene::~GameScene() {
    clean();
}

void GameScene::resetGame() {
    // std::cout << "Resetting game state for new game" << std::endl;

    // Clean up existing game if it exists by deleting it.
    // The Game destructor (~Game) will now call the modified Game::clean().
    if (game) {
        // game->cleanExceptRenderer(); // REMOVED - Let destructor handle cleanup
        delete game; // This will trigger ~Game() which calls clean()
        game = nullptr;
    }
    // New game will be created in init()
}

void GameScene::init() {
    // std::cout << "Initializing Game Scene" << std::endl;
    
    // Check if game already exists (returning from Win/Lose scene)
    if (game) {
        // std::cout << "Game already exists - just resetting state" << std::endl;
        // Just reset the game state without destroying everything
        Game::isRunning = true;
        return;
    }
    
    // Create a new game only if coming from menu
    game = new Game();
    game->init(WINDOW_TITLE, WINDOW_POS_X, WINDOW_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT, false);
}


void GameScene::handleEvents(SDL_Event& event) {
    // Store the event in the static Game event - THIS MIGHT BE PROBLEMATIC if called twice
    // Game::event = event; // Let's see if commenting this out helps later, but keep for now

    // ADD THIS: Log entry into GameScene::handleEvents for keydown
    if (event.type == SDL_KEYDOWN) {
        // std::cout << "+++ GameScene::handleEvents received keydown: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
    }

    // Let the game handle the event
    if (game) {
        // game->handleEvents(); // Original call - let's analyze the if/else below first
    }

    // Check the existing logic carefully
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_o) {

            SceneManager::instance->switchToScene(SceneType::Win);
        } else if (event.key.keysym.sym == SDLK_p) {

            SceneManager::instance->switchToScene(SceneType::Lose);
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {

            SceneManager::instance->switchToScene(SceneType::Menu);
        } else {

            if(game) game->handleEvents(); 
        }
    } else {
            if(game) game->handleEvents(); // Call game handler for other event types (like SDL_QUIT, SDL_MOUSEMOTION)
    }
}

void GameScene::update() {
    if (game && Game::isRunning) {
        game->update();
    }
}

void GameScene::render() {
    if (game && Game::isRunning) {
        game->render();
    }
}

void GameScene::clean() {
    // std::cout << "Cleaning Game Scene" << std::endl;
    
    // Don't destroy the game object - just set isRunning to false
    if (game) {
        Game::isRunning = false;
        // DON'T delete the game or call cleanExceptRenderer
    }
    // The game object will be properly cleaned up only when the application exits
}