#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../TextureManager.h"
#include "../constants.h"
#include "GameScene.h"
#include "MenuScene.h"
#include "SceneManager.h"

namespace fs = std::filesystem;

MenuScene::MenuScene() {}

MenuScene::~MenuScene() { clean(); }

void MenuScene::init() {
    clean();

    isEditingName = false;
    playerName = "";

    startSound = Mix_LoadWAV("assets/sound/start.wav");
    clickSound = Mix_LoadWAV("assets/sound/buttonclick.wav");
    if (!startSound) {
        std::cerr << "Failed to load start.wav: " << Mix_GetError()
                  << std::endl;
    }
    if (!clickSound) {
        std::cerr << "Failed to load buttonclick.wav: " << Mix_GetError()
                  << std::endl;
    }

    backgroundTexture = loadTexture("assets/menu/bg.png");
    titleTexture = loadTexture("assets/menu/title.png");
    playButtonTexture = loadTexture("assets/menu/play.png");
    nameBoxTexture = loadTexture("assets/menu/box.png");
    soundOnTexture = loadTexture("assets/menu/soundon.png");
    soundOffTexture = loadTexture("assets/menu/soundoff.png");
    sliderTrackTexture = loadTexture("assets/menu/slidebar.png");
    slideButtonTexture = loadTexture("assets/menu/slidebutton.png");
    sliderButtonTexture = loadTexture("assets/menu/slidebutton.png");

    inputFont = TTF_OpenFont("assets/font.ttf", 20);
    saveSlotFont = TTF_OpenFont("assets/font.ttf", 18);
    uiHintFont = TTF_OpenFont("assets/font.ttf", 12);

    if (!inputFont) {
        std::cerr << "Failed to load input font!" << std::endl;
    }
    if (!saveSlotFont) {
        std::cerr << "Failed to load save slot font!" << std::endl;
    }
    if (!uiHintFont) {
        std::cerr
            << "Failed to load hint font! Using save slot font as fallback."
            << std::endl;
        if (saveSlotFont)
            uiHintFont = saveSlotFont;
        else if (inputFont)
            uiHintFont = inputFont;
        else
            std::cerr << "FATAL: No valid font loaded for hints!" << std::endl;
    }

    isMusicMuted = (Game::getMusicVolume() == 0);
    storedMusicVolumeBeforeMute =
        isMusicMuted ? (MIX_MAX_VOLUME / 2) : Game::getMusicVolume();
    isSfxMuted = (Game::getSfxVolume() == 0);
    storedSfxVolumeBeforeMute =
        isSfxMuted ? (MIX_MAX_VOLUME / 2) : Game::getSfxVolume();
    isDraggingBgmSlider = false;
    isDraggingSfxSlider = false;

    loadSaveFiles();

    updateNameTexture();

    if (uiHintFont && Game::renderer) {
        SDL_Color hintColor = {200, 200, 200, 255};
        SDL_Surface* surface =
            TTF_RenderText_Blended(uiHintFont, "F11: Fullscreen", hintColor);
        if (surface) {
            if (fullscreenTextTex) {
                SDL_DestroyTexture(fullscreenTextTex);
            }
            fullscreenTextTex =
                SDL_CreateTextureFromSurface(Game::renderer, surface);
            SDL_FreeSurface(surface);
            if (!fullscreenTextTex) {
                std::cerr
                    << "Failed to create fullscreen hint texture! SDL Error: "
                    << SDL_GetError() << std::endl;
            }
        } else {
            std::cerr << "Failed to render fullscreen hint surface! TTF Error: "
                      << TTF_GetError() << std::endl;
        }
    } else {
        if (!uiHintFont)
            std::cerr << "Cannot render hint text: uiHintFont is null."
                      << std::endl;
        if (!Game::renderer)
            std::cerr << "Cannot render hint text: Game::renderer is null."
                      << std::endl;
    }

    Mix_HaltMusic();
    if (menuMusic) {
        Mix_FreeMusic(menuMusic);
        menuMusic = nullptr;
    }
    menuMusic = Mix_LoadMUS("assets/sound/menubgm.mp3");
    if (menuMusic) {
        if (Mix_PlayMusic(menuMusic, -1) == -1) {
            std::cerr << "Failed to play menu BGM: " << Mix_GetError()
                      << std::endl;
        } else {
            Mix_VolumeMusic(Game::getMusicVolume());
        }
    } else {
        std::cerr << "Failed to load menu BGM!" << std::endl;
    }

    int w = 800, h = 600;
    if (Game::renderer) {
        SDL_GetRendererOutputSize(Game::renderer, &w, &h);
    } else {
        std::cerr
            << "Renderer not available during MenuScene init for layout calc!"
            << std::endl;
    }
    calculateLayout(w, h);
}

