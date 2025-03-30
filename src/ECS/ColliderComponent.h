#pragma once
#include <string>
#include <SDL.h>
#include "Components.h"
#include "../game.h"
#include "../TextureManager.h"
#include "../Vector2D.h"


class ColliderComponent : public Component {
public:
    SDL_Rect collider;
    std::string tag;

    SDL_Texture* tex;

    SDL_Rect srcR, destR;

    TransformComponent* transform;
    Vector2D position;

    int colliderWidth = 0;
    int colliderHeight = 0; 

    ColliderComponent(std::string t) {
        tag = t;
    }

    // Constructor with custom collision box size
    ColliderComponent(std::string t, int cWidth, int cHeight) {
        tag = t;
        colliderWidth = cWidth;
        colliderHeight = cHeight;
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
        // If no custom size is provided, use the sprite's size
        if (colliderWidth == 0 || colliderHeight == 0) {
            colliderWidth = transform->width * transform->scale;
            colliderHeight = transform->height * transform->scale;
        }

        collider.w = colliderWidth;
        collider.h = colliderHeight;
    }
    // Constructor with tag, position, and size
    ColliderComponent(std::string t, int xpos, int ypos, int size) {
        tag = t;
        collider.x = xpos;
        collider.y = ypos;
        collider.w = size = size;
        colliderWidth = colliderHeight = size;
    }
    
    void update() override {
        // Update collider position based on the TransformComponent
        
        if (tag == "player") {
            collider.x = static_cast<int>(transform->position.x) + 50;
            collider.y = static_cast<int>(transform->position.y) + 70;
            collider.w = colliderWidth;
            collider.h = colliderHeight;
        }
        else if (tag == "zombie" || tag == "aligator1" || tag == "aligator2" || 
            tag == "bear1" || tag == "bear2" || tag == "eliteskeleton_shield" || 
            tag == "enemy1" || tag == "ina1" || tag == "ina2" || tag == "ina3" || 
            tag == "kfc1" || tag == "kfc2" || 
            tag == "skeleton1" || tag == "skeleton2" || tag == "skeleton3" || 
            tag == "skeleton4" || tag == "skeleton5" || tag == "skeleton_shield") {
            collider.x = static_cast<int>(transform->position.x) + 32;
            collider.y = static_cast<int>(transform->position.y) + 60;
            collider.w = colliderWidth;
            collider.h = colliderHeight;
        }
        else if (tag == "projectile") {
            collider.x = static_cast<int>(transform->position.x) ;
            collider.y = static_cast<int>(transform->position.y);
            collider.w = colliderWidth;
            collider.h = colliderHeight;
        }
        else if (tag != "terrain") {
            collider.x = static_cast<int>(transform->position.x);
            collider.y = static_cast<int>(transform->position.y);
            collider.w = colliderWidth;
            collider.h = colliderHeight;
        }
        position.x = collider.x;
        position.y = collider.y;
        // Adjust the collider position relative to the camera
        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
    }

    void draw() override {
        bool debug = false ;
        // Set the color for the collision box (e.g., red)
        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 255);

        // Adjust the collider position relative to the camera
        SDL_Rect debugRect = {
            collider.x - Game::camera.x,
            collider.y - Game::camera.y,
            collider.w,
            collider.h
        };
        if(debug){
        // Draw the collision box
        SDL_RenderDrawRect(Game::renderer, &debugRect);

        // Reset the render color to white
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
        }
       
    }
};

// void draw() override {
    //     if (tag == "terrain") {
    //         TextureManager::Draw(tex, srcR, destR, SDL_FLIP_NONE);
    //     }
    // }