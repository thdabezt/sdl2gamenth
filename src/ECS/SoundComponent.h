#pragma once

#include "../game.h" // For Game::assets
#include "ECS.h"
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <map>
#include "../AssetManager.h" // <<< Add this line

class SoundComponent : public Component {
private:
    // Store IDs of sounds this component can play
    std::map<std::string, std::string> soundEffectIDs; // Map internal name (e.g., "shoot") to asset ID ("gunshot_sound")
    std::string backgroundMusicID = "";

    bool playMusicOnStart = false;
    int musicLoops = -1; // -1 for infinite looping
    
public:
    SoundComponent() = default;

    // Optional: Add sounds during construction or later
    void addSoundEffect(const std::string& internalName, const std::string& assetID) {
        soundEffectIDs[internalName] = assetID;
    }

    void playBackgroundMusic(int loops = -1) {
        if (!backgroundMusicID.empty()) {
            // 'music' is declared here:
            Mix_Music* music = Game::instance->assets->GetMusic(backgroundMusicID);
    
            // Check if GetMusic returned a valid pointer:
            if (music) {
                Mix_HaltMusic();
                // std::cout << "Attempting Mix_PlayMusic..." << std::endl; // <<< ADD THIS BACK
                if (Mix_PlayMusic(music, loops) == -1) {
                    //  std::cerr << "SoundComponent: Failed to play music: " << Mix_GetError() << std::endl;
                } else {
                    //  std::cout << "Mix_PlayMusic finished (returned 0)." << std::endl; // <<< ADD THIS BACK
                }
            }  else {
                //  std::cerr << "SoundComponent: Could not get music with asset ID: " << backgroundMusicID << std::endl;
            }
        } else {
            //  std::cerr << "SoundComponent: No background music ID set." << std::endl;
        }
    }

    void init() override {
        
    }

    int playSoundEffect(const std::string& internalName, int loops = 0) {
        int channel = -1; // Store the result
        if (soundEffectIDs.count(internalName)) {
            Mix_Chunk* sound = Game::instance->assets->GetSoundEffect(soundEffectIDs[internalName]);
            if (sound) {
                // <<< ADD DEBUG LOG Before Playing >>>
                // std::cout << "DEBUG: Attempting to play SFX '" << internalName
                //           << "' (Asset: " << soundEffectIDs[internalName]
                //           << ") on any channel. Current SFX Volume Setting: "
                //           << Game::instance->getSfxVolume() << std::endl; // Access volume via Game instance
    
                // Mix_PlayChannel takes channel (-1 for first available), chunk*, loops (0=play once)
                channel = Mix_PlayChannel(-1, sound, loops); // Play the sound
    
                // <<< ADD DEBUG LOG After Playing >>>
                if (channel == -1) {
                    // std::cerr << "DEBUG: Mix_PlayChannel failed for SFX '" << internalName
                    //           << "'. SDL_mixer Error: " << Mix_GetError() << std::endl;
                } else {
                    // std::cout << "DEBUG: Mix_PlayChannel succeeded for SFX '" << internalName
                    //           << "' on channel " << channel << "." << std::endl;
                    // Optional: Set volume on the specific channel *after* playing,
                    // if Mix_Volume(-1, ...) isn't working as expected.
                    Mix_Volume(channel, Game::instance->getSfxVolume());
                }
    
            } else {
                // std::cerr << "SoundComponent: Could not get sound effect chunk with asset ID: " << soundEffectIDs[internalName] << std::endl;
            }
        } else {
            // std::cerr << "SoundComponent: No sound effect registered with internal name: " << internalName << std::endl;
        }
        return channel; // Return the channel used or -1 on failure
    }

    // Play the background music associated with this component
    
    void setBackgroundMusic(const std::string& assetID, bool playOnStart = true, int loops = -1) {
        backgroundMusicID = assetID;
        playMusicOnStart = playOnStart; // Keep this for potential future reference if needed
        musicLoops = loops;
    
        // --- ADD THIS ---
        // If we want to play on start, try playing the music immediately after setting it.
        if (playMusicOnStart) {
            // You can optionally add the debug cout lines around playBackgroundMusic here if you want
            // std::cout << "Attempting Mix_PlayMusic from setBackgroundMusic..." << std::endl;
            playBackgroundMusic(musicLoops);
            // std::cout << "Mix_PlayMusic finished from setBackgroundMusic." << std::endl;
        }
        // --- END ADD ---
    }

    void stopMusic() {
        Mix_HaltMusic();
    }

    // Optional: Add methods to stop specific channels, fade music, etc.
};