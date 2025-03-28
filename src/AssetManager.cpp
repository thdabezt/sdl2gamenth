#include "AssetManager.h"
#include "ECS/Components.h"
#include <iostream> // For error reporting
#include <SDL_mixer.h> // For sound effects and music
AssetManager::AssetManager(Manager* Man) : manager(Man){

}

AssetManager::~AssetManager(){

}
void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int range, int damage, int size, std::string id, int pierce) { // Add pierce parameter
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, size, size, 1);
    projectile.addComponent<SpriteComponent>(id);
    projectile.addComponent<ProjectileComponent>(range, damage, vel, pierce); // Pass pierce here
    projectile.addComponent<ColliderComponent>("projectile"); // Consider collider size relative to 'size'
    projectile.addGroup(Game::groupProjectiles);
}

void AssetManager::AddTexture(std::string id, const char* path){
    textures.emplace(id, TextureManager::LoadTexture(path));
}

SDL_Texture* AssetManager::GetTexture(std::string id){
    return textures[id];
}
void AssetManager::AddSoundEffect(std::string id, const char* path) {
    Mix_Chunk* sound = Mix_LoadWAV(path);
    if (sound == nullptr) {
        std::cerr << "Failed to load sound effect: " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        soundEffects.emplace(id, sound);
    }
}

Mix_Chunk* AssetManager::GetSoundEffect(std::string id) {
    if (soundEffects.count(id)) {
        return soundEffects[id];
    }
    std::cerr << "Sound effect not found: " << id << std::endl;
    return nullptr;
}

void AssetManager::AddMusic(std::string id, const char* path) {
    Mix_Music* music = Mix_LoadMUS(path);
    if (music == nullptr) {
        std::cerr << "Failed to load music: " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        musicTracks.emplace(id, music);
    }
}

Mix_Music* AssetManager::GetMusic(std::string id) {
    if (musicTracks.count(id)) {
        return musicTracks[id];
    }
    std::cerr << "Music track not found: " << id << std::endl;
    return nullptr;
}