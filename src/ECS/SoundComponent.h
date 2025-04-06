#pragma once

#include <SDL_mixer.h>

#include <map>
#include <string>
#include <vector>  

#include "../AssetManager.h"  
#include "../game.h"          
#include "ECS.h"

class SoundComponent : public Component {
   private:

    std::map<std::string, std::string> soundEffectIDs;

    std::string backgroundMusicID = "";

    bool playMusicOnStart = false;
    int musicLoops = -1;  

   public:

    SoundComponent() = default;  

    void addSoundEffect(const std::string& internalName,
                        const std::string& assetID) {
        soundEffectIDs[internalName] = assetID;
    }

    void setBackgroundMusic(const std::string& assetID, bool playOnStart = true,
                            int loops = -1) {
        backgroundMusicID = assetID;
        playMusicOnStart = playOnStart;  
        musicLoops = loops;

        if (playMusicOnStart) {
            playBackgroundMusic(musicLoops);
        }
    }

    void playBackgroundMusic(int loops = -1) {
        if (backgroundMusicID.empty()) {

            return;
        }
        if (!Game::instance || !Game::instance->assets) {
            std::cerr << "SoundComponent Error: Cannot play music, Game "
                         "instance or AssetManager is null!"
                      << std::endl;
            return;
        }

        Mix_Music* music = Game::instance->assets->GetMusic(backgroundMusicID);
        if (music) {
            Mix_HaltMusic();  
            if (Mix_PlayMusic(music, loops) == -1) {
                std::cerr << "SoundComponent: Failed to play music ID '"
                          << backgroundMusicID << "': " << Mix_GetError()
                          << std::endl;
            }
        } else {
            std::cerr << "SoundComponent: Could not get music with asset ID: "
                      << backgroundMusicID << std::endl;
        }
    }

    int playSoundEffect(const std::string& internalName, int loops = 0) {
        int channel = -1;
        if (!Game::instance || !Game::instance->assets) {
            std::cerr << "SoundComponent Error: Cannot play sound effect, Game "
                         "instance or AssetManager is null!"
                      << std::endl;
            return channel;
        }

        if (soundEffectIDs.count(internalName)) {
            const std::string& assetID = soundEffectIDs[internalName];
            Mix_Chunk* sound = Game::instance->assets->GetSoundEffect(assetID);
            if (sound) {

                channel = Mix_PlayChannel(-1, sound, loops);
                if (channel == -1) {
                    std::cerr
                        << "SoundComponent: Mix_PlayChannel failed for SFX '"
                        << internalName << "' (Asset: " << assetID
                        << "). Error: " << Mix_GetError() << std::endl;
                } else {

                    Mix_Volume(channel, Game::getSfxVolume());
                }
            } else {
                std::cerr << "SoundComponent: Could not get sound effect chunk "
                             "with asset ID: "
                          << assetID << std::endl;
            }
        } else {
            std::cerr << "SoundComponent: No sound effect registered with "
                         "internal name: "
                      << internalName << std::endl;
        }
        return channel;
    }

    void stopMusic() { Mix_HaltMusic(); }

    void init() override {}

};  