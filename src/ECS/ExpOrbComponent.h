#pragma once

#include "ECS.h"
#include "Components.h"
#include <iostream>

// Forward declare needed components
class TransformComponent;
class ColliderComponent;
class Player;

class ExpOrbComponent : public Component {
private:
    bool initialized = false; // <<< ADD Initialization Flag

public:
    int experienceAmount;

    // Pointers to components on the same entity (orb)
    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;
    bool collected = false;

    // Keep inline constructor definition here
    ExpOrbComponent(int exp) : experienceAmount(exp) {}

    // Declarations for methods defined in .cpp
    void init() override;
    void update() override;

    // void draw() override; // Optional draw method
};