#include "AssetManager.h"
#include "ECS/Components.h"
#include <iostream> // For error reporting
#include <SDL_mixer.h> // For sound effects and music
AssetManager::AssetManager(Manager* Man) : manager(Man){

}

AssetManager::~AssetManager(){

}
void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int damage, int size, std::string id, int pierce) { // Removed range
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, size, size, 1);
    projectile.addComponent<SpriteComponent>(id);
     // --- ADD ProjectileComponent and Set Spinning Flag ---
    projectile.addComponent<ProjectileComponent>(damage, vel, pierce);
     if (id == "boss_projectile") { // Check the asset ID
         if (projectile.hasComponent<ProjectileComponent>()) {
             projectile.getComponent<ProjectileComponent>().isSpinning = true; // Sets flag on the FIRST component
             // --- DEBUG LOG ---
             // std::cout << "[DEBUG] AssetManager: Set isSpinning=true for projectile with ID: " << id << std::endl;
             // --- END DEBUG LOG ---
         }
     }
     // --- END ---
    projectile.addComponent<ColliderComponent>("projectile");
    projectile.addGroup(Game::groupProjectiles);
}

void AssetManager::AddTexture(std::string id, const char* path){
    // std::cout << "DEBUG: AssetManager attempting to load Texture. ID: '" << id << "', Path: '" << path << "'" << std::endl; // Log attempt
    SDL_Texture* loadedTexture = TextureManager::LoadTexture(path);
    if (loadedTexture == nullptr) {
        std::cerr << "ERROR: AssetManager failed to load texture for ID: '" << id << "'" << std::endl; // Log failure
    } else {
        // std::cout << "DEBUG: AssetManager successfully loaded texture for ID: '" << id << "'" << std::endl; // Log success
        textures.emplace(id, loadedTexture);
    }
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
        // std::cerr << "Failed to load music: " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        musicTracks.emplace(id, music);
    }
}

Mix_Music* AssetManager::GetMusic(std::string id) {
    if (musicTracks.count(id)) {
        return musicTracks[id];
    }
    // std::cerr << "Music track not found: " << id << std::endl;
    return nullptr;
}