#include "game.h"
#include <iostream>
#include <SDL.h>
#include "constants.h"
#include <SDL_image.h>
#include "TextureManager.h"
#include "GameObject.h"

GameObject *player;
GameObject *enemy1;

SDL_Renderer *Game::renderer = nullptr;

Game::Game() : isRunning(false), window(nullptr), count(0){


}

Game::~Game(){

}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen){
    int flags = 0;
    if(fullscreen){
        flags = SDL_WINDOW_FULLSCREEN;
    }
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0){
        std::cout<< "Starting" <<std::endl;

        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if(window){
            std::cout<<"Ran Game - Created window"<<std::endl;

        }
        renderer = SDL_CreateRenderer(window,-1,0);
        if(renderer){
            std::cout<< "Renderer running" <<std::endl;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
            isRunning = false;
            return;
        }
        isRunning = true;
    }
    else {
        isRunning = false;
    }
    player = new GameObject(playerSprites, CHAR_X, CHAR_Y, CHAR_W, CHAR_H);
    enemy1 = new GameObject(enemy1Sprites, E1_X, E1_Y, E1_W, E1_H);
}

void Game::handleEvents(){
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        isRunning = false;
        break;
    
    default:
        break;
    }
}

void Game::update(){
    player->Update();
    enemy1->Update();
    count++;
    std::cout<< count << std::endl;
}

void Game::render(){
    SDL_RenderClear(renderer);
    player->Render();
    enemy1->Render();
    SDL_RenderPresent(renderer);
}

void Game::clean(){
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
    SDL_Quit();
    std::cout<< "Game quitted"<< std::endl;

}
