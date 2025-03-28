#pragma once

#include "Components.h"
#include <SDL.h>
#include "../TextureManager.h"
#include "Animation.h"
#include <map>
#include "../AssetManager.h"

class SpriteComponent : public Component {
private:
    TransformComponent *transform;
    SDL_Texture *texture;
        SDL_Rect srcRect, destRect;

    bool animated = false;
    int frames = 0;
    int speed = 100; //ms


public:
bool isHit = false;
    Uint32 hitTime = 0;
    Uint32 hitDuration = 150; // Flash duration in milliseconds
    SDL_Color tint = {255, 255, 255, 255}; // Add this line to declare the tint variable
    // Add this getter method if it doesn't exist yet
    SDL_Texture* getTexture() {
        return texture;
    }
    
    
    int animIndex = 0;

    std::map<const char*, Animation> animations;

    SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;

    SpriteComponent() = default;
    SpriteComponent(std::string id){
        setTex(id);
    }
    SpriteComponent(std::string id, bool isAnimated){
        animated = isAnimated;
        Animation idle = Animation(0, 3, 200);
        Animation walk = Animation(1, 6, 200);

        animations.emplace("Idle", idle);
        animations.emplace("Walk", walk);

        Play("Idle");
        
        setTex(id);
    }

    ~SpriteComponent() {
        
    }
    void setTex(std::string id){ {
        texture = Game::assets->GetTexture(id);
    }}

    void init() override {
        transform = &entity->getComponent<TransformComponent>();
        srcRect.x = srcRect.y = 0;
        srcRect.w = transform->width;
        srcRect.h = transform->height; 
        
    }

    void update() override {
        if(animated){
            srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
        }
        srcRect.y = animIndex * transform->height;
        
        destRect.x = static_cast<int>(transform->position.x) - Game::camera.x;
        destRect.y = static_cast<int>(transform->position.y) - Game::camera.y;
        destRect.w = transform->width * transform->scale;
        destRect.h = transform->height * transform->scale;
    }

    void draw() override {
        // Calculate the current tint color
        SDL_Color currentTint = tint;
        
        // If hit, apply red tint for this specific entity
        if (isHit) {
            if (SDL_GetTicks() > hitTime + hitDuration) {
                // Reset tint after duration
                isHit = false;
                currentTint = {255, 255, 255, 255}; // Reset to normal color
            } else {
                // Use red tint during hit duration
                currentTint = {255, 100, 100, 255}; // Red tint
            }
        }
        
        // Apply the current tint for this render call only
        SDL_SetTextureColorMod(texture, currentTint.r, currentTint.g, currentTint.b);
        
        // Draw the sprite
        TextureManager::Draw(texture, srcRect, destRect, spriteFlip);
        
        // Reset texture color back to normal after drawing
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    }

    void Play(const char* animName){
        frames = animations[animName].frames;
        animIndex = animations[animName].index;
        speed = animations[animName].speed;
    }

};

