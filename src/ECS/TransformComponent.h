#pragma once

// --- Includes ---
#include "Components.h" // Includes ECS.h indirectly
#include "../Vector2D.h" // Explicit include for Vector2D

// --- Struct Definition ---

// Holds position, velocity, size, and scale data for an entity.
struct TransformComponent : public Component {
public: // Struct members are public by default, added for clarity
    // --- Public Members ---
    Vector2D position; // Top-left position (world coordinates)
    Vector2D velocity; // Change in position per update cycle

    int height = 32;   // Default height
    int width = 32;    // Default width
    int scale = 1;     // Scaling factor

    // --- Constructors ---
    // Default constructor: Initializes position and velocity to zero.
    TransformComponent() {
        position.Zero();
        velocity.Zero();
    }

    // Constructor with scale: Initializes position to default center, velocity to zero.
    TransformComponent(int sc) : scale(sc) {
        // Default position (center of typical screen, adjust if needed)
        position.x = 400;
        position.y = 320;
        velocity.Zero();
        // height and width use defaults
    }

    // Constructor with position: Initializes velocity to zero, uses default size/scale.
    TransformComponent(float x, float y) : position(x, y) {
        velocity.Zero();
        // height, width, scale use defaults
    }

    // Full constructor: Initializes all members.
    TransformComponent(float x, float y, int h, int w, float s)
        : position(x, y), height(h), width(w), scale(static_cast<int>(s)) { // Ensure scale is int
        velocity.Zero();
    }

    // --- Public Methods ---
    // Initializes velocity to zero when the component is added.
    void init() override {
        velocity.Zero();
    }

    // Updates the position based on the current velocity.
    void update() override {
        position.x += velocity.x;
        position.y += velocity.y;
    }

}; // End TransformComponent struct