void MenuScene::clean() {
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
    if (titleTexture) {
        SDL_DestroyTexture(titleTexture);
        titleTexture = nullptr;
    }
    if (playButtonTexture) {
        SDL_DestroyTexture(playButtonTexture);
        playButtonTexture = nullptr;
    }
    if (nameBoxTexture) {
        SDL_DestroyTexture(nameBoxTexture);
        nameBoxTexture = nullptr;
    }
    if (nameTextTexture) {
        SDL_DestroyTexture(nameTextTexture);
        nameTextTexture = nullptr;
    }
    if (slideButtonTexture) {
        SDL_DestroyTexture(slideButtonTexture);
        slideButtonTexture = nullptr;
    }
    if (sliderButtonTexture) {
        SDL_DestroyTexture(sliderButtonTexture);
        sliderButtonTexture = nullptr;
    }
    if (soundOnTexture) {
        SDL_DestroyTexture(soundOnTexture);
        soundOnTexture = nullptr;
    }
    if (soundOffTexture) {
        SDL_DestroyTexture(soundOffTexture);
        soundOffTexture = nullptr;
    }
    if (sliderTrackTexture) {
        SDL_DestroyTexture(sliderTrackTexture);
        sliderTrackTexture = nullptr;
    }
    if (fullscreenTextTex) {
        SDL_DestroyTexture(fullscreenTextTex);
        fullscreenTextTex = nullptr;
    }

    if (inputFont) {
        TTF_CloseFont(inputFont);
        inputFont = nullptr;
    }
    if (saveSlotFont) {
        TTF_CloseFont(saveSlotFont);
        saveSlotFont = nullptr;
    }

    if (uiHintFont && uiHintFont != saveSlotFont && uiHintFont != inputFont) {
        TTF_CloseFont(uiHintFont);
    }
    uiHintFont = nullptr;

    Mix_HaltMusic();
    if (menuMusic) {
        Mix_FreeMusic(menuMusic);
        menuMusic = nullptr;
    }
    if (startSound) {
        Mix_FreeChunk(startSound);
        startSound = nullptr;
    }
    if (clickSound) {
        Mix_FreeChunk(clickSound);
        clickSound = nullptr;
    }
}

SDL_Texture* MenuScene::loadTexture(const std::string& path) {
    SDL_Texture* newTexture = TextureManager::LoadTexture(path.c_str());
    if (newTexture == nullptr) {
    }
    return newTexture;
}

void MenuScene::updateNameTexture() {
    if (nameTextTexture) {
        SDL_DestroyTexture(nameTextTexture);
        nameTextTexture = nullptr;
    }
    originalNameTextW = 0;
    originalNameTextH = 0;

    if (!inputFont || !Game::renderer) {
        std::cerr << "Cannot update name texture: Font or Renderer is null!"
                  << std::endl;
        return;
    }

    std::string textToRender = playerName;
    SDL_Color colorToUse = inputTextColor;

    if (selectedSaveSlotIndex == 0 && !isEditingName && playerName.empty()) {
        textToRender = "Enter Name (Optional)";
        colorToUse = placeholderTextColor;
    } else if (playerName.empty() && isEditingName) {
        int w = 800, h = 600;
        if (Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
        calculateLayout(w, h);
        return;
    } else if (playerName.empty()) {
        return;
    }

    SDL_Surface* textSurface =
        TTF_RenderText_Blended(inputFont, textToRender.c_str(), colorToUse);
    if (!textSurface) {
        std::cerr << "Unable to render name text surface! SDL_ttf Error: "
                  << TTF_GetError() << std::endl;
        return;
    }

    nameTextTexture = SDL_CreateTextureFromSurface(Game::renderer, textSurface);
    if (!nameTextTexture) {
        std::cerr << "Unable to create name text texture! SDL Error: "
                  << SDL_GetError() << std::endl;
    } else {
        originalNameTextW = textSurface->w;
        originalNameTextH = textSurface->h;
    }
    SDL_FreeSurface(textSurface);

    int w = 800, h = 600;
    if (Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
    calculateLayout(w, h);
}

std::string MenuScene::parseTimestampFromFilename(const std::string& filename) {
    size_t last_slash_idx = filename.find_last_of("\\/");
    std::string name_only = (last_slash_idx == std::string::npos)
                                ? filename
                                : filename.substr(last_slash_idx + 1);
    size_t dot_idx = name_only.rfind(".state");
    if (dot_idx == std::string::npos) return "Invalid Save";

    std::string timestamp_str = name_only.substr(0, dot_idx);

    if (timestamp_str.length() == 15 && timestamp_str[8] == '-' &&
        timestamp_str.find_first_not_of("0123456789-") == std::string::npos) {
        return timestamp_str.substr(0, 2) + "/" + timestamp_str.substr(2, 2) +
               "/" + timestamp_str.substr(4, 4) + " " +
               timestamp_str.substr(9, 2) + ":" + timestamp_str.substr(11, 2);
    }

    return timestamp_str.empty() ? "Invalid Save" : timestamp_str;
}

int MenuScene::parseLevelFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) return 1;
    std::string line, key, value;
    int level = 1;
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerLevel") {
            try {
                level = std::stoi(value);
            } catch (...) {
                level = 1;
            }
            break;
        }
    }
    saveFile.close();
    return std::max(1, level);
}

std::string MenuScene::parseNameFromSaveFile(const std::string& filepath) {
    std::ifstream saveFile(filepath);
    if (!saveFile.is_open()) {
        return "Player";
    }
    std::string line, key, value;
    std::string name = "Player";
    while (std::getline(saveFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;
        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);
        if (key == "PlayerName") {
            name = value;
            if (name.empty()) {
                name = "Player";
            }
            break;
        }
    }
    saveFile.close();
    return name;
}

