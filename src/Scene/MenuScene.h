// Game/src/Scene/MenuScene.h
#pragma once
#include "Scene.h"
#include "../game.h"
#include "../SaveLoadManager.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

// Save Slot Info Struct (remains the same)
struct SaveSlotInfo {
    std::string timestamp;   
    int level = 0;      
    std::string playerName;
    std::string filename; 
    bool isNewGameOption = false; 
};

class MenuScene : public Scene {
public:
    MenuScene();
    ~MenuScene();

    void init() override;
    void handleEvents(SDL_Event& event) override;
    void update() override;
    void render() override;
    void clean() override;

private:
    // --- Textures ---
    SDL_Texture* backgroundTexture = nullptr;
    SDL_Texture* titleTexture = nullptr;
    SDL_Texture* playButtonTexture = nullptr;
    SDL_Texture* nameBoxTexture = nullptr; // Used for name input AND save slots
    SDL_Texture* nameTextTexture = nullptr;
    // REMOVED: slideBarTexture
    SDL_Texture* slideButtonTexture = nullptr; // Keep button
    Mix_Music* menuMusic = nullptr;
    SDL_Texture* fullscreenTextTex = nullptr;
    TTF_Font* uiHintFont = nullptr;

    // --- ADDED: Sounds loaded directly by MenuScene ---
    Mix_Chunk* startSound = nullptr;
    Mix_Chunk* clickSound = nullptr;
    // --- END ADDED ---
    
    // --- Rectangles for Rendering (Scaled) ---
    SDL_Rect backgroundRect;
    SDL_Rect titleRect;
    SDL_Rect playButtonRect;
    SDL_Rect nameInputBoxRect;
    SDL_Rect nameTextRect;
    // Save List UI Rects
    SDL_Rect saveListAreaRect;
    std::vector<SDL_Rect> visibleSaveSlotRects;
    SDL_Rect scrollbarTrackRect; // Keep track area rect for drawing background and positioning button
    SDL_Rect scrollbarButtonRect;
    SDL_Rect fullscreenTextRect;  

     // --- Volume Controls ---
     SDL_Texture* soundOnTexture = nullptr;
     SDL_Texture* soundOffTexture = nullptr;
     SDL_Texture* sliderTrackTexture = nullptr; // Reuse slidebar.png
     SDL_Texture* sliderButtonTexture = nullptr; // Reuse slidebutton.png
 
     SDL_Rect bgmIconRect;
     SDL_Rect bgmSliderTrackRect;
     SDL_Rect bgmSliderButtonRect;
     SDL_Rect sfxIconRect;
     SDL_Rect sfxSliderTrackRect;
     SDL_Rect sfxSliderButtonRect;
 
     // Store target volumes for menu adjustments
     int menuTargetMusicVolume = MIX_MAX_VOLUME / 2;
     int menuTargetSfxVolume = MIX_MAX_VOLUME / 2;
     // Mute state and stored volume before mute
     bool isMusicMuted = false;
     bool isSfxMuted = false;
     int storedMusicVolumeBeforeMute = MIX_MAX_VOLUME / 2;
     int storedSfxVolumeBeforeMute = MIX_MAX_VOLUME / 2;
     // Dragging state
     bool isDraggingBgmSlider = false;
     bool isDraggingSfxSlider = false;
     int sliderDragStartX = 0; // Horizontal sliders now

    // --- Original Dimensions (Reference for scaling) ---
    const int referenceWidth = 1920;
    const int referenceHeight = 1080;
    const int originalTitleWidth = 594;
    const int originalTitleHeight = 235;
    const int originalPlayButtonWidth = 95;
    const int originalPlayButtonHeight = 107;
    const int originalNameBoxWidth = 180;
    const int originalNameBoxHeight = 29;
    const int originalSlidebarHeight = 3;
    const int originalSlideButtonWidth = 7; // Use directly assuming vertical orientation now
    const int originalSlideButtonHeight = 13;


    // --- Name Input ---
    std::string playerName = "";
    bool isEditingName = false;
    TTF_Font* inputFont = nullptr;
    SDL_Color inputTextColor = { 0, 0, 0, 255 };
    SDL_Color placeholderTextColor = { 150, 150, 150, 255 };
    int originalNameTextW = 0; // <<< ADDED: Store original width from TTF_RenderText
    int originalNameTextH = 0; // <<< ADDED: Store original height from TTF_RenderText

    // --- Save List & Scrolling ---
    std::vector<SaveSlotInfo> saveSlots;
    TTF_Font* saveSlotFont = nullptr;
    SDL_Color saveSlotTextColorSelected = { 255, 255, 255, 255 }; // White
    SDL_Color saveSlotTextColorDefault = { 0, 0, 0, 255 };      // Black
    int scrollOffset = 0;
    const int visibleSlotsCount = 3;
    int selectedSaveSlotIndex = 0;
    bool isDraggingScrollbar = false;
    int scrollbarDragStartY = 0;


    // --- Helper Functions ---
    SDL_Texture* loadTexture(const std::string& path);
    void calculateLayout(int windowWidth, int windowHeight);
    void updateNameTexture();
    void loadSaveFiles();
    std::string parseTimestampFromFilename(const std::string& filename);
    int parseLevelFromSaveFile(const std::string& filepath);
    std::string parseNameFromSaveFile(const std::string& filepath);
};