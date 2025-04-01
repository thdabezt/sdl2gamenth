// Game/src/Scene/MenuScene.cpp
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
#include "../AssetManager.h"

// Define namespace alias for easier use
namespace fs = std::filesystem;

// --- Constructor ---
MenuScene::MenuScene() {}

// --- Destructor ---
MenuScene::~MenuScene() {
    clean();
}

// --- Helper: Load Texture ---
SDL_Texture* MenuScene::loadTexture(const std::string& path) {
     SDL_Texture* newTexture = TextureManager::LoadTexture(path.c_str());
     if (newTexture == nullptr) {
         std::cerr << "Failed to load texture: " << path << " SDL_Error: " << SDL_GetError() << std::endl;
     }
     return newTexture;
}

// --- Helper: Update Name Texture ---
void MenuScene::updateNameTexture() {
    if (nameTextTexture) {
        SDL_DestroyTexture(nameTextTexture);
        nameTextTexture = nullptr;
    }
    // Reset original dimensions when texture is cleared or fails
    originalNameTextW = 0;
    originalNameTextH = 0;

    if (!inputFont) {
         std::cerr << "Cannot update name texture: inputFont is null!" << std::endl;
         // No need to call calculateLayout here if font failed
         return;
    }

    std::string textToRender = playerName;
    SDL_Color colorToUse = inputTextColor;

    // Determine text and color (handle placeholder)
    if (selectedSaveSlotIndex == 0 && !isEditingName && playerName.empty()) {
        textToRender = "Enter Name (Optional)";
        colorToUse = placeholderTextColor;
    } else if (playerName.empty()) { // Applies if editing OR (not editing and not placeholder case)
         // Don't render texture if name is truly empty
         int w, h;
         if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600; }
         calculateLayout(w, h); // Recalculate layout to position cursor correctly
         return;
    }

    // Render the text surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(inputFont, textToRender.c_str(), colorToUse);
    if (!textSurface) {
        std::cerr << "Unable to render name text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        // No need to call calculateLayout here if render failed
        return;
    }

    // Create texture from surface
    nameTextTexture = SDL_CreateTextureFromSurface(Game::renderer, textSurface);
    if (!nameTextTexture) {
        std::cerr << "Unable to create name text texture! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        // --- Store ORIGINAL dimensions ---
        originalNameTextW = textSurface->w; // <<< CHANGED
        originalNameTextH = textSurface->h; // <<< CHANGED
    }
    SDL_FreeSurface(textSurface);

    // Recalculate layout to position the potentially new text size correctly
    int w, h;
    if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600; }
    calculateLayout(w,h);
}


// --- Helper: Parse Timestamp from Filename ---
std::string MenuScene::parseTimestampFromFilename(const std::string& filename) {
    size_t last_slash_idx = filename.find_last_of("\\/");
    std::string name_only = (last_slash_idx == std::string::npos) ? filename : filename.substr(last_slash_idx + 1);
    size_t dot_idx = name_only.rfind(".state");
    if (dot_idx == std::string::npos) return "Unknown Date";
    std::string timestamp_str = name_only.substr(0, dot_idx);
    if (timestamp_str.length() == 15 && timestamp_str[8] == '-') {
        return timestamp_str.substr(0, 2) + "/" + timestamp_str.substr(2, 2) + "/" + timestamp_str.substr(4, 4) +
               " " + timestamp_str.substr(9, 2) + ":" + timestamp_str.substr(11, 2);
    }
    return timestamp_str;
}

// --- Helper: Parse Level from Save File ---
int MenuScene::parseLevelFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) return 0;
    std::string line, key, value;
    int level = 1;
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerLevel") {
            try { level = std::stoi(value); break; }
            catch (...) { level = 1; break; }
        }
    }
    saveFile.close();
    return std::max(1, level);
}