void MenuScene::loadSaveFiles() {
    saveSlots.clear();
    saveSlots.push_back(SaveSlotInfo{"", 0, "Player", "", true});

    std::string saveDir = "saves";
    std::vector<fs::path> stateFiles;

    try {
        if (!fs::exists(saveDir)) {
            if (!fs::create_directory(saveDir)) {
                std::cerr << "Failed to create 'saves' directory!" << std::endl;
            }
        }

        if (fs::is_directory(saveDir)) {
            for (const auto& entry : fs::directory_iterator(saveDir)) {
                std::string filename_str = entry.path().filename().string();

                if (filename_str == "default.state" ||
                    filename_str == "quicksave.state") {
                    continue;
                }
                if (entry.is_regular_file() &&
                    entry.path().extension() == ".state") {
                    stateFiles.push_back(entry.path());
                }
            }

            std::sort(
                stateFiles.begin(), stateFiles.end(),
                [](const fs::path& a, const fs::path& b) {
                    try {
                        if (!fs::exists(a) || !fs::exists(b)) return false;
                        return fs::last_write_time(a) > fs::last_write_time(b);
                    } catch (const fs::filesystem_error& e) {
                        std::cerr << "Error comparing file times: " << e.what()
                                  << std::endl;
                        return false;
                    }
                });

            for (const auto& path : stateFiles) {
                std::string filename = path.filename().string();
                std::string fullpath = path.string();
                std::replace(fullpath.begin(), fullpath.end(), '\\', '/');

                std::string timestamp = parseTimestampFromFilename(filename);
                int level = parseLevelFromSaveFile(fullpath);
                std::string name = parseNameFromSaveFile(fullpath);

                saveSlots.push_back(
                    SaveSlotInfo{timestamp, level, name, fullpath, false});
            }
        } else {
            std::cerr << "'saves' exists but is not a directory." << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error while reading saves: " << e.what()
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading save files: " << e.what() << std::endl;
    }

    scrollOffset = 0;
    selectedSaveSlotIndex = 0;
}

void MenuScene::handleEvents(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mousePoint = {mouseX, mouseY};

    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_RESIZED) {
        updateNameTexture();
        int w = 800, h = 600;
        if (Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
        calculateLayout(w, h);
    }

    else if (event.type == SDL_MOUSEWHEEL) {
        bool canScroll = static_cast<int>(saveSlots.size()) > visibleSlotsCount;
        SDL_Rect scrollArea = saveListAreaRect;
        if (scrollbarTrackRect.w > 0) {
            scrollArea.w += scrollbarTrackRect.w +
                            (scrollbarTrackRect.x -
                             (saveListAreaRect.x + saveListAreaRect.w));
            scrollArea.h = std::max(scrollArea.h, scrollbarTrackRect.h);
        }

        if (canScroll && SDL_PointInRect(&mousePoint, &scrollArea)) {
            int oldOffset = scrollOffset;
            if (event.wheel.y > 0)
                scrollOffset--;
            else if (event.wheel.y < 0)
                scrollOffset++;

            int maxScrollOffset = std::max(
                0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
            scrollOffset = std::max(0, std::min(scrollOffset, maxScrollOffset));

            if (oldOffset != scrollOffset) {
                int w = 800, h = 600;
                if (Game::renderer)
                    SDL_GetRendererOutputSize(Game::renderer, &w, &h);
                calculateLayout(w, h);
            }
        }
    }

    else if (event.type == SDL_MOUSEBUTTONDOWN &&
             event.button.button == SDL_BUTTON_LEFT) {
        bool playClickSoundFlag = false;

        if (SDL_PointInRect(&mousePoint, &playButtonRect)) {
            if (isEditingName) {
                isEditingName = false;
                SDL_StopTextInput();
                updateNameTexture();
            }
            if (startSound) Mix_PlayChannel(-1, startSound, 0);
            playClickSoundFlag = false;

            if (selectedSaveSlotIndex >= 0 &&
                selectedSaveSlotIndex < static_cast<int>(saveSlots.size())) {
                const auto& selectedSlot = saveSlots[selectedSaveSlotIndex];
                Scene* gameScenePtr =
                    SceneManager::instance->getScene(SceneType::Game);
                GameScene* gameScene = dynamic_cast<GameScene*>(gameScenePtr);

                if (!gameScene) {
                    std::cerr << "Error: Could not get/cast GameScene!"
                              << std::endl;
                    return;
                }

                gameScene->resetGame();
                gameScene->init();

                if (!Game::instance) {
                    std::cerr
                        << "Error: Game::instance is null after GameScene init!"
                        << std::endl;
                    return;
                }

                if (selectedSlot.isNewGameOption) {
                    Game::instance->setPlayerName(this->playerName);
                } else {
                    if (Game::instance->saveLoadManager) {
                        if (!Game::instance->saveLoadManager->loadGameState(
                                selectedSlot.filename)) {
                            std::cerr << "Failed to load game state: "
                                      << selectedSlot.filename << std::endl;
                            return;
                        }
                        this->playerName = Game::instance->getPlayerName();
                        updateNameTexture();
                    } else {
                        std::cerr
                            << "Critical Error: SaveLoadManager not available!"
                            << std::endl;
                        return;
                    }
                }
                SceneManager::instance->switchToScene(SceneType::Game);
            } else {
                std::cerr << "Error: Invalid selected save slot index."
                          << std::endl;
            }
            return;

        } else if (SDL_PointInRect(&mousePoint, &bgmIconRect)) {
            bool wasMuted = (Game::getMusicVolume() == 0);
            if (!wasMuted) {
                storedMusicVolumeBeforeMute = Game::getMusicVolume();
                Game::setMusicVolume(0);
            } else {
                Game::setMusicVolume(storedMusicVolumeBeforeMute > 0
                                         ? storedMusicVolumeBeforeMute
                                         : MIX_MAX_VOLUME / 4);
                storedMusicVolumeBeforeMute = Game::getMusicVolume();
            }
            isMusicMuted = (Game::getMusicVolume() == 0);
            if (menuMusic) Mix_VolumeMusic(Game::getMusicVolume());
            playClickSoundFlag = true;

        } else if (SDL_PointInRect(&mousePoint, &sfxIconRect)) {
            bool wasMuted = (Game::getSfxVolume() == 0);
            if (!wasMuted) {
                storedSfxVolumeBeforeMute = Game::getSfxVolume();
                Game::setSfxVolume(0);
            } else {
                Game::setSfxVolume(storedSfxVolumeBeforeMute > 0
                                       ? storedSfxVolumeBeforeMute
                                       : MIX_MAX_VOLUME / 4);
                storedSfxVolumeBeforeMute = Game::getSfxVolume();
            }
            isSfxMuted = (Game::getSfxVolume() == 0);
            playClickSoundFlag = true;

        } else if (SDL_PointInRect(&mousePoint, &bgmSliderButtonRect)) {
            isDraggingBgmSlider = true;
            sliderDragStartX = mouseX - bgmSliderButtonRect.x;
            playClickSoundFlag = true;
        } else if (SDL_PointInRect(&mousePoint, &sfxSliderButtonRect)) {
            isDraggingSfxSlider = true;
            sliderDragStartX = mouseX - sfxSliderButtonRect.x;
            playClickSoundFlag = true;
        } else if (static_cast<int>(saveSlots.size()) > visibleSlotsCount &&
                   SDL_PointInRect(&mousePoint, &scrollbarButtonRect)) {
            if (!isDraggingScrollbar) {
                isDraggingScrollbar = true;
                scrollbarDragStartY = mouseY - scrollbarButtonRect.y;
                playClickSoundFlag = true;
            }
        } else if (SDL_PointInRect(&mousePoint, &nameInputBoxRect)) {
            if (!isEditingName && selectedSaveSlotIndex == 0) {
                isEditingName = true;
                SDL_StartTextInput();
                updateNameTexture();
                playClickSoundFlag = true;
            }
        } else {
            bool clickedOnSlot = false;
            for (int i = 0; i < visibleSlotsCount; ++i) {
                int actualIndex = scrollOffset + i;
                if (actualIndex >= 0 &&
                    actualIndex < static_cast<int>(saveSlots.size())) {
                    if (SDL_PointInRect(&mousePoint,
                                        &visibleSaveSlotRects[i])) {
                        if (selectedSaveSlotIndex != actualIndex) {
                            selectedSaveSlotIndex = actualIndex;
                            if (saveSlots[actualIndex].isNewGameOption) {
                                playerName = "";
                                if (!isEditingName) {
                                    isEditingName = true;
                                    SDL_StartTextInput();
                                }
                            } else {
                                if (isEditingName) {
                                    isEditingName = false;
                                    SDL_StopTextInput();
                                }
                                playerName = saveSlots[actualIndex].playerName;
                            }
                            updateNameTexture();
                        }
                        playClickSoundFlag = true;
                        clickedOnSlot = true;
                        break;
                    }
                }
            }

            if (!clickedOnSlot && isEditingName) {
                isEditingName = false;
                SDL_StopTextInput();
                updateNameTexture();
            }
        }

        if (playClickSoundFlag && clickSound) {
            Mix_PlayChannel(-1, clickSound, 0);
        }

        if (playClickSoundFlag) {
            int w = 800, h = 600;
            if (Game::renderer)
                SDL_GetRendererOutputSize(Game::renderer, &w, &h);
            calculateLayout(w, h);
        }
    }

    else if (event.type == SDL_MOUSEBUTTONUP &&
             event.button.button == SDL_BUTTON_LEFT) {
        if (isDraggingScrollbar || isDraggingBgmSlider || isDraggingSfxSlider) {
            isDraggingScrollbar = false;
            isDraggingBgmSlider = false;
            isDraggingSfxSlider = false;
        }
    }

    else if (event.type == SDL_MOUSEMOTION) {
        int w = 800, h = 600;
        if (Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h);
        if (isDraggingBgmSlider) {
            int trackX = bgmSliderTrackRect.x;
            int trackButtonW = bgmSliderButtonRect.w;
            int trackW = std::max(1, bgmSliderTrackRect.w - trackButtonW);
            int targetButtonX = std::max(
                trackX, std::min(mouseX - sliderDragStartX, trackX + trackW));
            float percent =
                (trackW > 0)
                    ? static_cast<float>(targetButtonX - trackX) / trackW
                    : 0.0f;
            int newVolume =
                static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
            if (newVolume != Game::getMusicVolume()) {
                Game::setMusicVolume(newVolume);
                isMusicMuted = (newVolume == 0);
                if (!isMusicMuted) storedMusicVolumeBeforeMute = newVolume;
                if (menuMusic) {
                    Mix_VolumeMusic(newVolume);
                }
                calculateLayout(w, h);
            }
        } else if (isDraggingSfxSlider) {
            int trackX = sfxSliderTrackRect.x;
            int trackButtonW = sfxSliderButtonRect.w;
            int trackW = std::max(1, sfxSliderTrackRect.w - trackButtonW);
            int targetButtonX = std::max(
                trackX, std::min(mouseX - sliderDragStartX, trackX + trackW));
            float percent =
                (trackW > 0)
                    ? static_cast<float>(targetButtonX - trackX) / trackW
                    : 0.0f;
            int newVolume =
                static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
            if (newVolume != Game::getSfxVolume()) {
                Game::setSfxVolume(newVolume);
                isSfxMuted = (newVolume == 0);
                if (!isSfxMuted) {
                    storedSfxVolumeBeforeMute = newVolume;
                }
                calculateLayout(w, h);
            }
        } else if (isDraggingScrollbar) {
            int scrollTrackHeight =
                std::max(1, scrollbarTrackRect.h - scrollbarButtonRect.h);
            int targetButtonTop =
                std::max(scrollbarTrackRect.y,
                         std::min(mouseY - scrollbarDragStartY,
                                  scrollbarTrackRect.y + scrollTrackHeight));
            float percentageScrolled =
                (scrollTrackHeight > 0)
                    ? static_cast<float>(targetButtonTop -
                                         scrollbarTrackRect.y) /
                          scrollTrackHeight
                    : 0.0f;
            int maxScrollOffset = std::max(
                0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
            int newOffset = static_cast<int>(
                std::round(percentageScrolled * maxScrollOffset));
            if (newOffset != scrollOffset) {
                scrollOffset = newOffset;
                calculateLayout(w, h);
            }
        }
    }

    else if (event.type == SDL_TEXTINPUT && isEditingName) {
        const int MAX_NAME_LENGTH = 15;
        if (playerName.length() < MAX_NAME_LENGTH) {
            playerName += event.text.text;
            updateNameTexture();
        }
    }

    else if (event.type == SDL_KEYDOWN) {
        if (isEditingName) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
                playerName.pop_back();
                updateNameTexture();
            } else if (event.key.keysym.sym == SDLK_RETURN ||
                       event.key.keysym.sym == SDLK_KP_ENTER) {
                isEditingName = false;
                SDL_StopTextInput();
                updateNameTexture();
            } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                isEditingName = false;
                SDL_StopTextInput();
                playerName = "";
                updateNameTexture();
            }
        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }
    }
}

