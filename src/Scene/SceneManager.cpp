#include "SceneManager.h"
#include <iostream>

SceneManager* SceneManager::instance = nullptr;

SceneManager::SceneManager() : currentScene(SceneType::Menu) {
    instance = this;
}

SceneManager::~SceneManager() {
    clean();
}

void SceneManager::addScene(SceneType type, std::unique_ptr<Scene> scene) {
    scenes[type] = std::move(scene);
}

Scene* SceneManager::getScene(SceneType type) {
    if (scenes.find(type) != scenes.end()) {
        return scenes[type].get();
    }
    return nullptr;
}

void SceneManager::switchToScene(SceneType type) {
    if (scenes.find(type) != scenes.end()) {
        currentScene = type;
        scenes[currentScene]->init();
    } else {
        std::cerr << "Scene not found!" << std::endl;
    }
}

void SceneManager::handleEvents(SDL_Event& event) {
    if (scenes.find(currentScene) != scenes.end()) {
        scenes[currentScene]->handleEvents(event);
    }
}

void SceneManager::update() {
    if (scenes.find(currentScene) != scenes.end()) {
        scenes[currentScene]->update();
    }
}

void SceneManager::render() {
    if (scenes.find(currentScene) != scenes.end()) {
        scenes[currentScene]->render();
    }
}

void SceneManager::clean() {
    for (auto& scene : scenes) {
        scene.second->clean();
    }
    scenes.clear();
}