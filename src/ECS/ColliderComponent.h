#pragma once

#include <SDL.h>

#include <iostream>
#include <string>

#include "../TextureManager.h"
#include "../Vector2D.h"
#include "../game.h"
#include "Components.h"

class ColliderComponent : public Component {
   private:
    bool initialized = false;

   public:
    SDL_Rect collider;
    std::string tag;

    SDL_Texture* tex = nullptr;
    SDL_Rect srcR = {0, 0, 0, 0};
    SDL_Rect destR = {0, 0, 0, 0};

    TransformComponent* transform = nullptr;
    Vector2D position;
    int colliderWidth = 0;
    int colliderHeight = 0;

    ColliderComponent(std::string t) : tag(std::move(t)) {}

    ColliderComponent(std::string t, int cWidth, int cHeight)
        : tag(std::move(t)), colliderWidth(cWidth), colliderHeight(cHeight) {}

    ColliderComponent(std::string t, int xpos, int ypos, int size)
        : tag(std::move(t)),
          position(static_cast<float>(xpos), static_cast<float>(ypos)),
          colliderWidth(size),
          colliderHeight(size) {
        collider.x = xpos;
        collider.y = ypos;
        collider.w = size;
        collider.h = size;
    }

    void init() override {
        initialized = false;
        if (!entity) {
            std::cerr
                << "Error in ColliderComponent::init: Entity is null for tag '"
                << tag << "'!" << std::endl;
            return;
        }

        if (tag != "terrain") {
            if (!entity->hasComponent<TransformComponent>()) {
                std::cerr
                    << "Error in ColliderComponent::init: Entity with tag '"
                    << tag << "' missing TransformComponent!" << std::endl;
                return;
            }
            transform = &entity->getComponent<TransformComponent>();
            if (!transform) {
                std::cerr << "Error in ColliderComponent::init: Failed to get "
                             "TransformComponent for tag '"
                          << tag << "'!" << std::endl;
                return;
            }

            if (colliderWidth == 0) {
                colliderWidth =
                    static_cast<int>(transform->width * transform->scale);
            }
            if (colliderHeight == 0) {
                colliderHeight =
                    static_cast<int>(transform->height * transform->scale);
            }

        } else {
            transform = nullptr;

            if (Game::instance && Game::instance->assets) {
            }
            srcR = {0, 0, 32, 32};
        }

        collider.w = colliderWidth;
        collider.h = colliderHeight;

        initialized = true;
    }

    void update() override {
        if (!initialized) return;

        if (transform) {
            float basePosX = transform->position.x;
            float basePosY = transform->position.y;

            if (tag == "player") {
                collider.x = static_cast<int>(basePosX + 50);
                collider.y = static_cast<int>(basePosY + 70);
            }

            else if (tag == "zombie" || tag == "aligator1" ||
                     tag == "aligator2" || tag == "bear1" || tag == "bear2" ||
                     tag == "eliteskeleton_shield" || tag == "ina1" ||
                     tag == "ina2" || tag == "ina3" || tag == "kfc1" ||
                     tag == "kfc2" || tag == "skeleton1" ||
                     tag == "skeleton2" || tag == "skeleton3" ||
                     tag == "skeleton4" || tag == "skeleton5" ||
                     tag == "skeleton_shield" || tag == "boss") {
                collider.x = static_cast<int>(basePosX + 32);
                collider.y = static_cast<int>(basePosY + 60);
            } else if (tag == "projectile" || tag == "exp_orb") {
                collider.x = static_cast<int>(basePosX);
                collider.y = static_cast<int>(basePosY);
            } else {
                collider.x = static_cast<int>(basePosX);
                collider.y = static_cast<int>(basePosY);
            }
        } else if (tag == "terrain") {
            collider.x = static_cast<int>(position.x);
            collider.y = static_cast<int>(position.y);
        } else {
            return;
        }

        position.x = static_cast<float>(collider.x);
        position.y = static_cast<float>(collider.y);

        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
        destR.w = collider.w;
        destR.h = collider.h;
    }

    void draw() override {
        bool debug_draw = false;
        if (!debug_draw || !initialized || !Game::renderer) return;

        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 150);

        SDL_RenderDrawRect(Game::renderer, &destR);
    }
};