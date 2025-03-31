#pragma once
#include "Components.h"
#include "../Vector2D.h"
// #include "../constants.h"
struct TransformComponent : public Component {
    public:

        Vector2D position;
        Vector2D velocity;

        int speed = 3;

        int height = 32;
        int width = 32;
        int scale = 1;

        TransformComponent() {
            position.Zero();
        }
        TransformComponent(int sc) {
            position.x = 400;
            position.y = 320;
            scale = sc;
        }
        TransformComponent(float x, float y) {
            position.Zero();
        }
        TransformComponent(float x, float y, int h, int w, float s) {
            position.x = x;
            position.y = y;
            height = h;
            width = w;
            scale = s;
        }
        void init() override {
            velocity.Zero();
        }
        void update() override {
            // Only update position if there's actually movement
            // The velocity vector received here should already be correctly
            // normalized (if diagonal) and scaled by playerSpeed from KeyboardController
            if (velocity.x != 0 || velocity.y != 0) {
                position.x += velocity.x;
                position.y += velocity.y;
            }
        }

};