// --- Load Save Files (with filtering and explicit construction) ---
void MenuScene::loadSaveFiles() {
    std::cout << "Loading save files..." << std::endl;
    saveSlots.clear();
    // --- MODIFIED: Explicitly construct SaveSlotInfo ---
    saveSlots.push_back(SaveSlotInfo{ "", 0, "Player", "", true }); // Add "+ New Game" option first

    std::string saveDir = "saves";
    std::vector<fs::path> stateFiles;

    try {
        // Check if the directory exists, create it if it doesn't
        if (!fs::exists(saveDir) || !fs::is_directory(saveDir)) {
            std::cout << "'saves' directory not found or is not a directory. Creating..." << std::endl;
             if (!fs::create_directory(saveDir)) {
                  std::cerr << "Failed to create 'saves' directory!" << std::endl;
                  // Still proceed, list will just have "+ New Game"
             }
        }

        // Iterate through the directory
        for (const auto& entry : fs::directory_iterator(saveDir)) {
            // --- Filter specific files ---
            std::string filename_str = entry.path().filename().string();
            if (filename_str == "default.state" || filename_str == "quicksave.state") {
                std::cout << "Skipping file: " << filename_str << std::endl;
                continue; // Skip this file
            }
            // --- End Filter ---

            // Check if it's a regular file with the correct extension
            if (entry.is_regular_file() && entry.path().extension() == ".state") {
                stateFiles.push_back(entry.path());
            }
        }

        // Sort collected files by last write time (latest first)
        std::sort(stateFiles.begin(), stateFiles.end(), [](const fs::path& a, const fs::path& b) {
            try {
                // Handle potential errors if file is deleted between listing and sorting
                 if (!fs::exists(a) || !fs::exists(b)) return false;
                 return fs::last_write_time(a) > fs::last_write_time(b);
            } catch (const fs::filesystem_error& e) {
                 std::cerr << "Error comparing file times: " << e.what() << std::endl;
                 return false; // Maintain original order on error
            }
        });


        // Process sorted valid files
        for (const auto& path : stateFiles) {
            std::string filename = path.filename().string();
            std::string fullpath = path.string();
            // Ensure consistent path separators
            std::replace(fullpath.begin(), fullpath.end(), '\\', '/');

            // --- Store separate pieces of info ---
            std::string timestamp = parseTimestampFromFilename(filename);
            int level = parseLevelFromSaveFile(fullpath);
            std::string name = parseNameFromSaveFile(fullpath);

            // --- MODIFIED: Explicitly construct SaveSlotInfo ---
            saveSlots.push_back(SaveSlotInfo{ timestamp, level, name, fullpath, false });
            // --- End Storing separate info ---
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error while reading saves: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Catch other potential exceptions during file processing
        std::cerr << "Error loading save files: " << e.what() << std::endl;
    }

    // Reset scroll position and default selection
    scrollOffset = 0;
    selectedSaveSlotIndex = 0; // Default selection back to "+ New Game"
    std::cout << "Found " << saveSlots.size() -1 << " valid save files." << std::endl;
}



void MenuScene::init() {
    clean();
    std::cout << "Initializing Menu Scene..." << std::endl;
    isEditingName = false;
    playerName = "";

    // --- ADDED: Load sounds directly for MenuScene ---
    startSound = Mix_LoadWAV("assets/sound/start.wav");
    clickSound = Mix_LoadWAV("assets/sound/buttonclick.wav");
    if (!startSound) { std::cerr << "Failed to load start.wav: " << Mix_GetError() << std::endl; }
    if (!clickSound) { std::cerr << "Failed to load buttonclick.wav: " << Mix_GetError() << std::endl; }
    // --- END ADDED ---

    // --- Load Textures (Order matters less for these) ---
    backgroundTexture = loadTexture("assets/menu/bg.png");
    titleTexture = loadTexture("assets/menu/title.png");
    playButtonTexture = loadTexture("assets/menu/play.png");
    nameBoxTexture = loadTexture("assets/menu/box.png");
    soundOnTexture = loadTexture("assets/menu/soundon.png");
    soundOffTexture = loadTexture("assets/menu/soundoff.png");
    sliderTrackTexture = loadTexture("assets/menu/slidebar.png");
    slideButtonTexture = loadTexture("assets/menu/slidebutton.png"); // Reuse if loaded for scrollbar
    sliderButtonTexture = loadTexture("assets/menu/slidebutton.png");
    // --- Load Fonts ---
    inputFont = TTF_OpenFont("assets/font.ttf", 20);
    saveSlotFont = TTF_OpenFont("assets/font.ttf", 18);
    uiHintFont = TTF_OpenFont("assets/font.ttf", 12); // Font for hint text

    // Basic Font Check/Fallback
    if (!inputFont) { std::cerr << "Failed to load input font!" << std::endl; /* Add fallback if needed */ }
    if (!saveSlotFont) { std::cerr << "Failed to load save slot font!" << std::endl; /* Add fallback if needed */ }
    if (!uiHintFont) {
        std::cerr << "Failed to load hint font! Using save slot font as fallback." << std::endl;
        if(saveSlotFont) uiHintFont = saveSlotFont; // Fallback
        else if(inputFont) uiHintFont = inputFont; // Further fallback
        else std::cerr << "FATAL: No valid font loaded for hints!" << std::endl;
    }

    // --- Initialize Volume State ---
    isMusicMuted = (Game::getMusicVolume() == 0);
    storedMusicVolumeBeforeMute = isMusicMuted ? (MIX_MAX_VOLUME / 2) : Game::getMusicVolume();
    isSfxMuted = (Game::getSfxVolume() == 0);
    storedSfxVolumeBeforeMute = isSfxMuted ? (MIX_MAX_VOLUME / 2) : Game::getSfxVolume();
    isDraggingBgmSlider = false;
    isDraggingSfxSlider = false;

    // --- Load Save Files (Might affect selection state) ---
    loadSaveFiles();

    // --- Render Text Textures AFTER fonts are loaded ---
    // Render Name Text Texture (Placeholder or loaded name)
    updateNameTexture();

    // Pre-render Fullscreen Hint Text Texture
    if (uiHintFont && Game::renderer) { // Check font and renderer
        SDL_Color hintColor = { 200, 200, 200, 255 }; // Light grey color
        SDL_Surface* surface = TTF_RenderText_Blended(uiHintFont, "F11: Fullscreen", hintColor);
        if(surface) {
            // Destroy previous texture if any
            if (fullscreenTextTex) { SDL_DestroyTexture(fullscreenTextTex); fullscreenTextTex = nullptr; }
            fullscreenTextTex = SDL_CreateTextureFromSurface(Game::renderer, surface);
            SDL_FreeSurface(surface);
            if (!fullscreenTextTex) {
                std::cerr << "Failed to create fullscreen hint texture! SDL Error: " << SDL_GetError() << std::endl;
            } else {
                 std::cout << "DEBUG: fullscreenTextTex created successfully." << std::endl;
            }
        } else {
            std::cerr << "Failed to render fullscreen hint surface! TTF Error: " << TTF_GetError() << std::endl;
        }
    } else {
        if(!uiHintFont) std::cerr << "Cannot render hint text: uiHintFont is null." << std::endl;
        if(!Game::renderer) std::cerr << "Cannot render hint text: Game::renderer is null." << std::endl;
    }

    // --- BGM Handling ---
    Mix_HaltMusic();
    const char* menuMusicPath = "assets/sound/menubgm.mp3";
    if (menuMusic) { Mix_FreeMusic(menuMusic); menuMusic = nullptr; } // Free old music if any
    menuMusic = Mix_LoadMUS(menuMusicPath);
    if (!menuMusic) { /* ... error log ... */ }
    if (menuMusic) {
         if (Mix_PlayMusic(menuMusic, -1) == -1) { /* ... error log ... */ }
         else { Mix_VolumeMusic(Game::getMusicVolume()); } // Apply current volume
    }
    // --- End BGM Handling ---


    // --- Calculate Initial Layout AFTER all assets potentially affecting it are loaded/created ---
    int w, h;
    if (Game::renderer) {
        SDL_GetRendererOutputSize(Game::renderer, &w, &h);
        calculateLayout(w, h); // Now calculate layout
    } else {
        std::cerr << "Renderer not available during MenuScene init for layout calc!" << std::endl;
        w = 800; h = 600;
        calculateLayout(w, h); // Attempt layout with default size
    }

    std::cout << "Menu Scene Initialized." << std::endl;
}

// Replace the existing calculateLayout function in Game/src/Scene/MenuScene.cpp

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
        // Scale slightly? Or keep original small size? Let's keep original for now.
        // float hintTextScale = scaleFactor * 0.7f; // Example scaling
        // fullscreenTextRect.w = static_cast<int>(fullscreenTextRect.w * hintTextScale);
        // fullscreenTextRect.h = static_cast<int>(fullscreenTextRect.h * hintTextScale);

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



// --- handleEvents (Removed scrollbar dragging logic related to track texture, mouse capture) ---
// Replace the existing handleEvents function in Game/src/Scene/MenuScene.cpp with this complete version:

void MenuScene::handleEvents(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mousePoint = {mouseX, mouseY};

    // --- Window Resize Handling ---
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
             updateNameTexture();
             // Recalculate layout to resize and reposition everything
             int w, h;
             if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;}
             calculateLayout(w, h);
        }
    }
    // --- Mouse Wheel Scroll ---
    else if (event.type == SDL_MOUSEWHEEL) {
        bool canScroll = saveSlots.size() > visibleSlotsCount;
        SDL_Rect scrollArea = saveListAreaRect;
        if (scrollbarTrackRect.w > 0) { // Include track area only if scrollbar is visible
             scrollArea.w += scrollbarTrackRect.w + (scrollbarTrackRect.x - (saveListAreaRect.x + saveListAreaRect.w));
             scrollArea.h = std::max(scrollArea.h, scrollbarTrackRect.h); // Ensure height covers track too
        }

        if (canScroll && SDL_PointInRect(&mousePoint, &scrollArea)) {
            int oldOffset = scrollOffset;
            if (event.wheel.y > 0) scrollOffset--; // Scroll Up
            else if (event.wheel.y < 0) scrollOffset++; // Scroll Down

            // Clamp scrollOffset
            int maxScrollOffset = std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
            scrollOffset = std::max(0, std::min(scrollOffset, maxScrollOffset));

            if(oldOffset != scrollOffset) {
                std::cout << "Scroll Offset: " << scrollOffset << std::endl;
                 // Recalculate layout to update scrollbar position based on new offset
                 int w, h;
                 if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;}
                 calculateLayout(w, h);
            }
        }
    }
    // --- Mouse Button Down ---
     else if (event.type == SDL_MOUSEBUTTONDOWN) {
         if (event.button.button == SDL_BUTTON_LEFT) {

            bool playClickSound = false; // Flag to play generic click sound

             // --- Check Mute Icon Clicks ---
             if (SDL_PointInRect(&mousePoint, &bgmIconRect)) {
                 bool wasMuted = (Game::getMusicVolume() == 0);
                 if (!wasMuted) { storedMusicVolumeBeforeMute = Game::getMusicVolume(); Game::setMusicVolume(0); }
                 else { Game::setMusicVolume(storedMusicVolumeBeforeMute > 0 ? storedMusicVolumeBeforeMute : MIX_MAX_VOLUME / 4); storedMusicVolumeBeforeMute = Game::getMusicVolume(); }
                 isMusicMuted = (Game::getMusicVolume() == 0);
                 if(menuMusic) Mix_VolumeMusic(Game::getMusicVolume());
                 int w, h; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;} calculateLayout(w, h);
                 playClickSound = true; // Flag to play click sound
                 // return; // Decide if execution should stop immediately
             }
             // Use 'else if' to prevent double plays if elements overlap
             else if (SDL_PointInRect(&mousePoint, &sfxIconRect)) {
                 bool wasMuted = (Game::getSfxVolume() == 0);
                 if (!wasMuted) { storedSfxVolumeBeforeMute = Game::getSfxVolume(); Game::setSfxVolume(0); }
                 else { Game::setSfxVolume(storedSfxVolumeBeforeMute > 0 ? storedSfxVolumeBeforeMute : MIX_MAX_VOLUME / 4); storedSfxVolumeBeforeMute = Game::getSfxVolume(); }
                 isSfxMuted = (Game::getSfxVolume() == 0);
                 Mix_Volume(-1, Game::getSfxVolume());
                 int w, h; if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;} calculateLayout(w, h);
                 playClickSound = true; // Flag to play click sound
                 // return;
             }
             // --- Check Slider Button Drag Start ---
             else if (saveSlots.size() > visibleSlotsCount && SDL_PointInRect(&mousePoint, &bgmSliderButtonRect)) {
                 isDraggingBgmSlider = true;
                 sliderDragStartX = mouseX - bgmSliderButtonRect.x;
                 playClickSound = true; // Play sound on initial slider click
                 std::cout << "BGM Slider drag started." << std::endl;
                 // return;
             }
              else if (saveSlots.size() > visibleSlotsCount && SDL_PointInRect(&mousePoint, &sfxSliderButtonRect)) {
                 isDraggingSfxSlider = true;
                 sliderDragStartX = mouseX - sfxSliderButtonRect.x;
                 playClickSound = true; // Play sound on initial slider click
                 std::cout << "SFX Slider drag started." << std::endl;
                 // return;
             }
             // --- Check Scrollbar Button Drag Start ---
              else if (saveSlots.size() > visibleSlotsCount && SDL_PointInRect(&mousePoint, &scrollbarButtonRect)) {
                   if (!isDraggingScrollbar) {
                       isDraggingScrollbar = true;
                       scrollbarDragStartY = mouseY - scrollbarButtonRect.y;
                       playClickSound = true; // Play sound on initial scrollbar click
                       std::cout << "Scrollbar drag started." << std::endl;
                   }
                   // return;
              }
             // --- Play Button Click ---
              else if (SDL_PointInRect(&mousePoint, &playButtonRect)) {
                   if (isEditingName) { isEditingName = false; SDL_StopTextInput(); updateNameTexture(); }
                  std::cout << "Play button clicked!" << std::endl;

                  // Play START sound
                  if(startSound) Mix_PlayChannel(-1, startSound, 0);
                  playClickSound = false; // Don't play the generic click sound

                  if (selectedSaveSlotIndex >= 0 && selectedSaveSlotIndex < saveSlots.size()) {
                      const auto& selectedSlot = saveSlots[selectedSaveSlotIndex];
                      if (selectedSlot.isNewGameOption) {
                          // Start New Game logic
                          std::cout << "Starting new game with name input: '" << this->playerName << "'" << std::endl;
                          Scene* gameScenePtr = SceneManager::instance->getScene(SceneType::Game);
                          GameScene* gameScene = dynamic_cast<GameScene*>(gameScenePtr);
                          if (gameScene) {
                              gameScene->resetGame();
                              gameScene->init();
                              if (Game::instance) {
                                  Game::instance->setPlayerName(this->playerName);
                                  // Volume already set globally via static members
                              } else { std::cerr << "Error: Game::instance is null after GameScene init!" << std::endl; }
                          } else { std::cerr << "Error: Could not get/cast GameScene!" << std::endl; }
                          SceneManager::instance->switchToScene(SceneType::Game);
                      } else {
                          // Load Existing Game logic
                           std::string filenameToLoad = selectedSlot.filename;
                            std::cout << "Play clicked while save selected. Loading game: " << filenameToLoad << std::endl;
                            Scene* gameScenePtr = SceneManager::instance->getScene(SceneType::Game);
                            GameScene* gameScene = dynamic_cast<GameScene*>(gameScenePtr);
                            if (gameScene) {
                                gameScene->resetGame();
                                gameScene->init();
                                if (Game::instance && Game::instance->saveLoadManager) {
                                    if (Game::instance->saveLoadManager->loadGameState(filenameToLoad)) {
                                         std::cout << "Load successful. Switching scene..." << std::endl;
                                         // Static volume persists unless overwritten by loaded save data
                                         SceneManager::instance->switchToScene(SceneType::Game);
                                    } else { std::cerr << "Failed to load game state: " << filenameToLoad << std::endl; }
                                } else { std::cerr << "Critical Error: SaveLoadManager/Game Instance not available!" << std::endl; }
                            } else { std::cerr << "Error: Could not get/cast GameScene to load game!" << std::endl; }
                      } // End else (Load Game)
                  } else { std::cerr << "Error: Invalid selected save slot index: " << selectedSaveSlotIndex << ". Please select an option." << std::endl; }
                  return; // Handled play button click
             } // End Play Button Check

             // --- Name Input Box Click ---
             else if (SDL_PointInRect(&mousePoint, &nameInputBoxRect)) {
                  if (!isEditingName) {
                       if (selectedSaveSlotIndex >= 0 && selectedSaveSlotIndex < saveSlots.size() && saveSlots[selectedSaveSlotIndex].isNewGameOption) {
                           isEditingName = true;
                           SDL_StartTextInput();
                           updateNameTexture();
                           playClickSound = true; // Play sound for activating input
                       }
                  }
                  // Let click outside logic run if necessary
             }

             // --- Save Slot clicks ---
             else { // Check slots only if previous elements weren't handled by a return
                 bool clickedOnSlot = false;
                 for (int i = 0; i < visibleSlotsCount; ++i) {
                     int actualIndex = scrollOffset + i;
                     if (actualIndex >= 0 && actualIndex < saveSlots.size()) {
                         if (SDL_PointInRect(&mousePoint, &visibleSaveSlotRects[i])) {
                             if (selectedSaveSlotIndex != actualIndex) {
                                 selectedSaveSlotIndex = actualIndex;
                                 if (saveSlots[actualIndex].isNewGameOption) {
                                     playerName = ""; isEditingName = true; SDL_StartTextInput();
                                 } else {
                                     if (isEditingName) { isEditingName = false; SDL_StopTextInput(); }
                                     playerName = saveSlots[actualIndex].playerName;
                                 }
                                 updateNameTexture();
                                 playClickSound = true; // Play sound for selection change
                             } else {
                                 playClickSound = true; // Play click sound even if same slot clicked
                             }
                             clickedOnSlot = true;
                             break;
                         }
                     }
                 } // End loop through visible slots

                 // Click Outside interactive elements
                 if (!clickedOnSlot && isEditingName &&
                     !SDL_PointInRect(&mousePoint, &nameInputBoxRect) &&
                     !SDL_PointInRect(&mousePoint, &playButtonRect) &&
                     !SDL_PointInRect(&mousePoint, &scrollbarButtonRect) &&
                     !SDL_PointInRect(&mousePoint, &bgmIconRect) &&
                     !SDL_PointInRect(&mousePoint, &sfxIconRect) &&
                     !SDL_PointInRect(&mousePoint, &bgmSliderButtonRect) &&
                     !SDL_PointInRect(&mousePoint, &sfxSliderButtonRect))
                 {
                      isEditingName = false; SDL_StopTextInput(); updateNameTexture();
                      // No click sound for clicking outside
                 }
             } // End Save Slot / Click Outside Check


             // --- Play the generic click sound if flagged by any relevant action ---
             if (playClickSound && clickSound) {
                 Mix_PlayChannel(-1, clickSound, 0);
             }

         } // End Left Mouse Button Check
     } // End Mouse Button Down Event

     // --- Mouse Button Up ---
     else if (event.type == SDL_MOUSEBUTTONUP) {
          if (event.button.button == SDL_BUTTON_LEFT) {
               // Reset all dragging flags on left button release
               if (isDraggingScrollbar || isDraggingBgmSlider || isDraggingSfxSlider) {
                    isDraggingScrollbar = false;
                    isDraggingBgmSlider = false;
                    isDraggingSfxSlider = false;
                    std::cout << "Slider/Scrollbar drag ended." << std::endl;
               }
          }
     }
     // --- Mouse Motion (Handle Slider/Scrollbar Drag) ---
     else if (event.type == SDL_MOUSEMOTION) {
          int w, h;
          if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;}

          if (isDraggingBgmSlider) {
                int trackX = bgmSliderTrackRect.x;
                int trackButtonW = bgmSliderButtonRect.w;
                int trackW = bgmSliderTrackRect.w - trackButtonW;
                trackW = std::max(1, trackW);
                int targetButtonX = mouseX - sliderDragStartX;
                targetButtonX = std::max(trackX, std::min(targetButtonX, trackX + trackW));
                float percent = static_cast<float>(targetButtonX - trackX) / trackW;
                int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
                if (newVolume != Game::getMusicVolume()) {
                     Game::setMusicVolume(newVolume);
                     isMusicMuted = (Game::getMusicVolume() == 0);
                     if(!isMusicMuted) storedMusicVolumeBeforeMute = Game::getMusicVolume();
                     calculateLayout(w, h);
                     if(menuMusic) Mix_VolumeMusic(Game::getMusicVolume());
                     // No click sound during drag
                }
          }
           else if (isDraggingSfxSlider) {
                int trackX = sfxSliderTrackRect.x;
                int trackButtonW = sfxSliderButtonRect.w;
                int trackW = sfxSliderTrackRect.w - trackButtonW;
                trackW = std::max(1, trackW);
                int targetButtonX = mouseX - sliderDragStartX;
                targetButtonX = std::max(trackX, std::min(targetButtonX, trackX + trackW));
                float percent = static_cast<float>(targetButtonX - trackX) / trackW;
                int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
                if (newVolume != Game::getSfxVolume()) {
                     Game::setSfxVolume(newVolume);
                     isSfxMuted = (Game::getSfxVolume() == 0);
                     if(!isSfxMuted) storedSfxVolumeBeforeMute = Game::getSfxVolume();
                     calculateLayout(w, h);
                     // No click sound during drag
                }
          }
           else if (isDraggingScrollbar) { // Handle Scrollbar Drag
              int scrollTrackHeight = std::max(1, scrollbarTrackRect.h - scrollbarButtonRect.h);
              int targetButtonTop = mouseY - scrollbarDragStartY;
              targetButtonTop = std::max(scrollbarTrackRect.y, std::min(targetButtonTop, scrollbarTrackRect.y + scrollTrackHeight));
              float percentageScrolled = static_cast<float>(targetButtonTop - scrollbarTrackRect.y) / scrollTrackHeight;
              int maxScrollOffset = std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
              int newOffset = static_cast<int>(std::round(percentageScrolled * maxScrollOffset));
              newOffset = std::max(0, std::min(newOffset, maxScrollOffset));
              if (newOffset != scrollOffset) {
                   scrollOffset = newOffset;
                   calculateLayout(w, h);
              }
          }
     }
     // --- Text Input ---
     else if (event.type == SDL_TEXTINPUT && isEditingName) {
         const int MAX_NAME_LENGTH = 15;
         if (playerName.length() < MAX_NAME_LENGTH) {
              playerName += event.text.text;
              std::cout << "TextInput: " << playerName << std::endl;
              updateNameTexture();
         }
     }
     // --- Key Down ---
     else if (event.type == SDL_KEYDOWN) {
         if (isEditingName) { // Handle keys only when editing name
             if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
                 playerName.pop_back();
                 std::cout << "Backspace: " << playerName << std::endl;
                 updateNameTexture();
             } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                  std::cout << "Enter pressed - disabling input." << std::endl;
                  isEditingName = false;
                  SDL_StopTextInput();
                  updateNameTexture();
             } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                  std::cout << "Escape pressed during edit - cancelling." << std::endl;
                  isEditingName = false;
                  SDL_StopTextInput();
                  playerName = "";
                  updateNameTexture();
             }
         }

         // Global keydown handling (like ESC to quit if NOT editing name)
         if (event.key.keysym.sym == SDLK_ESCAPE && !isEditingName) {
             std::cout << "Exit requested from Menu" << std::endl;
             SDL_Event quitEvent;
             quitEvent.type = SDL_QUIT;
             SDL_PushEvent(&quitEvent);
         }
     } // End Key Down Event

} // End handleEvents

