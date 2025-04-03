#pragma once
#include <string>
#include <SDL.h>
#include "Components.h"
#include "../game.h"
#include "../TextureManager.h"
#include "../Vector2D.h"
#include <iostream> // For error logging

class ColliderComponent : public Component {
private:
    bool initialized = false; // <<< ADD Initialization Flag

public:
    SDL_Rect collider;
    std::string tag;

    SDL_Texture* tex = nullptr; // Initialize
    SDL_Rect srcR, destR;

    TransformComponent* transform = nullptr; // Initialize to nullptr
    Vector2D position; // Stores collider's top-left corner

    int colliderWidth = 0;
    int colliderHeight = 0;

    ColliderComponent(std::string t) : tag(std::move(t)) { }

    ColliderComponent(std::string t, int cWidth, int cHeight)
     : tag(std::move(t)), colliderWidth(cWidth), colliderHeight(cHeight) { }

    ColliderComponent(std::string t, int xpos, int ypos, int size)
        : tag(std::move(t)), position(static_cast<float>(xpos), static_cast<float>(ypos)), colliderWidth(size), colliderHeight(size) {
        collider.x = xpos;
        collider.y = ypos;
        collider.w = size;
        collider.h = size;
    }


    void init() override {
        initialized = false; // Reset flag
        if (!entity) {
            std::cerr << "Error in ColliderComponent::init: Entity is null for tag '" << tag << "'!" << std::endl;
            return;
        }

        // Dynamic colliders need TransformComponent
        if (tag != "terrain") {
            if (!entity->hasComponent<TransformComponent>()) {
                 std::cerr << "Error in ColliderComponent::init: Entity with tag '" << tag << "' missing TransformComponent!" << std::endl;
                 // Don't add default here, let init fail if transform required
                 return; // Fail init
            }
             transform = &entity->getComponent<TransformComponent>();

             if (!transform) {
                  std::cerr << "Error in ColliderComponent::init: Failed to get TransformComponent for tag '" << tag << "'!" << std::endl;
                  return; // Fail init
             }

            if (colliderWidth == 0 || colliderHeight == 0) {
                colliderWidth = transform->width * transform->scale;
                colliderHeight = transform->height * transform->scale;
            }
        } else {
             // Static terrain collider, doesn't need transform for basic position
             transform = nullptr; // Ensure transform is null for terrain
              if (Game::instance && Game::instance->assets) {
                  tex = Game::instance->assets->GetTexture("border"); // Example texture
              } else {
                  std::cerr << "Warning in ColliderComponent::init (terrain): Game instance or assets missing!" << std::endl;
              }
              srcR = {0, 0, 32, 32};
        }

        collider.w = colliderWidth;
        collider.h = colliderHeight;

        initialized = true; // <<< SET Flag on success
    }

    void update() override {
        if (!initialized) return; // <<< CHECK Flag

        if (transform) { // If it's a dynamic collider (has transform)
             // Calculate position based on transform
             if (tag == "player") {
                 collider.x = static_cast<int>(transform->position.x) + 50;
                 collider.y = static_cast<int>(transform->position.y) + 70;
             }
             else if (tag == "zombie" || tag == "aligator1" || tag == "aligator2" || 
                tag == "bear1" || tag == "bear2" || tag == "eliteskeleton_shield" || 
                tag == "enemy1" || tag == "ina1" || tag == "ina2" || tag == "ina3" || 
                tag == "kfc1" || tag == "kfc2" || 
                tag == "skeleton1" || tag == "skeleton2" || tag == "skeleton3" || 
                tag == "skeleton4" || tag == "skeleton5" || tag == "skeleton_shield") {
                 collider.x = static_cast<int>(transform->position.x) + 32;
                 collider.y = static_cast<int>(transform->position.y) + 60;
             }
             else if (tag == "projectile" || tag == "exp_orb") {
                 collider.x = static_cast<int>(transform->position.x);
                 collider.y = static_cast<int>(transform->position.y);
             }
             else { // Default
                 collider.x = static_cast<int>(transform->position.x);
                 collider.y = static_cast<int>(transform->position.y);
             }
        } else if (tag == "terrain") {
            // Use position set in constructor for static terrain
            collider.x = static_cast<int>(position.x);
            collider.y = static_cast<int>(position.y);
        } else {
             // Should not happen if init succeeded and it's not terrain
             return;
        }

        // Update internal position vector regardless of type
        position.x = static_cast<float>(collider.x);
        position.y = static_cast<float>(collider.y);

        // Update destR for drawing (depends on camera) - maybe belongs in draw?
        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
        destR.w = collider.w;
        destR.h = collider.h;
    }

    void draw() override {
        // if (!initialized) return; // Optional check

        bool debug = false ;
        if(!debug) return;
        if (!Game::renderer) return;

        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 255);
        SDL_Rect debugRect = { destR.x, destR.y, collider.w, collider.h }; // Use destR for camera offset
        SDL_RenderDrawRect(Game::renderer, &debugRect);
        SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    }
};