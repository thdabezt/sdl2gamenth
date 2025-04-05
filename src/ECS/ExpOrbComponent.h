#pragma once

#include "Components.h"  
#include "ECS.h"

class ExpOrbComponent : public Component {
   private:

    bool initialized = false;  

   public:

    int experienceAmount;  

    TransformComponent* transform = nullptr;
    ColliderComponent* collider = nullptr;

    bool collected = false;  

    ExpOrbComponent(int exp) : experienceAmount(exp) {}

    void init() override;
    void update() override;

};  