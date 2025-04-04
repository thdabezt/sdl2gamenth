#pragma once

// --- Includes ---
#include "Scene.h"
#include "../game.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>

// --- Structs ---

// Holds information for display in the save/load list
struct SaveSlotInfo {
    std::string timestamp;        // Formatted date/time string
    int level = 0;              // Player level from save
    std::string playerName;       // Player name from save
    std::string filename;         // Full path to the save file
    bool isNewGameOption = false; // True if this represents the "+ New Game" slot
};

// --- Class Definition ---

class MenuScene : public Scene {
public:
    // --- Constructor & Destructor ---
    MenuScene();
    ~MenuScene();

    // --- Public Methods (Scene Overrides) ---
    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;

private:
    // --- Private Members ---

    // Textures
    SDL_Texture* backgroundTexture = nullptr;
    SDL_Texture* titleTexture = nullptr;
    SDL_Texture* playButtonTexture = nullptr;
    SDL_Texture* nameBoxTexture = nullptr;      // Reused for input and save slots
    SDL_Texture* nameTextTexture = nullptr;     // For rendering player name input
    SDL_Texture* slideButtonTexture = nullptr;  // For scrollbar button
    SDL_Texture* sliderButtonTexture = nullptr; // For volume slider buttons
    SDL_Texture* soundOnTexture = nullptr;
    SDL_Texture* soundOffTexture = nullptr;
    SDL_Texture* sliderTrackTexture = nullptr; // Background for sliders
    SDL_Texture* fullscreenTextTex = nullptr; // Hint text for fullscreen toggle

    // Fonts
    TTF_Font* inputFont = nullptr;
    TTF_Font* saveSlotFont = nullptr;
    TTF_Font* uiHintFont = nullptr;

    // Audio
    Mix_Music* menuMusic = nullptr;
    Mix_Chunk* startSound = nullptr;
    Mix_Chunk* clickSound = nullptr;

    // Rendering Rectangles (positions calculated in calculateLayout)
    SDL_Rect backgroundRect;
    SDL_Rect titleRect;
    SDL_Rect playButtonRect;
    SDL_Rect nameInputBoxRect;
    SDL_Rect nameTextRect;
    SDL_Rect saveListAreaRect;
    std::vector<SDL_Rect> visibleSaveSlotRects; // Rects for the currently visible save slots
    SDL_Rect scrollbarTrackRect;
    SDL_Rect scrollbarButtonRect;
    SDL_Rect fullscreenTextRect;
    SDL_Rect bgmIconRect, bgmSliderTrackRect, bgmSliderButtonRect;
    SDL_Rect sfxIconRect, sfxSliderTrackRect, sfxSliderButtonRect;

    // Original Dimensions (Reference for scaling calculations)
    const int referenceWidth = 1920;
    const int referenceHeight = 1080;
    const int originalTitleWidth = 594;
    const int originalTitleHeight = 235;
    const int originalPlayButtonWidth = 95;
    const int originalPlayButtonHeight = 107;
    const int originalNameBoxWidth = 180;
    const int originalNameBoxHeight = 29;
    const int originalSlidebarHeight = 3; // Height of the slider track texture part
    const int originalSlideButtonWidth = 7;
    const int originalSlideButtonHeight = 13;
    int originalNameTextW = 0; // Stores original width of rendered name text
    int originalNameTextH = 0; // Stores original height of rendered name text

    // Name Input State
    std::string playerName = "";
    bool isEditingName = false;
    SDL_Color inputTextColor = { 0, 0, 0, 255 };       // Black
    SDL_Color placeholderTextColor = { 150, 150, 150, 255 }; // Grey

    // Save List & Scrolling State
    std::vector<SaveSlotInfo> saveSlots;
    SDL_Color saveSlotTextColorSelected = { 255, 255, 255, 255 }; // White
    SDL_Color saveSlotTextColorDefault = { 0, 0, 0, 255 };      // Black
    int scrollOffset = 0;                  // Index of the first visible save slot
    const int visibleSlotsCount = 3;       // Max number of save slots shown at once
    int selectedSaveSlotIndex = 0;         // Index in the full saveSlots vector
    bool isDraggingScrollbar = false;
    int scrollbarDragStartY = 0;           // Mouse Y offset when scrollbar drag started

    // Volume Control State
    bool isMusicMuted = false;
    bool isSfxMuted = false;
    int storedMusicVolumeBeforeMute = MIX_MAX_VOLUME / 2;
    int storedSfxVolumeBeforeMute = MIX_MAX_VOLUME / 2;
    bool isDraggingBgmSlider = false;
    bool isDraggingSfxSlider = false;
    int sliderDragStartX = 0;              // Mouse X offset when slider drag started

    // --- Private Methods ---
    SDL_Texture* loadTexture(const std::string& path); // Helper to load textures
    void calculateLayout(int windowWidth, int windowHeight); // Recalculates UI element positions
    void updateNameTexture(); // Renders the player name input to a texture
    void loadSaveFiles(); // Loads and parses save files from the directory
    std::string parseTimestampFromFilename(const std::string& filename); // Extracts formatted time
    int parseLevelFromSaveFile(const std::string& filepath); // Reads player level
    std::string parseNameFromSaveFile(const std::string& filepath); // Reads player name

}; // End MenuScene class