void MenuScene::update() {}

void MenuScene::render() {
    if (!Game::renderer) {
        return;
    }
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);
    SDL_RenderClear(Game::renderer);

    int windowWidth = 800, windowHeight = 600; 
    if (Game::renderer) {
        SDL_GetRendererOutputSize(Game::renderer, &windowWidth, &windowHeight);
    }

    if (backgroundTexture)
        SDL_RenderCopy(Game::renderer, backgroundTexture, NULL, &backgroundRect);
    if (titleTexture)
        SDL_RenderCopy(Game::renderer, titleTexture, NULL, &titleRect);
    if (playButtonTexture)
        SDL_RenderCopy(Game::renderer, playButtonTexture, NULL, &playButtonRect);
    if (nameBoxTexture)
        SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &nameInputBoxRect);
    if (nameTextTexture)
        SDL_RenderCopy(Game::renderer, nameTextTexture, NULL, &nameTextRect);

    int slotsToDraw = std::min(
        visibleSlotsCount, static_cast<int>(saveSlots.size()) - scrollOffset);
    int paddingX = std::max(5, static_cast<int>(windowWidth * 0.02f)); 
    int textPaddingX = std::max(3, paddingX / 3); 

    for (int i = 0; i < slotsToDraw; ++i) {
        int actualIndex = scrollOffset + i;
        if (actualIndex < 0 || actualIndex >= static_cast<int>(saveSlots.size()))
            continue;

        const auto& currentSlot = saveSlots[actualIndex];
        SDL_Rect currentSlotRect = visibleSaveSlotRects[i]; 

        if (nameBoxTexture) {
            SDL_Color tint = (actualIndex == selectedSaveSlotIndex)
                                 ? SDL_Color{200, 200, 255, 255} 
                                 : SDL_Color{255, 255, 255, 255}; 
            SDL_SetTextureColorMod(nameBoxTexture, tint.r, tint.g, tint.b);
            SDL_RenderCopy(Game::renderer, nameBoxTexture, NULL, &currentSlotRect);
            SDL_SetTextureColorMod(nameBoxTexture, 255, 255, 255); 
        }

        if (saveSlotFont) {

            std::string baseTextStr =
                currentSlot.isNewGameOption
                    ? "+ New Game"
                    : (actualIndex == selectedSaveSlotIndex
                           ? (currentSlot.playerName + " Lvl " + std::to_string(currentSlot.level))
                           : currentSlot.timestamp);

            std::string textStrToRender = baseTextStr;
            int maxTextWidth = currentSlotRect.w - 2 * textPaddingX; 

            float scaleX = static_cast<float>(windowWidth) / referenceWidth;
            float scaleY = static_cast<float>(windowHeight) / referenceHeight;
            float scaleFactor = std::max(std::min(scaleX, scaleY), 0.8f);
            float saveSlotTextScale = std::max(scaleFactor * 0.9f, 0.72f);

            int textW_unscaled = 0, textH_unscaled = 0;

            if (TTF_SizeText(saveSlotFont, textStrToRender.c_str(), &textW_unscaled, &textH_unscaled) == 0) {
                int originalScaledWidth = static_cast<int>(textW_unscaled * saveSlotTextScale);

                if (originalScaledWidth > maxTextWidth && maxTextWidth > 0) {

                    while (textStrToRender.length() > 0) {

                        if (textStrToRender.length() <= 3 && (textStrToRender == "..." || textStrToRender.length() < 3) ) {
                             textStrToRender = "...";
                             break;
                        }
                        textStrToRender.pop_back(); 
                        std::string tempStr = textStrToRender + "...";

                        if (TTF_SizeText(saveSlotFont, tempStr.c_str(), &textW_unscaled, &textH_unscaled) == 0) {
                            int currentScaledWidth = static_cast<int>(textW_unscaled * saveSlotTextScale);
                            if (currentScaledWidth <= maxTextWidth) {
                                textStrToRender = tempStr; 
                                break;
                            }
                        } else {
                             textStrToRender = "..."; 
                             break;
                        }

                        if (textStrToRender.empty()) {
                             textStrToRender = "...";
                        }
                    }
                }

            } else {
                 textStrToRender = "?"; 
                 std::cerr << "Failed to get size for save slot text: " << baseTextStr << std::endl; 
            }

            SDL_Color colorToUse = (actualIndex == selectedSaveSlotIndex)
                                       ? saveSlotTextColorSelected
                                       : saveSlotTextColorDefault;

            if (actualIndex == selectedSaveSlotIndex) {

                SDL_Color outlineColor = {0, 0, 0, 255};
                int outlineWidth = 1;

                SDL_Surface* outlineSurface = TTF_RenderText_Blended(saveSlotFont, textStrToRender.c_str(), outlineColor);
                SDL_Texture* outlineTexture = nullptr;
                if (outlineSurface) {
                    outlineTexture = SDL_CreateTextureFromSurface(Game::renderer, outlineSurface);
                    SDL_FreeSurface(outlineSurface);
                } else {
                    std::cerr << "Failed to render outline text surface for save slot: " << textStrToRender << std::endl; 
                }

                SDL_Surface* mainSurface = TTF_RenderText_Blended(saveSlotFont, textStrToRender.c_str(), colorToUse);
                SDL_Texture* mainTexture = nullptr;
                SDL_Rect textDestRect = {0,0,0,0}; 
                if (mainSurface) {
                     mainTexture = SDL_CreateTextureFromSurface(Game::renderer, mainSurface);
                    if (mainTexture) {

                        textDestRect.w = static_cast<int>(mainSurface->w * saveSlotTextScale);
                        textDestRect.h = static_cast<int>(mainSurface->h * saveSlotTextScale);
                        textDestRect.x = currentSlotRect.x + (currentSlotRect.w - textDestRect.w) / 2;
                        textDestRect.y = currentSlotRect.y + (currentSlotRect.h - textDestRect.h) / 2;
                    } else {
                         std::cerr << "Failed to create main texture for save slot text: " << textStrToRender << std::endl; 
                    }
                     SDL_FreeSurface(mainSurface);
                } else {
                    std::cerr << "Failed to render main text surface for save slot: " << textStrToRender << std::endl; 
                }

                if (outlineTexture && textDestRect.w > 0) {
                    for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
                        for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
                            if (dx == 0 && dy == 0) continue;
                            SDL_Rect outlineDest = textDestRect;
                            outlineDest.x += dx;
                            outlineDest.y += dy;
                            SDL_RenderCopy(Game::renderer, outlineTexture, NULL, &outlineDest);
                        }
                    }
                     SDL_DestroyTexture(outlineTexture);
                }

                if (mainTexture && textDestRect.w > 0) {
                    SDL_RenderCopy(Game::renderer, mainTexture, NULL, &textDestRect);
                    SDL_DestroyTexture(mainTexture);
                }

            } else {

                 SDL_Surface* surface = TTF_RenderText_Blended(saveSlotFont, textStrToRender.c_str(), colorToUse);
                 if (surface) {
                     SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game::renderer, surface);
                     if (textTexture) {
                         SDL_Rect textDestRect;

                         textDestRect.w = static_cast<int>(surface->w * saveSlotTextScale);
                         textDestRect.h = static_cast<int>(surface->h * saveSlotTextScale);
                         textDestRect.x = currentSlotRect.x + (currentSlotRect.w - textDestRect.w) / 2;
                         textDestRect.y = currentSlotRect.y + (currentSlotRect.h - textDestRect.h) / 2;

                         SDL_RenderCopy(Game::renderer, textTexture, NULL, &textDestRect);
                         SDL_DestroyTexture(textTexture);
                     } else {
                          std::cerr << "Failed to create texture for save slot text: " << textStrToRender << std::endl; 
                     }
                      SDL_FreeSurface(surface);
                 } else {
                      std::cerr << "Failed to render text surface for save slot: " << textStrToRender << std::endl; 
                 }
            }

        } 
    } 

    bool needsScrollbar = static_cast<int>(saveSlots.size()) > visibleSlotsCount;
    if (needsScrollbar) {
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Game::renderer, 80, 80, 80, 150);
        SDL_RenderFillRect(Game::renderer, &scrollbarTrackRect);
        SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);

        if (slideButtonTexture) { 
            SDL_Color tint = isDraggingScrollbar
                                 ? SDL_Color{200, 200, 200, 255}
                                 : SDL_Color{255, 255, 255, 255};
            SDL_SetTextureColorMod(slideButtonTexture, tint.r, tint.g, tint.b);
            SDL_RenderCopy(Game::renderer, slideButtonTexture, NULL, &scrollbarButtonRect);
            SDL_SetTextureColorMod(slideButtonTexture, 255, 255, 255);
        }
    }

    if (isEditingName && selectedSaveSlotIndex == 0) {
        Uint32 ticks = SDL_GetTicks();
        if (ticks % 1000 < 500) { 
            SDL_SetRenderDrawColor(Game::renderer, inputTextColor.r,
                                   inputTextColor.g, inputTextColor.b, 255);

            float scaleX_cursor = static_cast<float>(windowWidth) / referenceWidth;
            float scaleY_cursor = static_cast<float>(windowHeight) / referenceHeight;
            float scaleFactor_cursor = std::max(std::min(scaleX_cursor, scaleY_cursor), 0.8f);
            float textScale_cursor = std::max(scaleFactor_cursor * 0.8f, 0.64f);

            int cursorW = std::max(1, static_cast<int>(2.0f * textScale_cursor));
            int cursorPadding = std::max(1, static_cast<int>(2.0f * textScale_cursor));
            int cursorH = nameTextRect.h > 0 ? nameTextRect.h : std::max(1, nameInputBoxRect.h - 2 * cursorPadding);

            int cursorX;
            if (nameTextRect.w > 0) { 
                cursorX = nameTextRect.x + nameTextRect.w + cursorPadding / 2; 
            } else { 
                cursorX = nameInputBoxRect.x + textPaddingX; 
            }

            int cursorY = (nameTextRect.h > 0) ? nameTextRect.y : (nameInputBoxRect.y + (nameInputBoxRect.h - cursorH) / 2);

            cursorX = std::min(cursorX, nameInputBoxRect.x + nameInputBoxRect.w - cursorW - textPaddingX);
            cursorX = std::max(nameInputBoxRect.x + textPaddingX, cursorX);

            SDL_Rect cursorRect = {cursorX, cursorY, cursorW, cursorH};
            SDL_RenderFillRect(Game::renderer, &cursorRect);
        }
    }

    SDL_Texture* bgmIconTex = isMusicMuted ? soundOffTexture : soundOnTexture;
    SDL_Texture* sfxIconTex = isSfxMuted ? soundOffTexture : soundOnTexture;

    if (bgmIconTex) SDL_RenderCopy(Game::renderer, bgmIconTex, NULL, &bgmIconRect);
    if (sfxIconTex) SDL_RenderCopy(Game::renderer, sfxIconTex, NULL, &sfxIconRect);
    if (sliderTrackTexture) {
        SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &bgmSliderTrackRect);
        SDL_RenderCopy(Game::renderer, sliderTrackTexture, NULL, &sfxSliderTrackRect);
    }
    if (sliderButtonTexture) { 
        SDL_Color bgmTint = isDraggingBgmSlider ? SDL_Color{200, 200, 200, 255} : SDL_Color{255, 255, 255, 255};
        SDL_SetTextureColorMod(sliderButtonTexture, bgmTint.r, bgmTint.g, bgmTint.b);
        SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &bgmSliderButtonRect);

        SDL_Color sfxTint = isDraggingSfxSlider ? SDL_Color{200, 200, 200, 255} : SDL_Color{255, 255, 255, 255};
        SDL_SetTextureColorMod(sliderButtonTexture, sfxTint.r, sfxTint.g, sfxTint.b);
        SDL_RenderCopy(Game::renderer, sliderButtonTexture, NULL, &sfxSliderButtonRect);

        SDL_SetTextureColorMod(sliderButtonTexture, 255, 255, 255); 
    }

    if (fullscreenTextTex) {
        SDL_RenderCopy(Game::renderer, fullscreenTextTex, NULL, &fullscreenTextRect);
    }

    SDL_RenderPresent(Game::renderer);
}

