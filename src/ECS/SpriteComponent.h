#pragma once

#include <SDL.h>

#include <iostream>
#include <map>
#include <string>

#include "../AssetManager.h"
#include "../TextureManager.h"
#include "../game.h"
#include "Animation.h"
#include "Components.h"
#include "ProjectileComponent.h"

class TransformComponent;

class SpriteComponent : public Component {
   private:
    TransformComponent* transform = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Rect srcRect = {0, 0, 0, 0};
    SDL_Rect destRect = {0, 0, 0, 0};

    bool animated = false;
    int frames = 0;
    bool initialized = false;

   public:
    int speed = 100;
    int animIndex = 0;
    std::map<const char*, Animation> animations;
    SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;
    SDL_Color tint = {255, 255, 255, 255};

    double angle = 0.0;
    float rotationSpeed = 180.0f;

    bool isHit = false;
    Uint32 hitTime = 0;
    Uint32 hitDuration = 150;

    SpriteComponent() = default;

    SpriteComponent(std::string id) { setTex(id); }

    SpriteComponent(std::string id, bool isAnimated) {
        animated = isAnimated;

        Animation idle = Animation(0, 3, 200);
        Animation walk = Animation(1, 6, 200);
        animations.emplace("Idle", idle);
        animations.emplace("Walk", walk);

        Play("Idle");
        setTex(id);
    }

    ~SpriteComponent() override {}

    void init() override {
        initialized = false;
        if (!entity) {
            std::cerr << "Error in SpriteComponent::init: Entity is null!"
                      << std::endl;
            return;
        }
        if (!entity->hasComponent<TransformComponent>()) {
            std::cerr << "Error in SpriteComponent::init: Entity missing "
                         "TransformComponent!"
                      << std::endl;
            return;
        }
        transform = &entity->getComponent<TransformComponent>();
        if (!transform) {
            std::cerr << "Error in SpriteComponent::init: Failed to get "
                         "TransformComponent pointer!"
                      << std::endl;
            return;
        }

        srcRect.x = srcRect.y = 0;
        srcRect.w = transform->width;
        srcRect.h = transform->height;

        initialized = true;
    }

    void update() override {
        if (!initialized || !transform) return;

        if (animated && frames > 0 && speed > 0) {
            srcRect.x =
                srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
        } else if (!animated) {
            srcRect.x = 0;
        }

        srcRect.y =
            (transform->height > 0) ? (animIndex * transform->height) : 0;

        destRect.x = static_cast<int>(transform->position.x - Game::camera.x);
        destRect.y = static_cast<int>(transform->position.y - Game::camera.y);
        destRect.w = transform->width * transform->scale;
        destRect.h = transform->height * transform->scale;

        bool shouldSpin = false;
        if (entity->hasComponent<ProjectileComponent>()) {
            shouldSpin = entity->getComponent<ProjectileComponent>().isSpinning;
        }

        if (shouldSpin) {
            const float timeStep = 1.0f / 60.0f;
            angle += rotationSpeed * timeStep;
            if (angle >= 360.0) angle -= 360.0;
            if (angle < 0.0) angle += 360.0;
        } else {
            angle = 0.0;
        }
    }

    void draw() override {
        if (!initialized || !texture || !transform) return;

        SDL_Color currentTint = tint;
        if (isHit) {
            if (SDL_GetTicks() > hitTime + hitDuration) {
                isHit = false;

            } else {
                currentTint = {255, 100, 100, 255};
            }
        }

        SDL_SetTextureColorMod(texture, currentTint.r, currentTint.g,
                               currentTint.b);
        TextureManager::Draw(texture, srcRect, destRect, angle, spriteFlip);
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    }

    void setTex(std::string id) {
        if (Game::instance && Game::instance->assets) {
            texture = Game::instance->assets->GetTexture(id);
            if (!texture) {
                std::cerr << "Warning in SpriteComponent::setTex: Texture ID '"
                          << id << "' not found in AssetManager!" << std::endl;
            }
        } else {
            std::cerr << "Error in SpriteComponent::setTex: Game::instance or "
                         "Game::instance->assets is null!"
                      << std::endl;
            texture = nullptr;
        }
    }

    SDL_Texture* getTexture() { return texture; }

    void Play(const char* animName) {
        auto it = animations.find(animName);
        if (it != animations.end()) {
            const Animation& anim = it->second;

            if (this->animIndex != anim.index || this->frames != anim.frames ||
                this->speed != anim.speed) {
                this->frames = anim.frames;
                this->animIndex = anim.index;
                this->speed = anim.speed > 0 ? anim.speed : 100;
            }
        } else {
            std::cerr << "Warning: Animation '" << animName
                      << "' not found in SpriteComponent!" << std::endl;

            this->frames = 1;
            this->animIndex = 0;
            this->speed = 100000;
        }
    }
};