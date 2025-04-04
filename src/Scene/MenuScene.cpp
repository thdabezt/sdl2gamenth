// --- Includes ---
#include "MenuScene.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "../constants.h"
#include "../TextureManager.h"
#include <iostream>
#include <algorithm> // For std::min, std::max, std::sort, std::replace
#include <cmath>     // For std::max with floats, std::round
#include <filesystem> // Requires C++17
#include <fstream>
#include <sstream>

// Define namespace alias for easier use
namespace fs = std::filesystem;

// --- Constructor & Destructor ---

MenuScene::MenuScene() {}

MenuScene::~MenuScene() {
    clean();
}

// --- Initialization & Cleanup ---

void MenuScene::init() {
    clean(); // Clean up any previous state first
    // std::cout << "Initializing Menu Scene..." << std::endl; // Removed log
    isEditingName = false;
    playerName = "";

    // Load sounds
    startSound = Mix_LoadWAV("assets/sound/start.wav");
    clickSound = Mix_LoadWAV("assets/sound/buttonclick.wav");
    if (!startSound) { std::cerr << "Failed to load start.wav: " << Mix_GetError() << std::endl; }
    if (!clickSound) { std::cerr << "Failed to load buttonclick.wav: " << Mix_GetError() << std::endl; }

    // Load Textures
    backgroundTexture = loadTexture("assets/menu/bg.png");
    titleTexture = loadTexture("assets/menu/title.png");
    playButtonTexture = loadTexture("assets/menu/play.png");
    nameBoxTexture = loadTexture("assets/menu/box.png");
    soundOnTexture = loadTexture("assets/menu/soundon.png");
    soundOffTexture = loadTexture("assets/menu/soundoff.png");
    sliderTrackTexture = loadTexture("assets/menu/slidebar.png");
    slideButtonTexture = loadTexture("assets/menu/slidebutton.png");  // Load texture for scrollbar button
    sliderButtonTexture = loadTexture("assets/menu/slidebutton.png"); // Load the same texture for volume slider buttons

    // Load Fonts
    inputFont = TTF_OpenFont("assets/font.ttf", 20);
    saveSlotFont = TTF_OpenFont("assets/font.ttf", 18);
    uiHintFont = TTF_OpenFont("assets/font.ttf", 12);

    if (!inputFont) { std::cerr << "Failed to load input font!" << std::endl; }
    if (!saveSlotFont) { std::cerr << "Failed to load save slot font!" << std::endl; }
    if (!uiHintFont) {
        std::cerr << "Failed to load hint font! Using save slot font as fallback." << std::endl;
        if(saveSlotFont) uiHintFont = saveSlotFont;
        else if(inputFont) uiHintFont = inputFont;
        else std::cerr << "FATAL: No valid font loaded for hints!" << std::endl;
    }

    // Initialize Volume State from Game static members
    isMusicMuted = (Game::getMusicVolume() == 0);
    storedMusicVolumeBeforeMute = isMusicMuted ? (MIX_MAX_VOLUME / 2) : Game::getMusicVolume();
    isSfxMuted = (Game::getSfxVolume() == 0);
    storedSfxVolumeBeforeMute = isSfxMuted ? (MIX_MAX_VOLUME / 2) : Game::getSfxVolume();
    isDraggingBgmSlider = false;
    isDraggingSfxSlider = false;

    // Load Save Files
    loadSaveFiles(); // This might reset selectedSaveSlotIndex

    // Update Name Texture (Placeholder or loaded name)
    updateNameTexture(); // Needs to happen after loadSaveFiles potentially sets playerName

    // Pre-render Fullscreen Hint Text Texture
    if (uiHintFont && Game::renderer) {
        SDL_Color hintColor = { 200, 200, 200, 255 };
        SDL_Surface* surface = TTF_RenderText_Blended(uiHintFont, "F11: Fullscreen", hintColor);
        if(surface) {
            if (fullscreenTextTex) { SDL_DestroyTexture(fullscreenTextTex); } // Clean previous if any
            fullscreenTextTex = SDL_CreateTextureFromSurface(Game::renderer, surface);
            SDL_FreeSurface(surface);
            if (!fullscreenTextTex) { std::cerr << "Failed to create fullscreen hint texture! SDL Error: " << SDL_GetError() << std::endl; }
        } else { std::cerr << "Failed to render fullscreen hint surface! TTF Error: " << TTF_GetError() << std::endl; }
    } else {
        if(!uiHintFont) std::cerr << "Cannot render hint text: uiHintFont is null." << std::endl;
        if(!Game::renderer) std::cerr << "Cannot render hint text: Game::renderer is null." << std::endl;
    }

    // Start Menu BGM
    Mix_HaltMusic();
    if (menuMusic) { Mix_FreeMusic(menuMusic); menuMusic = nullptr; }
    menuMusic = Mix_LoadMUS("assets/sound/menubgm.mp3");
    if (menuMusic) {
         if (Mix_PlayMusic(menuMusic, -1) == -1) { std::cerr << "Failed to play menu BGM: " << Mix_GetError() << std::endl; }
         else { Mix_VolumeMusic(Game::getMusicVolume()); } // Apply current volume
    } else { std::cerr << "Failed to load menu BGM!" << std::endl; }

    // Calculate Initial Layout
    int w = 800, h = 600; // Default size
    if (Game::renderer) { SDL_GetRendererOutputSize(Game::renderer, &w, &h); }
    else { std::cerr << "Renderer not available during MenuScene init for layout calc!" << std::endl; }
    calculateLayout(w, h);

    // std::cout << "Menu Scene Initialized." << std::endl; // Removed log
}

