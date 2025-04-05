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
<<<<<<< HEAD

=======
        // Initialize the game using constants
>>>>>>> 48aebd591664aaebcc837f2de6b6a7394e56c0f2
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
<<<<<<< HEAD

=======
        //Just for it to not show in the console
>>>>>>> 48aebd591664aaebcc837f2de6b6a7394e56c0f2
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
<<<<<<< HEAD

=======
    
>>>>>>> 48aebd591664aaebcc837f2de6b6a7394e56c0f2
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