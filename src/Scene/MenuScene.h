#pragma once

#include "Scene.h"
#include "../game.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>

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

    SDL_Texture* backgroundTexture = nullptr;
    SDL_Texture* titleTexture = nullptr;
    SDL_Texture* playButtonTexture = nullptr;
    SDL_Texture* nameBoxTexture = nullptr;      
    SDL_Texture* nameTextTexture = nullptr;     
    SDL_Texture* slideButtonTexture = nullptr;  
    SDL_Texture* sliderButtonTexture = nullptr; 
    SDL_Texture* soundOnTexture = nullptr;
    SDL_Texture* soundOffTexture = nullptr;
    SDL_Texture* sliderTrackTexture = nullptr; 
    SDL_Texture* fullscreenTextTex = nullptr; 

    TTF_Font* inputFont = nullptr;
    TTF_Font* saveSlotFont = nullptr;
    TTF_Font* uiHintFont = nullptr;

    Mix_Music* menuMusic = nullptr;
    Mix_Chunk* startSound = nullptr;
    Mix_Chunk* clickSound = nullptr;

    SDL_Rect backgroundRect;
    SDL_Rect titleRect;
    SDL_Rect playButtonRect;
    SDL_Rect nameInputBoxRect;
    SDL_Rect nameTextRect;
    SDL_Rect saveListAreaRect;
    std::vector<SDL_Rect> visibleSaveSlotRects; 
    SDL_Rect scrollbarTrackRect;
    SDL_Rect scrollbarButtonRect;
    SDL_Rect fullscreenTextRect;
    SDL_Rect bgmIconRect, bgmSliderTrackRect, bgmSliderButtonRect;
    SDL_Rect sfxIconRect, sfxSliderTrackRect, sfxSliderButtonRect;

    const int referenceWidth = 1920;
    const int referenceHeight = 1080;
    const int originalTitleWidth = 594;
    const int originalTitleHeight = 235;
    const int originalPlayButtonWidth = 95;
    const int originalPlayButtonHeight = 107;
    const int originalNameBoxWidth = 180;
    const int originalNameBoxHeight = 29;
    const int originalSlidebarHeight = 3; 
    const int originalSlideButtonWidth = 7;
    const int originalSlideButtonHeight = 13;
    int originalNameTextW = 0; 
    int originalNameTextH = 0; 

    std::string playerName = "";
    bool isEditingName = false;
    SDL_Color inputTextColor = { 0, 0, 0, 255 };       
    SDL_Color placeholderTextColor = { 150, 150, 150, 255 }; 

    std::vector<SaveSlotInfo> saveSlots;
    SDL_Color saveSlotTextColorSelected = { 255, 255, 255, 255 }; 
    SDL_Color saveSlotTextColorDefault = { 0, 0, 0, 255 };      
    int scrollOffset = 0;                  
    const int visibleSlotsCount = 3;       
    int selectedSaveSlotIndex = 0;         
    bool isDraggingScrollbar = false;
    int scrollbarDragStartY = 0;           

    bool isMusicMuted = false;
    bool isSfxMuted = false;
    int storedMusicVolumeBeforeMute = MIX_MAX_VOLUME / 2;
    int storedSfxVolumeBeforeMute = MIX_MAX_VOLUME / 2;
    bool isDraggingBgmSlider = false;
    bool isDraggingSfxSlider = false;
    int sliderDragStartX = 0;              

    SDL_Texture* loadTexture(const std::string& path); 
    void calculateLayout(int windowWidth, int windowHeight); 
    void updateNameTexture(); 
    void loadSaveFiles(); 
    std::string parseTimestampFromFilename(const std::string& filename); 
    int parseLevelFromSaveFile(const std::string& filepath); 
    std::string parseNameFromSaveFile(const std::string& filepath); 

}; 