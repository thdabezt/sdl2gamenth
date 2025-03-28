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
    std::cout << "Resetting game state for new game" << std::endl;
    
    // Clean up existing game if it exists
    if (game) {
        game->cleanExceptRenderer();
        delete game;
        game = nullptr;
    }
    
    // New game will be created in init()
}

void GameScene::init() {
    std::cout << "Initializing Game Scene" << std::endl;
    
    // Check if game already exists (returning from Win/Lose scene)
    if (game) {
        std::cout << "Game already exists - just resetting state" << std::endl;
        // Just reset the game state without destroying everything
        Game::isRunning = true;
        return;
    }
    
    // Create a new game only if coming from menu
    game = new Game();
    game->init(WINDOW_TITLE, WINDOW_POS_X, WINDOW_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT, false);
}

void GameScene::handleEvents(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_o) {
            std::cout << "Switching to Win Scene" << std::endl;
            SceneManager::instance->switchToScene(SceneType::Win);
        } else if (event.key.keysym.sym == SDLK_p) {
            std::cout << "Switching to Lose Scene" << std::endl;
            SceneManager::instance->switchToScene(SceneType::Lose);
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            std::cout << "Returning to Menu" << std::endl;
            SceneManager::instance->switchToScene(SceneType::Menu);
        } else {
            game->handleEvents();
        }
    } else {
        game->handleEvents();
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
    std::cout << "Cleaning Game Scene" << std::endl;
    
    // Don't destroy the game object - just set isRunning to false
    if (game) {
        Game::isRunning = false;
        // DON'T delete the game or call cleanExceptRenderer
    }
    // The game object will be properly cleaned up only when the application exits
}