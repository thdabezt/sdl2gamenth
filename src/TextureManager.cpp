#include "TextureManager.h"
#include <SDL_image.h>
#include <iostream> // Include for std::cerr
#include <SDL.h>    // Include for SDL_GetError()

SDL_Texture* TextureManager::LoadTexture(const char* fileName) {
    SDL_Surface *tmpSurface = IMG_Load(fileName);
    // <<< ADD ERROR CHECKING FOR IMG_Load >>>
    if (!tmpSurface) {
        std::cerr << "Error: Failed to load image '" << fileName << "'. IMG_Error: " << IMG_GetError() << std::endl;
        return nullptr; // Return null if loading failed
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
    SDL_FreeSurface(tmpSurface); // Free surface regardless of texture creation success

    // <<< ADD ERROR CHECKING FOR SDL_CreateTextureFromSurface >>>
    if (!tex) {
        std::cerr << "Error: Failed to create texture from surface for '" << fileName << "'. SDL_Error: " << SDL_GetError() << std::endl;
        return nullptr; // Return null if texture creation failed
    }

    // Optional: Log success
    // std::cout << "Successfully loaded texture: " << fileName << std::endl;
    return tex;
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, double angle, SDL_RendererFlip flip) { // Added angle parameter
    if (!Game::renderer || !tex) return;
    // Pass the angle to SDL_RenderCopyEx
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, angle, NULL, flip);
}


void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    if (!Game::renderer || !tex) return;
    // Call SDL_RenderCopyEx with NULL (or 0.0) for the angle when no rotation is needed
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0.0, NULL, flip);
}