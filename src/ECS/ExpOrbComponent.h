#pragma once

#include "ECS.h"
#include "Components.h"     // Include this for base Component class, etc.
// #include "ColliderComponent.h" // Remove the explicit include we added before
#include <iostream>

// Forward declare needed components if not brought in by Components.h for pointers
class TransformComponent;
class ColliderComponent;
class Player; // Forward declare Player if needed for playerManager type

class ExpOrbComponent : public Component {
public:
    int experienceAmount;

    // Pointers to components on the same entity (orb)
    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    bool collected = false;

    ExpOrbComponent(int exp); // Constructor declaration

    void init() override;
    void update() override; // <<< DECLARATION ONLY

    // void draw() override;
};