// --- update ---
void MenuScene::update() {}

// --- render (Draw rect for track) ---
void MenuScene::render() {
    if (!Game::renderer) { return; }
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);
    SDL_RenderClear(Game::renderer);

    // Render Background & Center Column
    if (backgroundTexture) SDL_RenderCopy(Game::renderer, backgroundTexture, NULL, &backgroundRect);
    if (titleTexture) SDL_RenderCopy(Game::renderer, titleTexture, NULL, &titleRect);
    if (playButtonTexture) SDL_RenderCopy(Game::renderer, playButtonTexture, NULL, &playButtonRect);
    if (nameBoxTexture) SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &nameInputBoxRect);
    if (nameTextTexture) SDL_RenderCopy(Game::renderer, nameTextTexture, NULL, &nameTextRect);

    // In render() function in MenuScene.cpp:

    // --- Render Left Column (Save List) ---
    int slotsToDraw = std::min(visibleSlotsCount, static_cast<int>(saveSlots.size()) - scrollOffset);
    for (int i = 0; i < slotsToDraw; ++i) {
        int actualIndex = scrollOffset + i;
        if (actualIndex < 0 || actualIndex >= saveSlots.size()) continue;

        const auto& currentSlot = saveSlots[actualIndex]; // Get reference to slot info

        // Draw the box background (with selection tint)
        if (nameBoxTexture) {
            SDL_Color tint = (actualIndex == selectedSaveSlotIndex) ? SDL_Color{200, 200, 255, 255} : SDL_Color{255, 255, 255, 255};
            SDL_SetTextureColorMod(nameBoxTexture, tint.r, tint.g, tint.b);
            SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &visibleSaveSlotRects[i]);
            SDL_SetTextureColorMod(nameBoxTexture, 255, 255, 255); // Reset tint
        }

        // --- Render text directly (with dynamic string) ---
        if (saveSlotFont) {
            // 1. Determine text string based on selection and slot type
            std::string textStr;
            if (currentSlot.isNewGameOption) {
                textStr = "+ New Game";
            } else {
                if (actualIndex == selectedSaveSlotIndex) {
                    // Selected: Show Name (placeholder) and Level
                    textStr = currentSlot.playerName + " Lvl " + std::to_string(currentSlot.level);
                } else {
                    // Not Selected: Show Timestamp
                    textStr = currentSlot.timestamp;
                }
            }

        // 2. Determine color based on current selection
        SDL_Color colorToUse = (actualIndex == selectedSaveSlotIndex) ? saveSlotTextColorSelected : saveSlotTextColorDefault;

        // 3. Render text to a temporary texture
        SDL_Surface* surface = TTF_RenderText_Blended(saveSlotFont, textStr.c_str(), colorToUse);
        if (surface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game::renderer, surface);
            if (textTexture) {
                // 4. Calculate scaled text dimensions
                 int w, h;
                 SDL_GetRendererOutputSize(Game::renderer, &w, &h);
                 float scaleX = static_cast<float>(w) / static_cast<float>(referenceWidth);
                 float scaleY = static_cast<float>(h) / static_cast<float>(referenceHeight);
                 float scaleFactor = std::min(scaleX, scaleY);
                 const float minimumScaleFactor = 0.8f;
                 scaleFactor = std::max(scaleFactor, minimumScaleFactor);
                 float saveSlotTextScale = std::max(scaleFactor * 0.9f, minimumScaleFactor * 0.9f); // Use same scale as layout

                SDL_Rect textDestRect;
                // Use surface w/h for original size before scaling
                textDestRect.w = static_cast<int>(surface->w * saveSlotTextScale);
                textDestRect.h = static_cast<int>(surface->h * saveSlotTextScale);
                // Center the text rect within the box rect
                textDestRect.x = visibleSaveSlotRects[i].x + (visibleSaveSlotRects[i].w - textDestRect.w) / 2;
                textDestRect.y = visibleSaveSlotRects[i].y + (visibleSaveSlotRects[i].h - textDestRect.h) / 2;

                // 5. Render the temporary texture
                SDL_RenderCopy(Game::renderer, textTexture, NULL, &textDestRect);
                // 6. Destroy the temporary texture
                SDL_DestroyTexture(textTexture);
            } else {
                 std::cerr << "Failed to create texture for: " << textStr << std::endl;
            }
            SDL_FreeSurface(surface);
        } else {
             std::cerr << "Failed to render text surface for: " << textStr << std::endl;
        }
    }
    // --- END Render text directly ---

} // End loop through visible slots


    // --- Render Scrollbar ---
    bool needsScrollbar = saveSlots.size() > visibleSlotsCount;
    if (needsScrollbar) {
        // Draw Track Background (simple rectangle)
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Game::renderer, 80, 80, 80, 150); // Dark semi-transparent grey
        SDL_RenderFillRect(Game::renderer, &scrollbarTrackRect);
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);

        // Render Button
        if (slideButtonTexture) {
             if (isDraggingScrollbar) SDL_SetTextureColorMod(slideButtonTexture, 200, 200, 200);
             SDL_RenderCopy(Game::renderer, slideButtonTexture, NULL, &scrollbarButtonRect); // No rotation
             if (isDraggingScrollbar) SDL_SetTextureColorMod(slideButtonTexture, 255, 255, 255);
        }
    }

    // --- Render Cursor in Name Box ---
    if (isEditingName && selectedSaveSlotIndex == 0) { // Only show cursor if editing and "+ New Game" selected
        Uint32 ticks = SDL_GetTicks();
        if (ticks % 1000 < 500) { // Blink every 500ms
            SDL_SetRenderDrawColor(Game::renderer, inputTextColor.r, inputTextColor.g, inputTextColor.b, 255);

            // Recalculate scale for cursor size consistency
            int windowWidth, windowHeight;
            SDL_GetRendererOutputSize(Game::renderer, &windowWidth, &windowHeight);
            float scaleX = static_cast<float>(windowWidth) / static_cast<float>(referenceWidth);
            float scaleY = static_cast<float>(windowHeight) / static_cast<float>(referenceHeight);
            float scaleFactor = std::min(scaleX, scaleY);
            const float minimumScaleFactor = 0.8f;
            scaleFactor = std::max(scaleFactor, minimumScaleFactor);
            float textScale = std::max(scaleFactor * 0.8f, minimumScaleFactor * 0.8f);

            int cursorW = static_cast<int>(std::max(1.0f, 2.0f * textScale));
            int cursorPadding = static_cast<int>(std::max(1.0f, 2.0f * textScale));
            int cursorH = nameTextRect.h; // Use height of (potentially scaled) text rect

            int cursorX;
            if (nameTextRect.w > 0) { // Position after text if text exists
                cursorX = nameTextRect.x + nameTextRect.w + cursorPadding;
            } else { // Position at start if text is empty
                // Use nameInputBoxRect for reference, apply padding
                cursorX = nameInputBoxRect.x + (nameInputBoxRect.w / 20);
                cursorH = nameInputBoxRect.h - 2 * cursorPadding; // Adjust height based on box
                cursorH = std::max(1, cursorH); // Ensure positive height
            }
            int cursorY = nameTextRect.y; // Align with text top (or box top if no text?)
            // Adjust Y pos if text was empty (align V center in box)
            if (nameTextRect.w <= 0) {
                cursorY = nameInputBoxRect.y + (nameInputBoxRect.h - cursorH) / 2;
            }


            // Clamp cursor X position
            cursorX = std::min(cursorX, nameInputBoxRect.x + nameInputBoxRect.w - cursorW - cursorPadding);
            cursorX = std::max(nameInputBoxRect.x + cursorPadding, cursorX); // Don't go left of box start


            SDL_Rect cursorRect = {cursorX, cursorY, cursorW, cursorH};
            SDL_RenderFillRect(Game::renderer, &cursorRect);
        }
    }
    // --- Render Volume Controls ---
    SDL_Texture* bgmIconTex = isMusicMuted ? soundOffTexture : soundOnTexture;
    SDL_Texture* sfxIconTex = isSfxMuted ? soundOffTexture : soundOnTexture;

    // Draw BGM Row
    if (bgmIconTex) SDL_RenderCopy(Game::renderer, bgmIconTex, NULL, &bgmIconRect);
    if (sliderTrackTexture) SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &bgmSliderTrackRect);
    if (sliderButtonTexture) SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &bgmSliderButtonRect);

    // Draw SFX Row
    if (sfxIconTex) SDL_RenderCopy(Game::renderer, sfxIconTex, NULL, &sfxIconRect);
    if (sliderTrackTexture) SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &sfxSliderTrackRect);
    if (sliderButtonTexture) SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &sfxSliderButtonRect);

    // --- ADDED: Render Fullscreen Hint ---
    if (fullscreenTextTex) {
        SDL_RenderCopy(Game::renderer, fullscreenTextTex, NULL, &fullscreenTextRect);
    }
    SDL_RenderPresent(Game::renderer);
}


