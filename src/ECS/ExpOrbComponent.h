#pragma once

// --- Includes ---
#include "ECS.h"
#include "Components.h" // Includes TransformComponent, ColliderComponent indirectly



// --- Class Definition ---

// Represents an experience orb dropped by enemies, collected by the player on collision.
class ExpOrbComponent : public Component {
private:
    // --- Private Members ---
    bool initialized = false; // Initialization flag

public:
    // --- Public Members ---
    int experienceAmount; // Amount of experience this orb grants

    // Pointers to components on the same entity (orb)
    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;

    bool collected = false; // Flag indicating if the orb has been collected

    // --- Constructor ---
    // Initializes the experience amount for the orb.
    ExpOrbComponent(int exp) : experienceAmount(exp) {}

    // --- Public Methods ---

    // Component Lifecycle Overrides (Declarations only, definitions in .cpp)
    void init() override;
    void update() override;


}; // End ExpOrbComponent class