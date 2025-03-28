#include "MenuScene.h"
#include "SceneManager.h"
#include "GameScene.h"
#include <SDL_ttf.h>
#include <iostream>
#include "../constants.h"

MenuScene::MenuScene() : menuText(nullptr) {
}

MenuScene::~MenuScene() {
    clean();
}

void MenuScene::init() {
    // Load font
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    // Render text
    SDL_Color textColor = {255, 255, 255, 255}; // White color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Press ENTER to Start Game", textColor);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }
    
    menuText = SDL_CreateTextureFromSurface(Game::renderer, textSurface);
    if (!menuText) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
    }
    
    textRect.x = (WINDOW_WIDTH - textSurface->w) / 2;
    textRect.y = (WINDOW_HEIGHT - textSurface->h) / 2;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

void MenuScene::handleEvents(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_RETURN) {
            // Switch to game scene - this will be a fresh game
            std::cout << "Starting new game from menu" << std::endl;
            
            // Get the GameScene and reset it before switching
            GameScene* gameScene = static_cast<GameScene*>(SceneManager::instance->getScene(SceneType::Game));
            if (gameScene) {
                gameScene->resetGame();
            }
            
            SceneManager::instance->switchToScene(SceneType::Game);
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            // Exit the application
            std::cout << "Exit requested from Menu" << std::endl;
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }
    }
}

void MenuScene::update() {
    // No need for updates in menu scene
}

void MenuScene::render() {
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(Game::renderer);
    
    if (menuText) {
        SDL_RenderCopy(Game::renderer, menuText, NULL, &textRect);
    }
    
    SDL_RenderPresent(Game::renderer);
}

void MenuScene::clean() {
    if (menuText) {
        SDL_DestroyTexture(menuText);
        menuText = nullptr;
    }
}