void MenuScene::clean() {
    // Destroy Textures
    if (backgroundTexture) { SDL_DestroyTexture(backgroundTexture); backgroundTexture = nullptr; }
    if (titleTexture) { SDL_DestroyTexture(titleTexture); titleTexture = nullptr; }
    if (playButtonTexture) { SDL_DestroyTexture(playButtonTexture); playButtonTexture = nullptr; }
    if (nameBoxTexture) { SDL_DestroyTexture(nameBoxTexture); nameBoxTexture = nullptr; }
    if (nameTextTexture) { SDL_DestroyTexture(nameTextTexture); nameTextTexture = nullptr; }
    if (slideButtonTexture) { SDL_DestroyTexture(slideButtonTexture); slideButtonTexture = nullptr; }
    if (sliderButtonTexture) { SDL_DestroyTexture(sliderButtonTexture); sliderButtonTexture = nullptr; } // Clean the second button texture
    if (soundOnTexture) { SDL_DestroyTexture(soundOnTexture); soundOnTexture = nullptr; }
    if (soundOffTexture) { SDL_DestroyTexture(soundOffTexture); soundOffTexture = nullptr; }
    if (sliderTrackTexture) { SDL_DestroyTexture(sliderTrackTexture); sliderTrackTexture = nullptr; }
    if (fullscreenTextTex) { SDL_DestroyTexture(fullscreenTextTex); fullscreenTextTex = nullptr; }

    // Close Fonts
    if (inputFont) { TTF_CloseFont(inputFont); inputFont = nullptr; }
    if (saveSlotFont) { TTF_CloseFont(saveSlotFont); saveSlotFont = nullptr; }
    // Close uiHintFont ONLY if it's different and was loaded
    if (uiHintFont && uiHintFont != saveSlotFont && uiHintFont != inputFont) { TTF_CloseFont(uiHintFont); }
    uiHintFont = nullptr;

    // Stop and Free Audio
    Mix_HaltMusic();
    if (menuMusic) { Mix_FreeMusic(menuMusic); menuMusic = nullptr; }
    if (startSound) { Mix_FreeChunk(startSound); startSound = nullptr; }
    if (clickSound) { Mix_FreeChunk(clickSound); clickSound = nullptr; }

    // std::cout << "Menu Scene Cleaned." << std::endl; // Removed log
}

// --- Helper Functions ---

SDL_Texture* MenuScene::loadTexture(const std::string& path) {
     SDL_Texture* newTexture = TextureManager::LoadTexture(path.c_str());
     if (newTexture == nullptr) {
         // Error logged in TextureManager::LoadTexture
         // std::cerr << "Failed to load texture: " << path << std::endl; // Keep TextureManager's log
     }
     return newTexture;
}

void MenuScene::updateNameTexture() {
    if (nameTextTexture) {
        SDL_DestroyTexture(nameTextTexture);
        nameTextTexture = nullptr;
    }
    originalNameTextW = 0; // Reset original dimensions
    originalNameTextH = 0;

    if (!inputFont || !Game::renderer) {
         std::cerr << "Cannot update name texture: Font or Renderer is null!" << std::endl;
         return;
    }

    std::string textToRender = playerName;
    SDL_Color colorToUse = inputTextColor;

    // Determine text and color for placeholder display
    if (selectedSaveSlotIndex == 0 && !isEditingName && playerName.empty()) {
        textToRender = "Enter Name (Optional)";
        colorToUse = placeholderTextColor;
    } else if (playerName.empty() && isEditingName) {
         // If editing and empty, don't render texture, just let cursor show
         int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
         calculateLayout(w, h); // Recalculate for cursor position
         return;
    } else if (playerName.empty()) {
        // If not editing, not placeholder, and empty (e.g., loaded empty name), don't render
        return;
    }


    SDL_Surface* textSurface = TTF_RenderText_Blended(inputFont, textToRender.c_str(), colorToUse);
    if (!textSurface) {
        std::cerr << "Unable to render name text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    nameTextTexture = SDL_CreateTextureFromSurface(Game::renderer, textSurface);
    if (!nameTextTexture) {
        std::cerr << "Unable to create name text texture! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        originalNameTextW = textSurface->w; // Store original dimensions
        originalNameTextH = textSurface->h;
    }
    SDL_FreeSurface(textSurface);

    // Recalculate layout after updating texture size
    int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
    calculateLayout(w,h);
}

std::string MenuScene::parseTimestampFromFilename(const std::string& filename) {
    size_t last_slash_idx = filename.find_last_of("\\/");
    std::string name_only = (last_slash_idx == std::string::npos) ? filename : filename.substr(last_slash_idx + 1);
    size_t dot_idx = name_only.rfind(".state");
    if (dot_idx == std::string::npos) return "Invalid Save"; // Handle case with no extension

    std::string timestamp_str = name_only.substr(0, dot_idx);
    // Check for expected timestamp format DDMMYYYY-HHMMSS
    if (timestamp_str.length() == 15 && timestamp_str[8] == '-' && timestamp_str.find_first_not_of("0123456789-") == std::string::npos) {
        // Basic format check passed, format it nicely
        return timestamp_str.substr(0, 2) + "/" + timestamp_str.substr(2, 2) + "/" + timestamp_str.substr(4, 4) +
               " " + timestamp_str.substr(9, 2) + ":" + timestamp_str.substr(11, 2); // Removed seconds
    }
    // If not the expected format, return the raw stem or a placeholder
    return timestamp_str.empty() ? "Invalid Save" : timestamp_str;
}

int MenuScene::parseLevelFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) return 1; // Default level 1 if file cannot be opened
    std::string line, key, value;
    int level = 1; // Default level
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerLevel") {
            try { level = std::stoi(value); }
            catch (...) { level = 1; } // Reset to 1 on conversion error
            break; // Found level, no need to read further
        }
    }
    saveFile.close();
    return std::max(1, level); // Ensure level is at least 1
}

