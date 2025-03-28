#pragma once
#include "Scene.h"
#include "../game.h"

class LoseScene : public Scene {
public:
    LoseScene();
    ~LoseScene();
    
    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;
    
private:
    SDL_Texture* loseText;
    SDL_Rect textRect;
};