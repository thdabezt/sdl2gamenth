#pragma once
#include "Scene.h"
#include "../game.h"

class GameScene : public Scene {
public:
    GameScene();
    ~GameScene();
    
    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;
    void resetGame();
    
private:
    Game* game;
};