// --- Includes ---
#include "TextureManager.h"
#include <SDL_image.h>
#include <iostream> // For std::cerr
#include <SDL.h>    // For SDL_GetError()

// --- Function Definitions ---

SDL_Texture* TextureManager::LoadTexture(const char* fileName) {
    SDL_Surface *tmpSurface = IMG_Load(fileName);
    // Error check for surface loading
    if (!tmpSurface) {
        std::cerr << "Error: Failed to load image '" << fileName << "'. IMG_Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
    SDL_FreeSurface(tmpSurface); // Free surface regardless of texture creation success

    // Error check for texture creation
    if (!tex) {
        std::cerr << "Error: Failed to create texture from surface for '" << fileName << "'. SDL_Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return tex;
}

// Draw function with rotation angle
void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, double angle, SDL_RendererFlip flip) {
    if (!Game::renderer || !tex) {
        return;
    }
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, angle, NULL, flip);
}

// Draw function without rotation (calls Ex with 0 angle)
void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    if (!Game::renderer || !tex) {
        return;
    }
    // Use SDL_RenderCopyEx with 0.0 angle for consistency, or SDL_RenderCopy if performance is critical
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0.0, NULL, flip);
}