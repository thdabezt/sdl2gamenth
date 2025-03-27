#pragma once
#include "Components.h"
#include "../vector2D.h"
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
        TransformComponent(float x, float y, int h, int w, int s) {
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
            position.x += static_cast<int>(velocity.x * speed);
		    position.y += static_cast<int>(velocity.y * speed);

        }

};