std::string MenuScene::parseNameFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) {
        return "Player"; // Default if file cannot be opened
    }
    std::string line, key, value;
    std::string name = "Player"; // Default name
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerName") {
            name = value;
            if (name.empty()) { name = "Player"; } // Use default if loaded name is empty
            break; // Found name, exit loop
        }
    }
    saveFile.close();
    return name;
}

void MenuScene::loadSaveFiles() {
    // std::cout << "Loading save files..." << std::endl; // Removed log
    saveSlots.clear();
    saveSlots.push_back(SaveSlotInfo{ "", 0, "Player", "", true }); // Add "+ New Game" slot

    std::string saveDir = "saves";
    std::vector<fs::path> stateFiles;

    try {
        if (!fs::exists(saveDir)) { // Create directory if it doesn't exist
            // std::cout << "'saves' directory not found. Creating..." << std::endl; // Removed log
            if (!fs::create_directory(saveDir)) {
                 std::cerr << "Failed to create 'saves' directory!" << std::endl;
            }
        }

        if (fs::is_directory(saveDir)) { // Proceed only if it's a directory
            for (const auto& entry : fs::directory_iterator(saveDir)) {
                std::string filename_str = entry.path().filename().string();
                // Skip specific system files
                if (filename_str == "default.state" || filename_str == "quicksave.state") {
                    continue;
                }
                if (entry.is_regular_file() && entry.path().extension() == ".state") {
                    stateFiles.push_back(entry.path());
                }
            }

            // Sort by last write time (latest first)
            std::sort(stateFiles.begin(), stateFiles.end(), [](const fs::path& a, const fs::path& b) {
                try {
                    if (!fs::exists(a) || !fs::exists(b)) return false; // Handle deleted files
                    return fs::last_write_time(a) > fs::last_write_time(b);
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error comparing file times: " << e.what() << std::endl;
                    return false;
                }
            });

            // Process sorted files
            for (const auto& path : stateFiles) {
                std::string filename = path.filename().string();
                std::string fullpath = path.string();
                std::replace(fullpath.begin(), fullpath.end(), '\\', '/'); // Ensure consistent separators

                std::string timestamp = parseTimestampFromFilename(filename);
                int level = parseLevelFromSaveFile(fullpath);
                std::string name = parseNameFromSaveFile(fullpath);

                saveSlots.push_back(SaveSlotInfo{ timestamp, level, name, fullpath, false });
            }
        } else {
            std::cerr << "'saves' exists but is not a directory." << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error while reading saves: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading save files: " << e.what() << std::endl;
    }

    // Reset scroll and selection
    scrollOffset = 0;
    selectedSaveSlotIndex = 0; // Default to "+ New Game"
    // std::cout << "Found " << (saveSlots.size() > 0 ? saveSlots.size() - 1 : 0) << " valid save files." << std::endl; // Removed log
}

// --- Scene Methods ---

void MenuScene::handleEvents(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mousePoint = {mouseX, mouseY};

    // Window Resize Handling
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
         updateNameTexture(); // Update name texture first in case font size needs recalculating
         int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
         calculateLayout(w, h); // Recalculate full layout
    }
    // Mouse Wheel Scroll for Save List
    else if (event.type == SDL_MOUSEWHEEL) {
        bool canScroll = saveSlots.size() > visibleSlotsCount;
        SDL_Rect scrollArea = saveListAreaRect; // Base scroll area is the list itself
        if (scrollbarTrackRect.w > 0) { // Expand scroll area to include track if visible
             scrollArea.w += scrollbarTrackRect.w + (scrollbarTrackRect.x - (saveListAreaRect.x + saveListAreaRect.w));
             scrollArea.h = std::max(scrollArea.h, scrollbarTrackRect.h);
        }

        if (canScroll && SDL_PointInRect(&mousePoint, &scrollArea)) {
            int oldOffset = scrollOffset;
            if (event.wheel.y > 0) scrollOffset--; // Scroll Up
            else if (event.wheel.y < 0) scrollOffset++; // Scroll Down

            int maxScrollOffset = std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
            scrollOffset = std::max(0, std::min(scrollOffset, maxScrollOffset)); // Clamp

            if(oldOffset != scrollOffset) { // Only recalculate layout if offset changed
                 int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
                 calculateLayout(w, h); // Update scrollbar button position
            }
        }
    }
    // Mouse Button Down Logic
     else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
         bool playClickSoundFlag = false; // Flag to play generic click sound

         // Check UI Element Clicks (Order matters for overlapping elements)
         if (SDL_PointInRect(&mousePoint, &playButtonRect)) {
             // Play Button Action
             if (isEditingName) { isEditingName = false; SDL_StopTextInput(); updateNameTexture(); } // Stop editing if active
             if(startSound) Mix_PlayChannel(-1, startSound, 0); // Play specific start sound
             playClickSoundFlag = false; // Don't play generic click

             if (selectedSaveSlotIndex >= 0 && selectedSaveSlotIndex < saveSlots.size()) {
                 const auto& selectedSlot = saveSlots[selectedSaveSlotIndex];
                 Scene* gameScenePtr = SceneManager::instance->getScene(SceneType::Game);
                 GameScene* gameScene = dynamic_cast<GameScene*>(gameScenePtr);

                 if (!gameScene) { std::cerr << "Error: Could not get/cast GameScene!" << std::endl; return; }

                 gameScene->resetGame(); // Resets/Cleans the existing game instance
                 gameScene->init();    // Creates/Initializes a new game instance

                 if (!Game::instance) { std::cerr << "Error: Game::instance is null after GameScene init!" << std::endl; return; }

                 if (selectedSlot.isNewGameOption) {
                      Game::instance->setPlayerName(this->playerName);
                 } else {
                      if (Game::instance->saveLoadManager) {
                           if (!Game::instance->saveLoadManager->loadGameState(selectedSlot.filename)) {
                                std::cerr << "Failed to load game state: " << selectedSlot.filename << std::endl; return; // Don't switch scene if load failed
                           }
                           this->playerName = Game::instance->getPlayerName(); // Update display name
                           updateNameTexture();
                      } else { std::cerr << "Critical Error: SaveLoadManager not available!" << std::endl; return; }
                 }
                 SceneManager::instance->switchToScene(SceneType::Game); // Switch after successful setup/load
             } else { std::cerr << "Error: Invalid selected save slot index." << std::endl; }
             return; // Play button action handled

         } else if (SDL_PointInRect(&mousePoint, &bgmIconRect)) {
             // Mute BGM Icon
             bool wasMuted = (Game::getMusicVolume() == 0);
             if (!wasMuted) { storedMusicVolumeBeforeMute = Game::getMusicVolume(); Game::setMusicVolume(0); }
             else { Game::setMusicVolume(storedMusicVolumeBeforeMute > 0 ? storedMusicVolumeBeforeMute : MIX_MAX_VOLUME / 4); storedMusicVolumeBeforeMute = Game::getMusicVolume(); }
             isMusicMuted = (Game::getMusicVolume() == 0);
             if(menuMusic) Mix_VolumeMusic(Game::getMusicVolume());
             playClickSoundFlag = true;

         } else if (SDL_PointInRect(&mousePoint, &sfxIconRect)) {
             // Mute SFX Icon
             bool wasMuted = (Game::getSfxVolume() == 0);
             if (!wasMuted) { storedSfxVolumeBeforeMute = Game::getSfxVolume(); Game::setSfxVolume(0); }
             else { Game::setSfxVolume(storedSfxVolumeBeforeMute > 0 ? storedSfxVolumeBeforeMute : MIX_MAX_VOLUME / 4); storedSfxVolumeBeforeMute = Game::getSfxVolume(); }
             isSfxMuted = (Game::getSfxVolume() == 0);
             playClickSoundFlag = true;

         } else if (SDL_PointInRect(&mousePoint, &bgmSliderButtonRect)) {
             // Start Drag BGM Slider
             isDraggingBgmSlider = true; sliderDragStartX = mouseX - bgmSliderButtonRect.x; playClickSoundFlag = true;
         } else if (SDL_PointInRect(&mousePoint, &sfxSliderButtonRect)) {
             // Start Drag SFX Slider
             isDraggingSfxSlider = true; sliderDragStartX = mouseX - sfxSliderButtonRect.x; playClickSoundFlag = true;
         } else if (saveSlots.size() > visibleSlotsCount && SDL_PointInRect(&mousePoint, &scrollbarButtonRect)) {
             // Start Drag Scrollbar
              if (!isDraggingScrollbar) { isDraggingScrollbar = true; scrollbarDragStartY = mouseY - scrollbarButtonRect.y; playClickSoundFlag = true; }
         } else if (SDL_PointInRect(&mousePoint, &nameInputBoxRect)) {
             // Activate Name Input (Only for New Game slot)
              if (!isEditingName && selectedSaveSlotIndex == 0) {
                  isEditingName = true; SDL_StartTextInput(); updateNameTexture(); playClickSoundFlag = true;
              }
         } else {
             // Check Save Slot Clicks
             bool clickedOnSlot = false;
             for (int i = 0; i < visibleSlotsCount; ++i) {
                 int actualIndex = scrollOffset + i;
                 if (actualIndex >= 0 && actualIndex < saveSlots.size()) {
                     if (SDL_PointInRect(&mousePoint, &visibleSaveSlotRects[i])) {
                         if (selectedSaveSlotIndex != actualIndex) {
                             selectedSaveSlotIndex = actualIndex;
                             if (saveSlots[actualIndex].isNewGameOption) {
                                 playerName = "";
                                 if (!isEditingName) { isEditingName = true; SDL_StartTextInput(); }
                             } else {
                                 if (isEditingName) { isEditingName = false; SDL_StopTextInput(); }
                                 playerName = saveSlots[actualIndex].playerName;
                             }
                             updateNameTexture();
                         }
                         playClickSoundFlag = true; clickedOnSlot = true; break;
                     }
                 }
             }

             // Click Outside Interactive Elements
             if (!clickedOnSlot && isEditingName) {
                  isEditingName = false; SDL_StopTextInput(); updateNameTexture();
             }
         }

         // Play generic click sound if flagged
         if (playClickSoundFlag && clickSound) { Mix_PlayChannel(-1, clickSound, 0); }

         // Recalculate layout if mute state or slider drag might have changed positions
         if (playClickSoundFlag) { // Check if layout needs recalculation
              int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
              calculateLayout(w, h);
         }
     } // End Mouse Button Down Event

     // --- Mouse Button Up ---
     else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
          if (isDraggingScrollbar || isDraggingBgmSlider || isDraggingSfxSlider) {
               isDraggingScrollbar = false; isDraggingBgmSlider = false; isDraggingSfxSlider = false;
               // std::cout << "Slider/Scrollbar drag ended." << std::endl; // Removed log
          }
     }
     // --- Mouse Motion (Dragging) ---
     else if (event.type == SDL_MOUSEMOTION) {
          int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
          if (isDraggingBgmSlider) {
                int trackX = bgmSliderTrackRect.x; int trackButtonW = bgmSliderButtonRect.w; int trackW = std::max(1, bgmSliderTrackRect.w - trackButtonW);
                int targetButtonX = std::max(trackX, std::min(mouseX - sliderDragStartX, trackX + trackW));
                float percent = (trackW > 0) ? static_cast<float>(targetButtonX - trackX) / trackW : 0.0f;
                int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
                if (newVolume != Game::getMusicVolume()) {
                     Game::setMusicVolume(newVolume); isMusicMuted = (newVolume == 0);
                     if(!isMusicMuted) storedMusicVolumeBeforeMute = newVolume;
                     if(menuMusic) Mix_VolumeMusic(newVolume); calculateLayout(w, h);
                }
          } else if (isDraggingSfxSlider) {
                int trackX = sfxSliderTrackRect.x; int trackButtonW = sfxSliderButtonRect.w; int trackW = std::max(1, sfxSliderTrackRect.w - trackButtonW);
                int targetButtonX = std::max(trackX, std::min(mouseX - sliderDragStartX, trackX + trackW));
                float percent = (trackW > 0) ? static_cast<float>(targetButtonX - trackX) / trackW : 0.0f;
                int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
                if (newVolume != Game::getSfxVolume()) {
                     Game::setSfxVolume(newVolume); isSfxMuted = (newVolume == 0);
                     if(!isSfxMuted) storedSfxVolumeBeforeMute = newVolume; calculateLayout(w, h);
                }
          } else if (isDraggingScrollbar) {
              int scrollTrackHeight = std::max(1, scrollbarTrackRect.h - scrollbarButtonRect.h);
              int targetButtonTop = std::max(scrollbarTrackRect.y, std::min(mouseY - scrollbarDragStartY, scrollbarTrackRect.y + scrollTrackHeight));
              float percentageScrolled = (scrollTrackHeight > 0) ? static_cast<float>(targetButtonTop - scrollbarTrackRect.y) / scrollTrackHeight : 0.0f;
              int maxScrollOffset = std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
              int newOffset = static_cast<int>(std::round(percentageScrolled * maxScrollOffset));
              if (newOffset != scrollOffset) { scrollOffset = newOffset; calculateLayout(w, h); }
          }
     }
     // --- Text Input ---
     else if (event.type == SDL_TEXTINPUT && isEditingName) {
         const int MAX_NAME_LENGTH = 15;
         if (playerName.length() < MAX_NAME_LENGTH) {
              playerName += event.text.text;
              updateNameTexture(); // Update display
         }
     }
     // --- Key Down ---
     else if (event.type == SDL_KEYDOWN) {
         if (isEditingName) {
             if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
                 playerName.pop_back(); updateNameTexture();
             } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                  isEditingName = false; SDL_StopTextInput(); updateNameTexture();
             } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                  isEditingName = false; SDL_StopTextInput(); playerName = ""; updateNameTexture(); // Clear name on Escape
             }
         } else if (event.key.keysym.sym == SDLK_ESCAPE) { // Global escape quits
             // std::cout << "Exit requested from Menu" << std::endl; // Removed log
             SDL_Event quitEvent; quitEvent.type = SDL_QUIT; SDL_PushEvent(&quitEvent);
         }
     } // End Key Down Event
} // End handleEvents

