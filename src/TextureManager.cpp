#include "TextureManager.h"
#include <SDL_image.h>
#include <iostream> 
#include <SDL.h>    

SDL_Texture* TextureManager::LoadTexture(const char* fileName) {
    SDL_Surface *tmpSurface = IMG_Load(fileName);

    if (!tmpSurface) {
        std::cerr << "Error: Failed to load image '" << fileName << "'. IMG_Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
    SDL_FreeSurface(tmpSurface); 

    if (!tex) {
        std::cerr << "Error: Failed to create texture from surface for '" << fileName << "'. SDL_Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return tex;
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, double angle, SDL_RendererFlip flip) {
    if (!Game::renderer || !tex) {
        return;
    }
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, angle, NULL, flip);
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    if (!Game::renderer || !tex) {
        return;
    }

    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0.0, NULL, flip);
}