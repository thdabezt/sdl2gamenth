#pragma once
#include <map>
#include <string>
#include "TextureManager.h"
#include "Vector2D.h"
#include "ECS/ECS.h"
#include <SDL_mixer.h>

class AssetManager {
    public:
        AssetManager(Manager* man);
        ~AssetManager();
    
        // --- Update CreateProjectile signature ---
        void CreateProjectile(Vector2D pos, Vector2D vel, int range, int damage, int size, std::string id, int pierce = 1); // Add pierce
    
        void AddTexture(std::string id, const char* path);
        SDL_Texture* GetTexture(std::string id);
    
        void AddSoundEffect(std::string id, const char* path);
        Mix_Chunk* GetSoundEffect(std::string id);

        // --- Music ---
        void AddMusic(std::string id, const char* path);
        Mix_Music* GetMusic(std::string id);
    private:
        Manager* manager;
        std::map<std::string, SDL_Texture*> textures;
        std::map<std::string, Mix_Chunk*> soundEffects; 
        std::map<std::string, Mix_Music*> musicTracks;  
    };