void MenuScene::update() {
    // Nothing typically needs continuous updating here
}

void MenuScene::render() {
    if (!Game::renderer) { return; }
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255); // Black background fallback
    SDL_RenderClear(Game::renderer);

    // Render Background & Center Column
    if (backgroundTexture) SDL_RenderCopy(Game::renderer, backgroundTexture, NULL, &backgroundRect);
    if (titleTexture) SDL_RenderCopy(Game::renderer, titleTexture, NULL, &titleRect);
    if (playButtonTexture) SDL_RenderCopy(Game::renderer, playButtonTexture, NULL, &playButtonRect);
    if (nameBoxTexture) SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &nameInputBoxRect);
    if (nameTextTexture) SDL_RenderCopy(Game::renderer, nameTextTexture, NULL, &nameTextRect);

    // Render Save List
    int slotsToDraw = std::min(visibleSlotsCount, static_cast<int>(saveSlots.size()) - scrollOffset);
    for (int i = 0; i < slotsToDraw; ++i) {
        int actualIndex = scrollOffset + i;
        if (actualIndex < 0 || actualIndex >= saveSlots.size()) continue;

        const auto& currentSlot = saveSlots[actualIndex];
        SDL_Rect currentSlotRect = visibleSaveSlotRects[i];

        // Draw box background (with selection tint)
        if (nameBoxTexture) {
            SDL_Color tint = (actualIndex == selectedSaveSlotIndex) ? SDL_Color{200, 200, 255, 255} : SDL_Color{255, 255, 255, 255};
            SDL_SetTextureColorMod(nameBoxTexture, tint.r, tint.g, tint.b);
            SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &currentSlotRect);
            SDL_SetTextureColorMod(nameBoxTexture, 255, 255, 255); // Reset tint
        }

        // Render text directly within the box
        if (saveSlotFont) {
            std::string textStr = currentSlot.isNewGameOption ? "+ New Game" :
                                  (actualIndex == selectedSaveSlotIndex ? (currentSlot.playerName + " Lvl " + std::to_string(currentSlot.level)) : currentSlot.timestamp);
            SDL_Color colorToUse = (actualIndex == selectedSaveSlotIndex) ? saveSlotTextColorSelected : saveSlotTextColorDefault;

            SDL_Surface* surface = TTF_RenderText_Blended(saveSlotFont, textStr.c_str(), colorToUse);
            if (surface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game::renderer, surface);
                if (textTexture) {
                     int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
                     float scaleX = static_cast<float>(w) / referenceWidth; float scaleY = static_cast<float>(h) / referenceHeight;
                     float scaleFactor = std::max(std::min(scaleX, scaleY), 0.8f);
                     float saveSlotTextScale = std::max(scaleFactor * 0.9f, 0.72f); // Consistent scale calc

                    SDL_Rect textDestRect;
                    textDestRect.w = static_cast<int>(surface->w * saveSlotTextScale);
                    textDestRect.h = static_cast<int>(surface->h * saveSlotTextScale);
                    textDestRect.x = currentSlotRect.x + (currentSlotRect.w - textDestRect.w) / 2;
                    textDestRect.y = currentSlotRect.y + (currentSlotRect.h - textDestRect.h) / 2;

                    SDL_RenderCopy(Game::renderer, textTexture, NULL, &textDestRect);
                    SDL_DestroyTexture(textTexture);
                } else { std::cerr << "Failed to create texture for save slot text: " << textStr << std::endl; }
                SDL_FreeSurface(surface);
            } else { std::cerr << "Failed to render text surface for save slot: " << textStr << std::endl; }
        }
    } // End loop through visible slots

    // Render Scrollbar (if needed)
    bool needsScrollbar = saveSlots.size() > visibleSlotsCount;
    if (needsScrollbar) {
        // Draw Track Background
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Game::renderer, 80, 80, 80, 150);
        SDL_RenderFillRect(Game::renderer, &scrollbarTrackRect);
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);

        // Render Button (using slideButtonTexture for scrollbar)
        if (slideButtonTexture) {
             SDL_Color tint = isDraggingScrollbar ? SDL_Color{200, 200, 200, 255} : SDL_Color{255, 255, 255, 255};
             SDL_SetTextureColorMod(slideButtonTexture, tint.r, tint.g, tint.b);
             SDL_RenderCopy(Game::renderer, slideButtonTexture, NULL, &scrollbarButtonRect);
             SDL_SetTextureColorMod(slideButtonTexture, 255, 255, 255); // Reset tint
        }
    }

    // Render Cursor in Name Box (if editing "+ New Game")
    if (isEditingName && selectedSaveSlotIndex == 0) {
        Uint32 ticks = SDL_GetTicks();
        if (ticks % 1000 < 500) { // Blink cursor
            SDL_SetRenderDrawColor(Game::renderer, inputTextColor.r, inputTextColor.g, inputTextColor.b, 255);

            // Calculate cursor dimensions based on scale
             int w = 800, h = 600; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
             float scaleX = static_cast<float>(w) / referenceWidth; float scaleY = static_cast<float>(h) / referenceHeight;
             float scaleFactor = std::max(std::min(scaleX, scaleY), 0.8f);
             float textScale = std::max(scaleFactor * 0.8f, 0.64f); // Consistent scale

            int cursorW = std::max(1, static_cast<int>(2.0f * textScale)); // Scaled cursor width
            int cursorPadding = std::max(1, static_cast<int>(2.0f * textScale));
            int cursorH = nameTextRect.h > 0 ? nameTextRect.h : std::max(1, nameInputBoxRect.h - 2 * cursorPadding); // Use text height or box height

            int cursorX;
            if (nameTextRect.w > 0) { cursorX = nameTextRect.x + nameTextRect.w + cursorPadding; } // After text
            else { cursorX = nameInputBoxRect.x + cursorPadding * 2; } // At start of box if no text

            int cursorY = (nameTextRect.h > 0) ? nameTextRect.y : (nameInputBoxRect.y + (nameInputBoxRect.h - cursorH) / 2); // Align with text or center in box

            cursorX = std::min(cursorX, nameInputBoxRect.x + nameInputBoxRect.w - cursorW - cursorPadding); // Clamp X
            cursorX = std::max(nameInputBoxRect.x + cursorPadding, cursorX);

            SDL_Rect cursorRect = {cursorX, cursorY, cursorW, cursorH};
            SDL_RenderFillRect(Game::renderer, &cursorRect);
        }
    }

    // Render Volume Controls
    SDL_Texture* bgmIconTex = isMusicMuted ? soundOffTexture : soundOnTexture;
    SDL_Texture* sfxIconTex = isSfxMuted ? soundOffTexture : soundOnTexture;

    if (bgmIconTex) SDL_RenderCopy(Game::renderer, bgmIconTex, NULL, &bgmIconRect);
    if (sfxIconTex) SDL_RenderCopy(Game::renderer, sfxIconTex, NULL, &sfxIconRect);
    if (sliderTrackTexture) {
        SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &bgmSliderTrackRect);
        SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &sfxSliderTrackRect);
    }
    // Render Volume Slider Buttons (using sliderButtonTexture)
    if (sliderButtonTexture) {
        SDL_Color bgmTint = isDraggingBgmSlider ? SDL_Color{200, 200, 200, 255} : SDL_Color{255, 255, 255, 255};
        SDL_SetTextureColorMod(sliderButtonTexture, bgmTint.r, bgmTint.g, bgmTint.b);
        SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &bgmSliderButtonRect);

        SDL_Color sfxTint = isDraggingSfxSlider ? SDL_Color{200, 200, 200, 255} : SDL_Color{255, 255, 255, 255};
        SDL_SetTextureColorMod(sliderButtonTexture, sfxTint.r, sfxTint.g, sfxTint.b);
        SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &sfxSliderButtonRect);

        SDL_SetTextureColorMod(sliderButtonTexture, 255, 255, 255); // Reset tint
    }

    // Render Fullscreen Hint
    if (fullscreenTextTex) {
        SDL_RenderCopy(Game::renderer, fullscreenTextTex, NULL, &fullscreenTextRect);
    }

    SDL_RenderPresent(Game::renderer);
}

