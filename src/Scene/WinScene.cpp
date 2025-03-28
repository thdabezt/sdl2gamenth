#include "WinScene.h"
#include "SceneManager.h"
#include <SDL_ttf.h>
#include <iostream>
#include "../constants.h"

WinScene::WinScene() : winText(nullptr) {
}

WinScene::~WinScene() {
    clean();
}

void WinScene::init() {
    // Load font
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 36);
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    // Render text
    SDL_Color textColor = {0, 255, 0, 255}; // Green color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "YOU WIN!", textColor);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }
    
    winText = SDL_CreateTextureFromSurface(Game::renderer, textSurface);
    if (!winText) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
    }
    
    textRect.x = (WINDOW_WIDTH - textSurface->w) / 2;
    textRect.y = (WINDOW_HEIGHT - textSurface->h) / 2;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

void WinScene::handleEvents(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            // Return to menu
            SceneManager::instance->switchToScene(SceneType::Menu);
        } else if (event.key.keysym.sym == SDLK_RETURN) {
            // Return to game without resetting
            SceneManager::instance->switchToScene(SceneType::Game);
        }
    }
}

void WinScene::update() {
    // No need for updates in win scene
}

void WinScene::render() {
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(Game::renderer);
    
    if (winText) {
        SDL_RenderCopy(Game::renderer, winText, NULL, &textRect);
    }
    
    SDL_RenderPresent(Game::renderer);
}

void WinScene::clean() {
    if (winText) {
        SDL_DestroyTexture(winText);
        winText = nullptr;
    }
}