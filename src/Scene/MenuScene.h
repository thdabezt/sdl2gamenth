#pragma once
#include "Scene.h"
#include "../game.h"

class MenuScene : public Scene {
public:
    MenuScene();
    ~MenuScene();
    
    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;
    
private:
    SDL_Texture* menuText;
    SDL_Rect textRect;
};