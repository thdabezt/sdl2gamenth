#pragma once
#include "Scene.h"
#include "../game.h"

class WinScene : public Scene {
public:
    WinScene();
    ~WinScene();
    
    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;
    
private:
    SDL_Texture* winText;
    SDL_Rect textRect;
};