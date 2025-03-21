#include "GameObject.h"
#include "TextureManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

GameObject::GameObject(const char* texturesheet, int x, int y, int w, int h){
    xpos = x;
    ypos = y;
    width = w;
    height = h;
    
    objTexture = TextureManager::LoadTexture(texturesheet);
}

void GameObject::Update(){
    
    srcRect.h = height;
    srcRect.w = width;
    srcRect.x = 0;
    srcRect.y = 0;
    
    destRect.x = xpos;
    destRect.y = ypos;
    destRect.w = width; // Use original width
    destRect.h = height; // Use original height

}

void GameObject::Render(){
    SDL_RenderCopy(Game::renderer, objTexture, &srcRect, &destRect);
}