#pragma once

#include "../Vector2D.h"
#include "Components.h"

struct TransformComponent : public Component {
   public:
    Vector2D position;
    Vector2D velocity;

    int height = 32;
    int width = 32;
    int scale = 1;

    TransformComponent() {
        position.Zero();
        velocity.Zero();
    }

    TransformComponent(int sc) : scale(sc) {
        position.x = 400;
        position.y = 320;
        velocity.Zero();
    }

    TransformComponent(float x, float y) : position(x, y) { velocity.Zero(); }

    TransformComponent(float x, float y, int h, int w, float s)
        : position(x, y), height(h), width(w), scale(static_cast<int>(s)) {
        velocity.Zero();
    }

    void init() override { velocity.Zero(); }

    void update() override {
        position.x += velocity.x;
        position.y += velocity.y;
    }
};