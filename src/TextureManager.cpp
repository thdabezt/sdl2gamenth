#include "TextureManager.h"
#include <SDL_image.h>
#include <iostream>
#include <SDL.h>
SDL_Texture* TextureManager::LoadTexture(const char* fileName){
    SDL_Surface *tmpSurface = IMG_Load(fileName);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
    SDL_FreeSurface(tmpSurface);
    return tex;
}