// --- Layout Calculation ---
void MenuScene::calculateLayout(int windowWidth, int windowHeight) {
    // --- Background ---
    backgroundRect = { 0, 0, windowWidth, windowHeight };

    // --- Scaling Factor (True Scale, No Minimum Clamp) ---
    float scaleX = static_cast<float>(windowWidth) / static_cast<float>(referenceWidth);
    float scaleY = static_cast<float>(windowHeight) / static_cast<float>(referenceHeight);
    // Use the smaller scale factor to ensure elements fit without distortion
    float scaleFactor = std::min(scaleX, scaleY) + 0.5f;

    // --- Effective Area & Offsets ---
    // Calculate effective dimensions based on scale factor
    int effectiveWidth = static_cast<int>(referenceWidth * scaleFactor);
    int effectiveHeight = static_cast<int>(referenceHeight * scaleFactor);
    // Calculate offsets to center the effective area within the actual window
    int offsetX = (windowWidth - effectiveWidth) / 2; 
    int offsetY = (windowHeight - effectiveHeight) / 2;

    // --- Scaled Padding ---
    // Base padding on window size or use scaled fixed values
    int paddingX = std::max(5, static_cast<int>(windowWidth * 0.02f)); // ~2% width padding, min 5px
    int paddingY = std::max(5, static_cast<int>(windowHeight * 0.02f)); // ~2% height padding, min 5px

    // --- Text Scaling ---
    // Scale text relative to overall scale factor, maybe clamp minimum font size later if needed
    float textScale = scaleFactor * 0.9f; // Adjust base multiplier as needed
    float saveSlotTextScale = scaleFactor * 1.0f; // Make save slot text slightly larger relative to scale


    // --- Center Column Elements (Position relative to windowWidth/Height) ---
    // Title
    titleRect.w = static_cast<int>(originalTitleWidth * scaleFactor);
    titleRect.h = static_cast<int>(originalTitleHeight * scaleFactor);
    // Ensure minimum size? e.g. titleRect.w = std::max(100, titleRect.w);
    titleRect.x = (windowWidth - titleRect.w) / 2; // Center in actual window
    titleRect.y = paddingY * 2; // Position near top with padding

    // Play Button
    playButtonRect.w = static_cast<int>(originalPlayButtonWidth * scaleFactor);
    playButtonRect.h = static_cast<int>(originalPlayButtonHeight * scaleFactor);
    playButtonRect.x = (windowWidth - playButtonRect.w) / 2; // Center in actual window
    playButtonRect.y = titleRect.y + titleRect.h + paddingY * 3; // Position below title

    // Name Input Box
    nameInputBoxRect.w = static_cast<int>(originalNameBoxWidth * scaleFactor * 1.5f); // Keep slightly larger ratio
    nameInputBoxRect.h = static_cast<int>(originalNameBoxHeight * scaleFactor * 1.2f);
    nameInputBoxRect.x = (windowWidth - nameInputBoxRect.w) / 2; // Center in actual window
    nameInputBoxRect.y = playButtonRect.y + playButtonRect.h + paddingY * 3; // Position below play button

    // Name Text Position (Centered within Name Input Box)
    int currentTextW = originalNameTextW;
    int currentTextH = originalNameTextH;
    int scaledTextW = static_cast<int>(currentTextW * textScale);
    int scaledTextH = static_cast<int>(currentTextH * textScale);
    // Ensure text fits in box
    scaledTextW = std::min(scaledTextW, nameInputBoxRect.w - paddingX / 2);
    scaledTextH = std::min(scaledTextH, nameInputBoxRect.h - paddingY / 2);
    nameTextRect.x = nameInputBoxRect.x + (nameInputBoxRect.w - scaledTextW) / 2;
    nameTextRect.y = nameInputBoxRect.y + (nameInputBoxRect.h - scaledTextH) / 2;
    nameTextRect.w = scaledTextW;
    nameTextRect.h = scaledTextH;


    // --- Left Column: Save List (Position relative to windowWidth/Height & Name Box) ---
    int listWidth = static_cast<int>(windowWidth * 0.2f); // % of actual window width
    int listHeight = static_cast<int>(windowHeight * 0.2f); // % of actual window height

    // Center list vertically
    int listY = (windowHeight - listHeight) / 2;

    // Center list horizontally between left edge (0) and name box left edge
    int targetListCenterX = (nameInputBoxRect.x) / 2 + 80 * (scaleFactor - 0.5f); // Midpoint calculation simplified
    int listX = targetListCenterX - listWidth / 2;
    // Ensure minimum padding from left edge
    listX = std::max(paddingX, listX);
    // Ensure list doesn't overlap name box (important at small widths)
    listWidth = std::min(listWidth, nameInputBoxRect.x - listX - paddingX);
    listWidth = std::max(50, listWidth); // Ensure minimum width

    saveListAreaRect = { listX, listY, listWidth, listHeight };

    // Position Save Slots
    visibleSaveSlotRects.resize(visibleSlotsCount);
    float slotHeight = (visibleSlotsCount > 0) ? static_cast<float>(listHeight) / visibleSlotsCount : 0;
    float slotPadding = slotHeight * 0.1f;
    float actualSlotHeight = std::max(0.0f, slotHeight - slotPadding);

    for (int i = 0; i < visibleSlotsCount; ++i) {
        int scaledBoxW = static_cast<int>((originalNameBoxWidth * (scaleFactor - 0.5f)) * 0.9f);
        int scaledBoxH = static_cast<int>((originalNameBoxHeight * (scaleFactor - 0.5f)) * 1.0f);
        // Ensure box fits list width & slot height
        scaledBoxW = std::min(scaledBoxW, listWidth - paddingX / 2);
        scaledBoxH = std::min(scaledBoxH, static_cast<int>(actualSlotHeight));

        visibleSaveSlotRects[i].x = listX + (listWidth - scaledBoxW) / 2; // Center horizontally in list area
        visibleSaveSlotRects[i].y = listY + static_cast<int>(i * slotHeight + (slotHeight - scaledBoxH)/2.0f); // Center vertically in slot space
        visibleSaveSlotRects[i].w = scaledBoxW * 1.8f;
        visibleSaveSlotRects[i].h = scaledBoxH * 2.4f;
        // Text positioning happens dynamically in render()
    }

    // --- Scrollbar Button & Track Area (Position relative to list) ---
    if (saveSlots.size() > visibleSlotsCount) {
        // Scale scrollbar elements based on actual scaleFactor
        int scrollbarWidth = std::max(5, static_cast<int>(originalSlideButtonWidth * scaleFactor * 1.5f)); // Track width based on button
        int scrollbarX = listX + listWidth + paddingX / 2; // Position next to list area
        // Ensure scrollbar doesn't overlap name box
        scrollbarX = std::min(scrollbarX, nameInputBoxRect.x - scrollbarWidth - paddingX / 2);

        scrollbarTrackRect.x = scrollbarX;
        scrollbarTrackRect.y = listY; // Align top with list top
        scrollbarTrackRect.w = scrollbarWidth;
        scrollbarTrackRect.h = listHeight; // Match list height

        int scrollButtonWidth = std::max(3, static_cast<int>(originalSlideButtonWidth * scaleFactor));
        int scrollButtonHeight = std::max(8, static_cast<int>(originalSlideButtonHeight * scaleFactor * 1.5f));
        scrollButtonHeight = std::min(scrollButtonHeight, scrollbarTrackRect.h);
        int scrollTrackTravelHeight = std::max(0, scrollbarTrackRect.h - scrollButtonHeight);

        int maxScrollOffset = std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
        float percentageScrolled = (maxScrollOffset == 0) ? 0.0f : static_cast<float>(scrollOffset) / maxScrollOffset;

        scrollbarButtonRect.w = scrollButtonWidth;
        scrollbarButtonRect.h = scrollButtonHeight;
        scrollbarButtonRect.x = scrollbarTrackRect.x + (scrollbarTrackRect.w - scrollbarButtonRect.w) / 2; // Center button in track
        scrollbarButtonRect.y = scrollbarTrackRect.y + static_cast<int>(percentageScrolled * scrollTrackTravelHeight);
    } else {
         scrollbarTrackRect = {0,0,0,0};
         scrollbarButtonRect = {0,0,0,0};
    }

    // --- Volume Controls (Bottom Right - relative to windowWidth/Height) ---
    // Use scaled fixed sizes or sizes relative to window corner
    int iconSize = std::max(20, static_cast<int>(windowHeight * 0.04f)); // Approx 4% of height, min 20px
    int sliderHeight = std::max(3, static_cast<int>(originalSlidebarHeight * scaleFactor * 1.5f));
    int sliderButtonWidth = std::max(5, static_cast<int>(originalSlideButtonWidth * scaleFactor * 1.5f));
    int sliderButtonHeight = std::max(8, static_cast<int>(originalSlideButtonHeight * scaleFactor * 1.2f));
    int sliderWidth = std::max(50, static_cast<int>(windowWidth * 0.15f)); // Slider track width as % of window width
    int controlPadding = paddingX / 2; // Padding between icon and slider

    int rowHeight = iconSize + controlPadding; // Height for one row (icon determines it)
    int controlAreaHeight = rowHeight * 2 + controlPadding; // Height for two rows
    int controlAreaWidth = iconSize + controlPadding + sliderWidth; // Width needed

    int controlAreaX = windowWidth - controlAreaWidth - paddingX; // Position bottom right
    int controlAreaY = windowHeight - controlAreaHeight - paddingY;

    // Row 1: BGM
    int bgmRowY = controlAreaY;
    bgmIconRect = { controlAreaX, bgmRowY + (rowHeight - iconSize) / 2, iconSize, iconSize };
    bgmSliderTrackRect = { bgmIconRect.x + iconSize + controlPadding, bgmRowY + (rowHeight - sliderHeight) / 2, sliderWidth, sliderHeight };
    int bgmSliderTrackWidth = std::max(1, bgmSliderTrackRect.w - sliderButtonWidth);
    float bgmPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getMusicVolume()) / MIX_MAX_VOLUME; // Use static getter
    bgmSliderButtonRect = { bgmSliderTrackRect.x + static_cast<int>(bgmPercent * bgmSliderTrackWidth),
                             bgmSliderTrackRect.y + (sliderHeight - sliderButtonHeight) / 2,
                             sliderButtonWidth, sliderButtonHeight };

    // Row 2: SFX
    int sfxRowY = controlAreaY + rowHeight;
    sfxIconRect = { controlAreaX, sfxRowY + (rowHeight - iconSize) / 2, iconSize, iconSize };
    sfxSliderTrackRect = { sfxIconRect.x + iconSize + controlPadding, sfxRowY + (rowHeight - sliderHeight) / 2, sliderWidth, sliderHeight };
    int sfxSliderTrackWidth = std::max(1, sfxSliderTrackRect.w - sliderButtonWidth);
    float sfxPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getSfxVolume()) / MIX_MAX_VOLUME; // Use static getter
    sfxSliderButtonRect = { sfxSliderTrackRect.x + static_cast<int>(sfxPercent * sfxSliderTrackWidth),
                             sfxSliderTrackRect.y + (sliderHeight - sliderButtonHeight) / 2,
                             sliderButtonWidth, sliderButtonHeight };

    // --- Fullscreen
    if (fullscreenTextTex) {
        // Get texture dimensions
        SDL_QueryTexture(fullscreenTextTex, NULL, NULL, &fullscreenTextRect.w, &fullscreenTextRect.h);
        // Position above volume controls, aligned right with control area
        fullscreenTextRect.x = controlAreaX + controlAreaWidth - fullscreenTextRect.w; // Align right
        fullscreenTextRect.y = controlAreaY - fullscreenTextRect.h - paddingY / 2; // Place above volume area with padding
        // Ensure it doesn't go off screen top/left
        fullscreenTextRect.x = std::max(offsetX + paddingX, fullscreenTextRect.x);
        fullscreenTextRect.y = std::max(offsetY + paddingY, fullscreenTextRect.y);
    } else {
        fullscreenTextRect = {0,0,0,0}; // Reset if texture is null
    }
}