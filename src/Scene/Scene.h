#pragma once
#include <SDL.h>
#include <string>

class Scene {
public:
    virtual ~Scene() {}
    virtual void init() = 0;
    virtual void handleEvents(SDL_Event& event) = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void clean() = 0;
};