// --- clean (Removed slideBarTexture cleanup) ---
void MenuScene::clean() {
    if (backgroundTexture) { SDL_DestroyTexture(backgroundTexture); backgroundTexture = nullptr; }
    if (titleTexture) { SDL_DestroyTexture(titleTexture); titleTexture = nullptr; }
    if (playButtonTexture) { SDL_DestroyTexture(playButtonTexture); playButtonTexture = nullptr; }
    if (nameBoxTexture) { SDL_DestroyTexture(nameBoxTexture); nameBoxTexture = nullptr; }
    if (nameTextTexture) { SDL_DestroyTexture(nameTextTexture); nameTextTexture = nullptr; }
    // REMOVED: slideBarTexture cleanup
    if (slideButtonTexture) { SDL_DestroyTexture(slideButtonTexture); slideButtonTexture = nullptr; }
    
  

    if (inputFont) { TTF_CloseFont(inputFont); inputFont = nullptr; }
    if (saveSlotFont) { TTF_CloseFont(saveSlotFont); saveSlotFont = nullptr; }
    // --- ADDED: Clean up menu music ---
    Mix_HaltMusic(); // Stop music if playing when scene cleans up
    if (menuMusic) {
        Mix_FreeMusic(menuMusic);
        menuMusic = nullptr;
        std::cout << "Menu BGM freed." << std::endl;
    }
    if (soundOnTexture) { SDL_DestroyTexture(soundOnTexture); soundOnTexture = nullptr; }
    if (soundOffTexture) { SDL_DestroyTexture(soundOffTexture); soundOffTexture = nullptr; }
    if (sliderTrackTexture) { SDL_DestroyTexture(sliderTrackTexture); sliderTrackTexture = nullptr; }
    // --- ADDED: Clean up hint text resources ---
    if (fullscreenTextTex) { SDL_DestroyTexture(fullscreenTextTex); fullscreenTextTex = nullptr; }
    // Close uiHintFont ONLY if it's different from saveSlotFont AND was loaded successfully
    if (uiHintFont && uiHintFont != saveSlotFont) { TTF_CloseFont(uiHintFont); }
    uiHintFont = nullptr; // Set to null regardless
    // --- END ADDED ---

    // --- ADDED: Free MenuScene sounds ---
    if (startSound) { Mix_FreeChunk(startSound); startSound = nullptr; }
    if (clickSound) { Mix_FreeChunk(clickSound); clickSound = nullptr; }
    // --- END ADDED ---

    std::cout << "Menu Scene Cleaned." << std::endl;
}

// --- Helper: Parse Name from Save File ---
std::string MenuScene::parseNameFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) {
        return "Player"; // Default if file cannot be opened
    }
    std::string line;
    std::string key;
    std::string value;
    std::string name = "Player"; // Default name
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerName") { // Check for the key used in SaveLoadManager
            name = value;
            // Handle empty name saved? Use default if empty.
            if (name.empty()) {
                name = "Player";
            }
            break; // Found name, exit loop
        }
    }
    saveFile.close();
    return name;
}