#pragma once
#include <string>
#include <SDL.h>
#include "Components.h"
#include "../game.h"
#include "../TextureManager.h"
class ColliderComponent : public Component {
public:
    SDL_Rect collider;
    std::string tag;

    SDL_Texture* tex;


    SDL_Rect srcR, destR;

    TransformComponent* transform;

    ColliderComponent(std::string t) {
        tag = t;
    }

    ColliderComponent(std::string t, int xpos, int ypos, int size)
	{
		tag = t;
		collider.x = xpos;
		collider.y = ypos;
		collider.h = collider.w = size;
	}

    void init() override {
        if (!entity->hasComponent<TransformComponent>()) {
            entity->addComponent<TransformComponent>();
        }
        transform = &entity->getComponent<TransformComponent>();

        if (tag == "terrain") {
            tex = TextureManager::LoadTexture("sprites/map/border.png");
            srcR = {0, 0, 32, 32};
            destR = {collider.x, collider.y, collider.w, collider.h};
        }
        

    }

    void update() override {
        if (tag != "terrain") {
            collider.x = static_cast<int>(transform->position.x);
            collider.y = static_cast<int>(transform->position.y);
            collider.w = transform->width * transform->scale;
            collider.h = transform->height * transform->scale;
        }

        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
    }

    // void draw() override {
    //     if (tag == "terrain") {
    //         TextureManager::Draw(tex, srcR, destR, SDL_FLIP_NONE);
    //     }
    // }
    void draw() override {
        // Set the color for the collision box (e.g., red)
        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 255);

        // Adjust the collider position relative to the camera
        SDL_Rect debugRect = {
            collider.x - Game::camera.x,
            collider.y - Game::camera.y,
            collider.w,
            collider.h
        };

        // Draw the collision box
        SDL_RenderDrawRect(Game::renderer, &debugRect);

        // Reset the render color to white
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    }
    
};
