#include "GameScene.h"
#include "../constants.h" 
#include <iostream>  

GameScene::GameScene() : game(nullptr) {

}

GameScene::~GameScene() {

    clean();
}

void GameScene::init() {

    if (game) {

        Game::isRunning = true; 
        return;
    }

    game = new Game();
    if (game) {

        game->init();
    } else {
        std::cerr << "Error: Failed to allocate memory for Game instance in GameScene::init!" << std::endl;

    }
}

void GameScene::handleEvents(SDL_Event& event) {

    if (game) {
        game->handleEvents();
    }
    if(event.type == SDL_WINDOWEVENT) {

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

    if (game) {
        delete game; 
        game = nullptr;
        Game::isRunning = false; 
    }
}

void GameScene::resetGame() {

    if (game) {
        delete game; 
        game = nullptr;
        Game::isRunning = false;
    }

}