void MenuScene::calculateLayout(int windowWidth, int windowHeight) {
    backgroundRect = {0, 0, windowWidth, windowHeight};

    float scaleX =
        static_cast<float>(windowWidth) / static_cast<float>(referenceWidth);
    float scaleY =
        static_cast<float>(windowHeight) / static_cast<float>(referenceHeight);

    float scaleFactor = std::min(scaleX, scaleY) + 0.5f;

    int effectiveWidth = static_cast<int>(referenceWidth * scaleFactor);
    int effectiveHeight = static_cast<int>(referenceHeight * scaleFactor);

    int offsetX = (windowWidth - effectiveWidth) / 2;
    int offsetY = (windowHeight - effectiveHeight) / 2;

    int paddingX = std::max(5, static_cast<int>(windowWidth * 0.02f));
    int paddingY = std::max(5, static_cast<int>(windowHeight * 0.02f));

    float textScale = scaleFactor * 0.9f;

    titleRect.w = static_cast<int>(originalTitleWidth * scaleFactor);
    titleRect.h = static_cast<int>(originalTitleHeight * scaleFactor);

    titleRect.x = (windowWidth - titleRect.w) / 2;
    titleRect.y = paddingY * 2;

    playButtonRect.w = static_cast<int>(originalPlayButtonWidth * scaleFactor);
    playButtonRect.h = static_cast<int>(originalPlayButtonHeight * scaleFactor);
    playButtonRect.x = (windowWidth - playButtonRect.w) / 2;
    playButtonRect.y = titleRect.y + titleRect.h + paddingY * 3;

    nameInputBoxRect.w =
        static_cast<int>(originalNameBoxWidth * scaleFactor * 1.5f);
    nameInputBoxRect.h =
        static_cast<int>(originalNameBoxHeight * scaleFactor * 1.2f);
    nameInputBoxRect.x = (windowWidth - nameInputBoxRect.w) / 2;
    nameInputBoxRect.y = playButtonRect.y + playButtonRect.h + paddingY * 3;

    int currentTextW = originalNameTextW;
    int currentTextH = originalNameTextH;
    int scaledTextW = static_cast<int>(currentTextW * textScale);
    int scaledTextH = static_cast<int>(currentTextH * textScale);

    scaledTextW = std::min(scaledTextW, nameInputBoxRect.w - paddingX / 2);
    scaledTextH = std::min(scaledTextH, nameInputBoxRect.h - paddingY / 2);
    nameTextRect.x =
        nameInputBoxRect.x + (nameInputBoxRect.w - scaledTextW) / 2;
    nameTextRect.y =
        nameInputBoxRect.y + (nameInputBoxRect.h - scaledTextH) / 2;
    nameTextRect.w = scaledTextW;
    nameTextRect.h = scaledTextH;

    int listWidth = static_cast<int>(windowWidth * 0.2f);
    int listHeight = static_cast<int>(windowHeight * 0.2f);

    int listY = (windowHeight - listHeight) / 2;

    int targetListCenterX =
        (nameInputBoxRect.x) / 2 + 80 * (scaleFactor - 0.5f);
    int listX = targetListCenterX - listWidth / 2;

    listX = std::max(paddingX, listX);

    listWidth = std::min(listWidth, nameInputBoxRect.x - listX - paddingX);
    listWidth = std::max(50, listWidth);

    saveListAreaRect = {listX, listY, listWidth, listHeight};

    visibleSaveSlotRects.resize(visibleSlotsCount);
    float slotHeight = (visibleSlotsCount > 0)
                           ? static_cast<float>(listHeight) / visibleSlotsCount
                           : 0;
    float slotPadding = slotHeight * 0.1f;
    float actualSlotHeight = std::max(0.0f, slotHeight - slotPadding);

    for (int i = 0; i < visibleSlotsCount; ++i) {
        int scaledBoxW = static_cast<int>(
            (originalNameBoxWidth * (scaleFactor - 0.5f)) * 0.9f);
        int scaledBoxH = static_cast<int>(
            (originalNameBoxHeight * (scaleFactor - 0.5f)) * 1.0f);

        scaledBoxW = std::min(scaledBoxW, listWidth - paddingX / 2);
        scaledBoxH = std::min(scaledBoxH, static_cast<int>(actualSlotHeight));

        visibleSaveSlotRects[i].x = listX + (listWidth - scaledBoxW) / 2;
        visibleSaveSlotRects[i].y =
            listY +
            static_cast<int>(i * slotHeight + (slotHeight - scaledBoxH) / 2.0f);
        visibleSaveSlotRects[i].w = scaledBoxW * 1.8f;
        visibleSaveSlotRects[i].h = scaledBoxH * 2.4f;
    }

    if (static_cast<int>(saveSlots.size()) > visibleSlotsCount) {
        int scrollbarWidth = std::max(
            5, static_cast<int>(originalSlideButtonWidth * scaleFactor * 1.5f));
        int scrollbarX = listX + listWidth + paddingX / 2;

        scrollbarX = std::min(
            scrollbarX, nameInputBoxRect.x - scrollbarWidth - paddingX / 2);

        scrollbarTrackRect.x = scrollbarX;
        scrollbarTrackRect.y = listY;
        scrollbarTrackRect.w = scrollbarWidth;
        scrollbarTrackRect.h = listHeight;

        int scrollButtonWidth = std::max(
            3, static_cast<int>(originalSlideButtonWidth * scaleFactor));
        int scrollButtonHeight = std::max(
            8,
            static_cast<int>(originalSlideButtonHeight * scaleFactor * 1.5f));
        scrollButtonHeight = std::min(scrollButtonHeight, scrollbarTrackRect.h);
        int scrollTrackTravelHeight =
            std::max(0, scrollbarTrackRect.h - scrollButtonHeight);

        int maxScrollOffset =
            std::max(0, static_cast<int>(saveSlots.size()) - visibleSlotsCount);
        float percentageScrolled =
            (maxScrollOffset == 0)
                ? 0.0f
                : static_cast<float>(scrollOffset) / maxScrollOffset;

        scrollbarButtonRect.w = scrollButtonWidth;
        scrollbarButtonRect.h = scrollButtonHeight;
        scrollbarButtonRect.x =
            scrollbarTrackRect.x +
            (scrollbarTrackRect.w - scrollbarButtonRect.w) / 2;
        scrollbarButtonRect.y =
            scrollbarTrackRect.y +
            static_cast<int>(percentageScrolled * scrollTrackTravelHeight);
    } else {
        scrollbarTrackRect = {0, 0, 0, 0};
        scrollbarButtonRect = {0, 0, 0, 0};
    }

    int iconSize = std::max(20, static_cast<int>(windowHeight * 0.04f));
    int sliderHeight = std::max(
        3, static_cast<int>(originalSlidebarHeight * scaleFactor * 1.5f));
    int sliderButtonWidth = std::max(
        5, static_cast<int>(originalSlideButtonWidth * scaleFactor * 1.5f));
    int sliderButtonHeight = std::max(
        8, static_cast<int>(originalSlideButtonHeight * scaleFactor * 1.2f));
    int sliderWidth = std::max(50, static_cast<int>(windowWidth * 0.15f));
    int controlPadding = paddingX / 2;

    int rowHeight = iconSize + controlPadding;
    int controlAreaHeight = rowHeight * 2 + controlPadding;
    int controlAreaWidth = iconSize + controlPadding + sliderWidth;

    int controlAreaX = windowWidth - controlAreaWidth - paddingX;
    int controlAreaY = windowHeight - controlAreaHeight - paddingY;

    int bgmRowY = controlAreaY;
    bgmIconRect = {controlAreaX, bgmRowY + (rowHeight - iconSize) / 2, iconSize,
                   iconSize};
    bgmSliderTrackRect = {bgmIconRect.x + iconSize + controlPadding,
                          bgmRowY + (rowHeight - sliderHeight) / 2, sliderWidth,
                          sliderHeight};
    int bgmSliderTrackWidth =
        std::max(1, bgmSliderTrackRect.w - sliderButtonWidth);
    float bgmPercent =
        (MIX_MAX_VOLUME == 0)
            ? 0.0f
            : static_cast<float>(Game::getMusicVolume()) / MIX_MAX_VOLUME;
    bgmSliderButtonRect = {
        bgmSliderTrackRect.x +
            static_cast<int>(bgmPercent * bgmSliderTrackWidth),
        bgmSliderTrackRect.y + (sliderHeight - sliderButtonHeight) / 2,
        sliderButtonWidth, sliderButtonHeight};

    int sfxRowY = controlAreaY + rowHeight;
    sfxIconRect = {controlAreaX, sfxRowY + (rowHeight - iconSize) / 2, iconSize,
                   iconSize};
    sfxSliderTrackRect = {sfxIconRect.x + iconSize + controlPadding,
                          sfxRowY + (rowHeight - sliderHeight) / 2, sliderWidth,
                          sliderHeight};
    int sfxSliderTrackWidth =
        std::max(1, sfxSliderTrackRect.w - sliderButtonWidth);
    float sfxPercent =
        (MIX_MAX_VOLUME == 0)
            ? 0.0f
            : static_cast<float>(Game::getSfxVolume()) / MIX_MAX_VOLUME;
    sfxSliderButtonRect = {
        sfxSliderTrackRect.x +
            static_cast<int>(sfxPercent * sfxSliderTrackWidth),
        sfxSliderTrackRect.y + (sliderHeight - sliderButtonHeight) / 2,
        sliderButtonWidth, sliderButtonHeight};

    if (fullscreenTextTex) {
        SDL_QueryTexture(fullscreenTextTex, NULL, NULL, &fullscreenTextRect.w,
                         &fullscreenTextRect.h);

        fullscreenTextRect.x =
            controlAreaX + controlAreaWidth - fullscreenTextRect.w;
        fullscreenTextRect.y =
            controlAreaY - fullscreenTextRect.h - paddingY / 2;

        fullscreenTextRect.x =
            std::max(offsetX + paddingX, fullscreenTextRect.x);
        fullscreenTextRect.y =
            std::max(offsetY + paddingY, fullscreenTextRect.y);
    } else {
        fullscreenTextRect = {0, 0, 0, 0};
    }
}