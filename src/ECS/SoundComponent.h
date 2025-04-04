#pragma once

// --- Includes ---
#include "../game.h" // For Game::instance->assets access
#include "ECS.h"
#include <SDL_mixer.h>
#include <string>
#include <vector> // Included via ECS.h but good practice to be explicit if needed
#include <map>
#include "../AssetManager.h" // For AssetManager methods used inline

// --- Class Definition ---

class SoundComponent : public Component {
private:
    // --- Private Members ---
    // Maps internal sound names (e.g., "shoot") to AssetManager IDs (e.g., "gunshot_sound")
    std::map<std::string, std::string> soundEffectIDs;
    // AssetManager ID for the background music track
    std::string backgroundMusicID = "";

    // Playback settings for background music
    bool playMusicOnStart = false;
    int musicLoops = -1; // -1 for infinite looping

public:
    // --- Constructor ---
    SoundComponent() = default; // Use default constructor

    // --- Public Methods ---

    // Maps an internal name to an AssetManager sound effect ID for later playback.
    void addSoundEffect(const std::string& internalName, const std::string& assetID) {
        soundEffectIDs[internalName] = assetID;
    }

    // Sets the background music track and optionally plays it immediately.
    void setBackgroundMusic(const std::string& assetID, bool playOnStart = true, int loops = -1) {
        backgroundMusicID = assetID;
        playMusicOnStart = playOnStart; // Store setting
        musicLoops = loops;

        // If requested, try playing the music immediately after setting it.
        if (playMusicOnStart) {
            playBackgroundMusic(musicLoops);
        }
    }

    // Plays the currently set background music.
    void playBackgroundMusic(int loops = -1) {
        if (backgroundMusicID.empty()) {
            // std::cerr << "SoundComponent: No background music ID set." << std::endl; // Keep for debugging if needed
            return;
        }
        if (!Game::instance || !Game::instance->assets) {
            std::cerr << "SoundComponent Error: Cannot play music, Game instance or AssetManager is null!" << std::endl;
            return;
        }

        Mix_Music* music = Game::instance->assets->GetMusic(backgroundMusicID);
        if (music) {
            Mix_HaltMusic(); // Stop any currently playing music
            if (Mix_PlayMusic(music, loops) == -1) {
                 std::cerr << "SoundComponent: Failed to play music ID '" << backgroundMusicID << "': " << Mix_GetError() << std::endl;
            }
        }  else {
             std::cerr << "SoundComponent: Could not get music with asset ID: " << backgroundMusicID << std::endl;
        }
    }

    // Plays a sound effect previously added via addSoundEffect.
    // Returns the channel number used or -1 on failure.
    int playSoundEffect(const std::string& internalName, int loops = 0) {
        int channel = -1;
        if (!Game::instance || !Game::instance->assets) {
            std::cerr << "SoundComponent Error: Cannot play sound effect, Game instance or AssetManager is null!" << std::endl;
            return channel;
        }

        if (soundEffectIDs.count(internalName)) {
            const std::string& assetID = soundEffectIDs[internalName];
            Mix_Chunk* sound = Game::instance->assets->GetSoundEffect(assetID);
            if (sound) {
                // Play on the first available channel (-1), loop 'loops' times (0 = play once)
                channel = Mix_PlayChannel(-1, sound, loops);
                if (channel == -1) {
                    std::cerr << "SoundComponent: Mix_PlayChannel failed for SFX '" << internalName
                              << "' (Asset: " << assetID << "). Error: " << Mix_GetError() << std::endl;
                } else {
                    // Optionally set volume on the specific channel immediately after playing
                    Mix_Volume(channel, Game::getSfxVolume());
                }
            } else {
                std::cerr << "SoundComponent: Could not get sound effect chunk with asset ID: " << assetID << std::endl;
            }
        } else {
            std::cerr << "SoundComponent: No sound effect registered with internal name: " << internalName << std::endl;
        }
        return channel;
    }

    // Stops any currently playing background music.
    void stopMusic() {
        Mix_HaltMusic();
    }

    // Component Lifecycle Overrides
    void init() override {
        
    }

}; // End SoundComponent class