#pragma once
#include "Scene.h"
#include <map>
#include <string>
#include <memory>

enum class SceneType {
    Menu,
    Game,
};

class SceneManager {
public:
    static SceneManager* instance;
    
    SceneManager();
    ~SceneManager();
    
    void addScene(SceneType type, std::unique_ptr<Scene> scene);
    void switchToScene(SceneType type);
    void handleEvents(SDL_Event& event);
    void update();
    void render();
    void clean();
    Scene* getScene(SceneType type);
    
private:
    std::map<SceneType, std::unique_ptr<Scene>> scenes;
    SceneType currentScene;
};