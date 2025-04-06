#include "game.h"
#include <iostream>
#include <SDL.h>
#include "constants.h"
#include <SDL_image.h>
#include "TextureManager.h"
#include "map.h"
#include "ECS/Components.h"
#include "Vector2D.h"
#include "Collision.h"
#include "AssetManager.h"
#include "ECS/EnemyAi.h"
#include <ctime>
#include <vector>
#include <cstdlib>
#include <sstream>
#include "SaveLoadManager.h"

Game* Game::instance = nullptr;
SDL_Event Game::event;
SDL_Renderer *Game::renderer = nullptr;
int Game::mouseX = 0;
int Game::mouseY = 0;
int Game::musicVolume = MIX_MAX_VOLUME / 2;
int Game::sfxVolume = MIX_MAX_VOLUME / 2;
SDL_Rect Game::camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
bool Game::isRunning = false;

Game::Game() {
    if (instance != nullptr) {
        std::cerr << "Warning: Multiple Game instances detected!" << std::endl;
    }
    instance = this;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    playerEntity = nullptr;
    playerManager = nullptr;
    ui = nullptr;
    map = nullptr;
    assets = nullptr;
    saveLoadManager = nullptr;

    assets = new AssetManager(&manager);
    saveLoadManager = new SaveLoadManager(this);
}

Game::~Game(){
    std::cout << "Game destructor (~Game) called." << std::endl;
    clean();
    instance = nullptr;
}

void Game::init(){
    if (!Game::renderer) {
        std::cerr << "Error: Game::init called but Game::renderer is null!" << std::endl;
        isRunning = false;
        if(saveLoadManager) { delete saveLoadManager; saveLoadManager = nullptr; }
        if(assets) { delete assets; assets = nullptr; }
        return;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    currentState = GameState::Playing;

    Mix_VolumeMusic(musicVolume);
    Mix_Volume(-1, sfxVolume);
    manager.refresh();

    initializeEnemyDatabase();

    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);

    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");
    assets->AddTexture("fire", "sprites/projectile/fire.png");
    assets->AddTexture("starproj", "sprites/projectile/star.png");

    assets->AddTexture("exp_orb_1", "sprites/projectile/exp_orb1.png");
    assets->AddTexture("exp_orb_10", "sprites/projectile/exp_orb10.png");
    assets->AddTexture("exp_orb_50", "sprites/projectile/exp_orb50.png");
    assets->AddTexture("exp_orb_100", "sprites/projectile/exp_orb100.png");
    assets->AddTexture("exp_orb_200", "sprites/projectile/exp_orb200.png");
    assets->AddTexture("exp_orb_500", "sprites/projectile/exp_orb500.png");
    assets->AddTexture("boss_walk", bossWalkSprite);
    assets->AddTexture("boss_charge", bossChargeSprite);
    assets->AddTexture("boss_slam", bossSlamSprite);
    assets->AddTexture("boss_projectile", bossProjectileSprite);

    assets->AddSoundEffect("gunshot_sound", "assets/sound/shot.wav");
    assets->AddMusic("level_music", "assets/sound/hlcbg.mp3");
    assets->AddSoundEffect("fire_spell_sound", "assets/sound/fire.wav");
    assets->AddSoundEffect("star_spell_sound", "assets/sound/star.wav");

    assets->AddTexture("pausebox", "assets/menu/pausebox.png");
    assets->AddTexture("buttonbox", "assets/menu/box.png");
    assets->AddTexture("soundon", "assets/menu/soundon.png");
    assets->AddTexture("soundoff", "assets/menu/soundoff.png");
    assets->AddTexture("slidebar", "assets/menu/slidebar.png");
    assets->AddTexture("slidebutton", "assets/menu/slidebutton.png");
    assets->AddTexture("gameover", "assets/menu/gameover.png");
    assets->AddSoundEffect("gameover_sfx", "assets/sound/gameover.wav");
    assets->AddSoundEffect("game_start", "assets/sound/start.wav");
    assets->AddSoundEffect("button_click", "assets/sound/buttonclick.wav");
    assets->AddTexture("weapon_icon", "assets/menu/weaponicon.png");
    assets->AddTexture("fire_icon", "assets/menu/fireicon.png");
    assets->AddTexture("star_icon", "assets/menu/staricon.png");
    assets->AddTexture("health_icon", "assets/menu/healthicon.png");
    assets->AddTexture("lifesteal_icon", "assets/menu/lifestealicon.png");

    int enemyTexFailed = 0;
    for (const auto& enemyData : allEnemyDatabase) {
        if (!enemyData.sprite) continue;
        SDL_Texture* loadedTexture = TextureManager::LoadTexture(enemyData.sprite);
        if (loadedTexture == nullptr) {
            enemyTexFailed++;
            std::cerr << "ERROR: Failed to load texture for enemy: '" << enemyData.tag << "' path: " << enemyData.sprite << std::endl;
        } else {
            assets->AddTexture(enemyData.tag, enemyData.sprite); 
            SDL_DestroyTexture(loadedTexture); 
        }
    }
    if (enemyTexFailed > 0) {
         std::cerr << "!!! ERROR: Failed to load one or more enemy textures. Check paths in constants.h and file existence. !!!" << std::endl;
    }

    delete ui;
    ui = new UIManager(renderer);
    if (ui) {
        ui->init();
    } else {
        std::cerr << "Error: Failed to create UIManager!" << std::endl;
        isRunning = false; return;
    }

    playerEntity = &manager.addEntity();
    playerEntity->addComponent<TransformComponent>(CHAR_X, CHAR_Y, CHAR_W, CHAR_H, 2);
    playerEntity->addComponent<SpriteComponent>("player", true);
    playerEntity->addComponent<KeyboardController>();
    playerEntity->addComponent<ColliderComponent>("player", 32, 37);
    playerEntity->addComponent<SoundComponent>();
    playerEntity->getComponent<SoundComponent>().addSoundEffect("shoot", "gunshot_sound");
    playerEntity->getComponent<SoundComponent>().addSoundEffect("fire_cast", "fire_spell_sound");
    playerEntity->getComponent<SoundComponent>().addSoundEffect("star_cast", "star_spell_sound");
    playerEntity->getComponent<SoundComponent>().setBackgroundMusic("level_music", true, -1);
    playerEntity->getComponent<SoundComponent>().addSoundEffect("gameover_sfx", "gameover_sfx");
    playerEntity->addComponent<HealthComponent>(1, 1); 
    playerEntity->addComponent<WeaponComponent>( "placeholder", 0, 99999, 0.0f, 0.0f, 1, 1, "projectile", 1, 1, 50); 
    playerEntity->addComponent<SpellComponent>(
        "placeholder_spell", 
        5,                   
        100,                 
        1.5f,                
        1,                   
        16,                  
        "fire",              
        SpellTrajectory::SPIRAL, 
        0.5f,                
        10                   
    );
    playerEntity->addComponent<SpellComponent>(
        "placeholder_star",  
        0,                   
        99999,               
        0.0f,                
        1,                   
        1,                   
        "starproj",          
        SpellTrajectory::RANDOM_DIRECTION, 
        0.0f,                
        1                    
    );
    playerEntity->addGroup(groupPlayers);

    delete playerManager;
    playerManager = new Player(playerEntity);

    if (saveLoadManager) {
        if (!saveLoadManager->loadGameState("saves/default.state")) {
            std::cerr << "Warning: Failed to load default.state. Using component constructor defaults." << std::endl;
        } else {
            std::cout << "Successfully loaded stats from default.state." << std::endl;
        }
    } else {
        std::cerr << "Error: Cannot load default stats, SaveLoadManager is null!" << std::endl;
    }
    Mix_VolumeMusic(Game::musicVolume);
    Mix_Volume(-1, Game::sfxVolume);

    delete map;
    map = new Map(manager, "terrain", 1, 32);
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10, spawnPoints);

    pauseFont = TTF_OpenFont("assets/font.ttf", 10);
    if (!pauseFont) {
        std::cerr << "Failed to load pause font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        if(ui && ui->getFont()) pauseFont = ui->getFont(); 
    }

    pauseBoxTex = assets->GetTexture("pausebox");
    buttonBoxTex = assets->GetTexture("buttonbox");
    soundOnTex = assets->GetTexture("soundon");
    soundOffTex = assets->GetTexture("soundoff");
    sliderTrackTex = assets->GetTexture("slidebar");
    sliderButtonTex = assets->GetTexture("slidebutton");

    SDL_Color textColor = { 0, 0, 0, 255 };
    continueTextTex = renderPauseText("Continue", textColor);
    saveTextTex = renderPauseText("Save Game", textColor);
    returnTextTex = renderPauseText("Return to Title", textColor);

    isDraggingBgmPause = false;
    isDraggingSfxPause = false;

    gameOverTex = assets->GetTexture("gameover");
    gameOverFont = pauseFont; 
    if (!gameOverFont) {
         std::cerr << "Error: Font not available for Game Over text!" << std::endl;
    } else {
         SDL_Color goTextColor = { 255, 255, 255, 220 };
         if (gameOverTextTex) { SDL_DestroyTexture(gameOverTextTex); gameOverTextTex = nullptr; }
         SDL_Surface* surface = TTF_RenderText_Blended(gameOverFont, "Click anywhere to return to Title", goTextColor);
         if(surface) {
             gameOverTextTex = SDL_CreateTextureFromSurface(renderer, surface);
             SDL_FreeSurface(surface);
             if (!gameOverTextTex) { std::cerr << "Failed to create game over text texture!" << std::endl; }
         } else { std::cerr << "Failed to render game over text surface!" << std::endl; }
    }

    updateSpawnPoolAndWeights();
    isRunning = true;
}

void Game::clean(){
    std::cout << "Game::clean() called." << std::endl;

    if (playerEntity) {
        playerEntity->destroy();
        playerEntity = nullptr;
    }
    manager.refresh(); 

    if (map) { delete map; map = nullptr; }
    if (ui) { delete ui; ui = nullptr; }
    if (playerManager) { delete playerManager; playerManager = nullptr; }
    if (saveLoadManager) { delete saveLoadManager; saveLoadManager = nullptr; }
    if (assets) { delete assets; assets = nullptr; }

    if (pauseFont) { TTF_CloseFont(pauseFont); pauseFont = nullptr; }
    if (continueTextTex) { SDL_DestroyTexture(continueTextTex); continueTextTex = nullptr; }
    if (saveTextTex) { SDL_DestroyTexture(saveTextTex); saveTextTex = nullptr; }
    if (returnTextTex) { SDL_DestroyTexture(returnTextTex); returnTextTex = nullptr; }
    if (gameOverTextTex) { SDL_DestroyTexture(gameOverTextTex); gameOverTextTex = nullptr; }

    std::cout<< "Game instance resources cleaned."<< std::endl;
}

void Game::handleEvents() {
    if (Game::event.type == SDL_QUIT) {
        isRunning = false;
        return;
    }

    switch (currentState) {
        case GameState::Playing:
            if (Game::event.type == SDL_KEYDOWN) {
                if (Game::event.key.keysym.sym == SDLK_ESCAPE) {
                    togglePause();
                    return;
                }
                if (Game::event.key.keysym.sym == SDLK_l) {
                    #ifdef DEBUG 
                    if(playerManager) playerManager->levelUp();
                    #endif
                    return;
                }
                if (Game::event.key.keysym.sym == SDLK_b) {
                    #ifdef DEBUG 
                    std::cout << "[DEBUG] 'B' pressed. Summoning boss." << std::endl;
                    spawnBossNearPlayer(); 
                    #endif
                    return;
                }
            }
            int mouseX_Screen, mouseY_Screen;
            SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
            Game::mouseX = mouseX_Screen + Game::camera.x;
            Game::mouseY = mouseY_Screen + Game::camera.y;
            break;

        case GameState::Paused:
            if (isInBuffSelection) {
                handleBuffSelectionEvents(); 
            } else {
                handlePauseMenuEvents(); 
            }
            break;

        case GameState::GameOver:
            if (Game::event.type == SDL_MOUSEBUTTONDOWN && Game::event.button.button == SDL_BUTTON_LEFT) {
                if(SceneManager::instance) {
                     SceneManager::instance->switchToScene(SceneType::Menu);
                } else {
                     std::cerr << "Error: SceneManager::instance is null, cannot return to title from Game Over!" << std::endl;
                }
                return;
            }
            break;
    }
}

void Game::update(){
    if (currentState != GameState::Playing) {
        return;
    }
    if (!isRunning || !playerEntity) return;

    manager.refresh();
    Uint32 currentTime = SDL_GetTicks();
    manager.update();

    if (!playerEntity->hasComponent<ColliderComponent>() || !playerEntity->hasComponent<TransformComponent>() || !playerEntity->hasComponent<HealthComponent>()) {
         std::cerr << "Error in Game::update: Player missing required components!" << std::endl;
         return;
    }
    ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();
    TransformComponent& playerTransform = playerEntity->getComponent<TransformComponent>();
    HealthComponent& playerHealth = playerEntity->getComponent<HealthComponent>();
    SDL_Rect playerColRect = playerCollider.collider;

    handleTerrainCollision(playerCollider, playerTransform, playerColRect);
    handleProjectileCollisions(currentTime);
    handleEnemySpawning(currentTime);
    updateCamera(playerTransform);
    checkPlayerDeath(playerHealth);
}

void Game::render(){
    if (!renderer) {
         std::cerr << "Error: Game::render called but renderer is null!" << std::endl;
         return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (currentState == GameState::Playing || currentState == GameState::Paused || currentState == GameState::GameOver) {

        for(auto* t : manager.getGroup(Game::groupMap)) if(t && t->isActive()) t->draw();
        for(auto* o : manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->draw();
        for(auto* p : manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->draw();
        for(auto* e : manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->draw();
        if(playerEntity && playerEntity->isActive()) playerEntity->draw();
        for(auto* e : manager.getGroup(Game::groupEnemies)) {
            if (e && e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>() && !e->hasComponent<BossAIComponent>()) {
                 renderHealthBar(*e, e->getComponent<ColliderComponent>().position);
            }
        }

        if (currentState == GameState::Playing && ui && playerManager) {
             ui->renderUI(playerManager);
        }
        else if (currentState == GameState::Paused) {
             renderPausedState();
        }
        else if (currentState == GameState::GameOver) {
            renderGameOverState();
        }
    }
    SDL_RenderPresent(renderer);
}

void Game::handlePauseMenuEvents() {
    int mouseX_Screen, mouseY_Screen;
    SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
    SDL_Point mousePoint = {mouseX_Screen, mouseY_Screen};
    SDL_Event& currentEvent = Game::event;

    auto playPauseClickSound = [&]() {
        if (Game::instance && Game::instance->assets) {
            Mix_Chunk* chunk = Game::instance->assets->GetSoundEffect("button_click");
            if (chunk) Mix_PlayChannel(-1, chunk, 0);
        }
    };

    if (currentEvent.type == SDL_MOUSEBUTTONDOWN && currentEvent.button.button == SDL_BUTTON_LEFT) {
        bool clickHandled = false;
        if (SDL_PointInRect(&mousePoint, &continueButtonRect)) { playPauseClickSound(); togglePause(); clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &saveButtonRect)) { playPauseClickSound(); if (saveLoadManager) saveLoadManager->saveGameState(); clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &returnButtonRect)) { playPauseClickSound(); togglePause(); if(SceneManager::instance) SceneManager::instance->switchToScene(SceneType::Menu); clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &bgmIconRectPause)) { playPauseClickSound(); toggleMute(true); clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &sfxIconRectPause)) { playPauseClickSound(); toggleMute(false); clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &bgmSliderButtonRectPause)) { playPauseClickSound(); isDraggingBgmPause = true; sliderDragXPause = mouseX_Screen - bgmSliderButtonRectPause.x; clickHandled = true; }
        else if (SDL_PointInRect(&mousePoint, &sfxSliderButtonRectPause)) { playPauseClickSound(); isDraggingSfxPause = true; sliderDragXPause = mouseX_Screen - sfxSliderButtonRectPause.x; clickHandled = true; }
        if(clickHandled) return;
    }
    else if (currentEvent.type == SDL_MOUSEBUTTONUP && currentEvent.button.button == SDL_BUTTON_LEFT) {
         if (isDraggingBgmPause || isDraggingSfxPause) { isDraggingBgmPause = false; isDraggingSfxPause = false; }
    }
    else if (currentEvent.type == SDL_MOUSEMOTION) {
         handleSliderDrag(mouseX_Screen);
    }
    else if (currentEvent.type == SDL_KEYDOWN) {
        if (currentEvent.key.keysym.sym == SDLK_ESCAPE) { togglePause(); return; }
    }
}

void Game::handleBuffSelectionEvents() {
    SDL_Event& currentEvent = Game::event;
    if (currentEvent.type == SDL_MOUSEBUTTONDOWN && currentEvent.button.button == SDL_BUTTON_LEFT) {
        int mouseX_Screen, mouseY_Screen;
        SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
        std::vector<SDL_Rect> buffButtonRects = getBuffButtonRects();
        for (size_t i = 0; i < buffButtonRects.size() && i < currentBuffOptions.size(); ++i) {
            if (ui && ui->isMouseInside(mouseX_Screen, mouseY_Screen, buffButtonRects[i])) {
                applySelectedBuff(static_cast<int>(i));
                return;
            }
        }
    } else if (currentEvent.type == SDL_KEYDOWN) {
        switch (currentEvent.key.keysym.sym) {
            case SDLK_1: applySelectedBuff(0); return;
            case SDLK_2: applySelectedBuff(1); return;
            case SDLK_3: applySelectedBuff(2); return;
            case SDLK_4: applySelectedBuff(3); return;
            default: break;
        }
    }
}

void Game::handleSliderDrag(int mouseX_Screen) {
    int w, h;
    if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600; }

    if (isDraggingBgmPause) {
        int trackX = bgmSliderTrackRectPause.x;
        int trackButtonW = bgmSliderButtonRectPause.w;
        int trackW = std::max(1, bgmSliderTrackRectPause.w - trackButtonW);
        int targetButtonX = std::max(trackX, std::min(mouseX_Screen - sliderDragXPause, trackX + trackW));
        float percent = static_cast<float>(targetButtonX - trackX) / trackW;
        int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
        if (newVolume != Game::musicVolume) {
            Game::setMusicVolume(newVolume);
            if (newVolume > 0) storedMusicVolumePause = newVolume;
            calculatePauseLayout();
        }
    } else if (isDraggingSfxPause) {
        int trackX = sfxSliderTrackRectPause.x;
        int trackButtonW = sfxSliderButtonRectPause.w;
        int trackW = std::max(1, sfxSliderTrackRectPause.w - trackButtonW);
        int targetButtonX = std::max(trackX, std::min(mouseX_Screen - sliderDragXPause, trackX + trackW));
        float percent = static_cast<float>(targetButtonX - trackX) / trackW;
        int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
        if (newVolume != Game::sfxVolume) {
            Game::setSfxVolume(newVolume);
            if (newVolume > 0) storedSfxVolumePause = newVolume;
            calculatePauseLayout();
        }
    }
}

void Game::renderPausedState() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
    SDL_Rect fullscreen = {0, 0, 0, 0};
    if(renderer) SDL_GetRendererOutputSize(renderer, &fullscreen.w, &fullscreen.h); else { fullscreen.w = 800; fullscreen.h = 600; }
    SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    if (isInBuffSelection && ui) {
        ui->renderBuffSelectionUI(currentBuffOptions, windowWidth, windowHeight);
        if (playerManager) { 
            int currentY = 15; 

            ui->renderPlayerHealthBar(playerManager, currentY); 
            ui->renderExpBar(playerManager, currentY);          
        } else {
            std::cerr << "Warning: Cannot render HP/EXP in buff selection - playerManager is null!" << std::endl; 
        }
    } else {
        renderPauseMenuUI();
    }
}

void Game::renderPauseMenuUI() {
    if (pauseBoxTex) SDL_RenderCopy(renderer, pauseBoxTex, NULL, &pauseBoxRect);
    if (buttonBoxTex) {
        SDL_RenderCopy(renderer, buttonBoxTex, NULL, &continueButtonRect);
        SDL_RenderCopy(renderer, buttonBoxTex, NULL, &saveButtonRect);
        SDL_RenderCopy(renderer, buttonBoxTex, NULL, &returnButtonRect);
    }
    if (continueTextTex) SDL_RenderCopy(renderer, continueTextTex, NULL, &continueTextRect);
    if (saveTextTex) SDL_RenderCopy(renderer, saveTextTex, NULL, &saveTextRect);
    if (returnTextTex) SDL_RenderCopy(renderer, returnTextTex, NULL, &returnTextRect);

    SDL_Texture* bgmIcon = (Game::getMusicVolume() == 0) ? soundOffTex : soundOnTex;
    SDL_Texture* sfxIcon = (Game::getSfxVolume() == 0) ? soundOffTex : soundOnTex;
    if (bgmIcon) SDL_RenderCopy(renderer, bgmIcon, NULL, &bgmIconRectPause);
    if (sfxIcon) SDL_RenderCopy(renderer, sfxIcon, NULL, &sfxIconRectPause);
    if (sliderTrackTex) {
      SDL_RenderCopy(renderer, sliderTrackTex, NULL, &bgmSliderTrackRectPause);
      SDL_RenderCopy(renderer, sliderTrackTex, NULL, &sfxSliderTrackRectPause);
    }
    if (sliderButtonTex) {
        SDL_Color origColor; SDL_GetTextureColorMod(sliderButtonTex, &origColor.r, &origColor.g, &origColor.b);
        if(isDraggingBgmPause || isDraggingSfxPause) SDL_SetTextureColorMod(sliderButtonTex, 200, 200, 200);
        SDL_RenderCopy(renderer, sliderButtonTex, NULL, &bgmSliderButtonRectPause);
        SDL_RenderCopy(renderer, sliderButtonTex, NULL, &sfxSliderButtonRectPause);
        SDL_SetTextureColorMod(sliderButtonTex, origColor.r, origColor.g, origColor.b);
    }
}

void Game::renderGameOverState() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fullscreen = {0, 0, 0, 0};
    if(renderer) SDL_GetRendererOutputSize(renderer, &fullscreen.w, &fullscreen.h); else { fullscreen.w = 800; fullscreen.h = 600; }
    SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    if (gameOverTex) {
        int texW_orig, texH_orig; SDL_QueryTexture(gameOverTex, NULL, NULL, &texW_orig, &texH_orig);
        float gameOverScale = 0.8f;
        int windowW_render, windowH_render; SDL_GetRendererOutputSize(renderer, &windowW_render, &windowH_render);
        gameOverRect.w = std::min(static_cast<int>(texW_orig * gameOverScale), windowW_render * 4/5 );
        gameOverRect.h = std::min(static_cast<int>(texH_orig * gameOverScale), windowH_render * 4/5 );
        gameOverRect.x = (windowW_render - gameOverRect.w) / 2;
        gameOverRect.y = (windowH_render - gameOverRect.h) / 3;
        SDL_RenderCopy(renderer, gameOverTex, NULL, &gameOverRect);
    }
    if (gameOverTextTex) {
        int textW_orig, textH_orig; SDL_QueryTexture(gameOverTextTex, NULL, NULL, &textW_orig, &textH_orig);
        float goTextScale = 0.9f;
        gameOverTextRect.w = static_cast<int>(textW_orig * goTextScale);
        gameOverTextRect.h = static_cast<int>(textH_orig * goTextScale);
        int currentWindowWidth_text; SDL_GetRendererOutputSize(renderer, &currentWindowWidth_text, NULL);
        gameOverTextRect.x = (currentWindowWidth_text - gameOverTextRect.w) / 2;
        gameOverTextRect.y = gameOverRect.y + gameOverRect.h + 30;
        SDL_RenderCopy(renderer, gameOverTextTex, NULL, &gameOverTextRect);
    }
}

void Game::renderHealthBar(Entity& entity, Vector2D position) {
    if (!entity.hasComponent<HealthComponent>() || !entity.hasComponent<ColliderComponent>()) return;

    const HealthComponent& health = entity.getComponent<HealthComponent>();
    if (health.getHealth() <= 0 || health.getHealth() == health.getMaxHealth()) return;

    int barWidth = 40, barHeight = 5;
    int xPos = static_cast<int>(position.x) - camera.x + (entity.getComponent<ColliderComponent>().collider.w / 2) - (barWidth / 2);
    int yPos = static_cast<int>(position.y) - camera.y - 15;

    if (xPos + barWidth < 0 || xPos > Game::camera.w || yPos + barHeight < 0 || yPos > Game::camera.h) return;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); 
    SDL_Rect bgRect = {xPos - 1, yPos - 1, barWidth + 2, barHeight + 2};
    SDL_RenderFillRect(renderer, &bgRect);

    float healthPercent = static_cast<float>(health.getHealth()) / health.getMaxHealth();
    int currentBarWidth = static_cast<int>(barWidth * healthPercent);
    int r = static_cast<int>(255 * (1.0f - healthPercent)); int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255); 
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, barHeight};
    SDL_RenderFillRect(renderer, &healthRect);
}

void Game::handleTerrainCollision(ColliderComponent& playerCollider, TransformComponent& playerTransform, SDL_Rect& playerColRect) {
    for (auto* c : manager.getGroup(Game::groupColliders)) {
        if (!c || !c->isActive() || !c->hasComponent<ColliderComponent>()) continue;
        ColliderComponent& obstacleCollider = c->getComponent<ColliderComponent>();
        if (obstacleCollider.tag != "terrain") continue;

        SDL_Rect cCol = obstacleCollider.collider;
        if (Collision::AABB(playerColRect, cCol)) {
             Vector2D centerPlayer(playerColRect.x + playerColRect.w / 2.0f, playerColRect.y + playerColRect.h / 2.0f);
             Vector2D centerObstacle(cCol.x + cCol.w / 2.0f, cCol.y + cCol.h / 2.0f);
             float overlapX = (playerColRect.w / 2.0f + cCol.w / 2.0f) - std::abs(centerPlayer.x - centerObstacle.x);
             float overlapY = (playerColRect.h / 2.0f + cCol.h / 2.0f) - std::abs(centerPlayer.y - centerObstacle.y);

             if (overlapX > 0 && overlapY > 0) {
                 if (overlapX < overlapY) {
                     playerTransform.position.x += (centerPlayer.x < centerObstacle.x ? -overlapX : overlapX);
                 } else {
                     playerTransform.position.y += (centerPlayer.y < centerObstacle.y ? -overlapY : overlapY);
                 }
                 playerCollider.update();
                 playerColRect = playerCollider.collider; 
             }
        }
    }
}

void Game::handleProjectileCollisions(Uint32 currentTime) {
    auto& projectiles = manager.getGroup(Game::groupProjectiles);
    auto& enemies = manager.getGroup(Game::groupEnemies);
    SDL_Texture* bossProjTexture = Game::instance && Game::instance->assets ? Game::instance->assets->GetTexture("boss_projectile") : nullptr;

    for (auto* p : projectiles) {
         if (!p || !p->isActive() || !p->hasComponent<ColliderComponent>() || !p->hasComponent<ProjectileComponent>() || !p->hasComponent<SpriteComponent>() || !p->hasComponent<TransformComponent>()) continue;

         ColliderComponent& projectileColliderComp = p->getComponent<ColliderComponent>();
         ProjectileComponent& projComp = p->getComponent<ProjectileComponent>();
         SpriteComponent& projSprite = p->getComponent<SpriteComponent>();

         if (bossProjTexture && projSprite.getTexture() == bossProjTexture) {
             continue;
         }

         for (auto* e : enemies) {
             if (!e || !e->isActive() || !e->hasComponent<ColliderComponent>()) continue;
             if (projComp.hasHit(e)) continue; 

             if (Collision::AABB(e->getComponent<ColliderComponent>().collider, projectileColliderComp.collider)) {
                 handleProjectileHitEnemy(p, e, projComp, currentTime);
                 if (!p->isActive()) break; 
             }
         }
     }

     ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();
     SDL_Rect playerColRect = playerCollider.collider;
     for (auto* p : projectiles) {
         if (!p || !p->isActive() || !p->hasComponent<ColliderComponent>() || !p->hasComponent<ProjectileComponent>() || !p->hasComponent<SpriteComponent>()) continue;

         ColliderComponent& projCollider = p->getComponent<ColliderComponent>();
         SpriteComponent& projSprite = p->getComponent<SpriteComponent>();

         if (bossProjTexture && projSprite.getTexture() == bossProjTexture) {
             if (Collision::AABB(projCollider.collider, playerColRect)) {
                 handleBossProjectileHitPlayer(p, currentTime);
             }
         }
     }
}

void Game::handleProjectileHitEnemy(Entity* projectile, Entity* enemy, ProjectileComponent& projComp, Uint32 currentTime) {
    int damage = projComp.getDamage();

    if (playerManager) {
        float lifestealPercent = playerManager->getLifestealPercentage();
        if (lifestealPercent > 0 && damage > 0) {
            int healAmount = static_cast<int>(std::round(damage * (lifestealPercent / 100.0f)));
            if (healAmount > 0) playerManager->heal(healAmount);
        }
    }

    if (enemy->hasComponent<HealthComponent>()) {
        HealthComponent& enemyHealth = enemy->getComponent<HealthComponent>();
        enemyHealth.takeDamage(damage);
        if (enemyHealth.getHealth() <= 0) {
            handleEnemyDeath(enemy, enemyHealth.getMaxHealth());
        }
    }

    if (enemy->hasComponent<TransformComponent>() && projectile->hasComponent<TransformComponent>()) {
        Vector2D knockbackDir = projectile->getComponent<TransformComponent>().velocity.Normalize();
        float knockbackForce = 35.0f;
        enemy->getComponent<TransformComponent>().position += knockbackDir * knockbackForce;
        if(enemy->hasComponent<ColliderComponent>()) enemy->getComponent<ColliderComponent>().update();
    }

    if (enemy->hasComponent<SpriteComponent>()) {
        enemy->getComponent<SpriteComponent>().isHit = true;
        enemy->getComponent<SpriteComponent>().hitTime = currentTime;
    }

    projComp.recordHit(enemy);
    if (projComp.shouldDestroy()) {
        projectile->destroy();
    }
}

void Game::handleEnemyDeath(Entity* enemy, int maxHp) {
    int finalExp = 0; int baseExp = 0; int playerLvl = 1;
    if (enemy->hasComponent<EnemyAIComponent>()) baseExp = enemy->getComponent<EnemyAIComponent>().getExpValue();
    if (playerManager) playerLvl = playerManager->getLevel();
    finalExp = std::max(1, baseExp + (maxHp / 100) + (playerLvl / 5));

    if (enemy->hasComponent<TransformComponent>()) {
         Vector2D deathPosition = enemy->getComponent<TransformComponent>().position;
         deathPosition.x += (enemy->getComponent<TransformComponent>().width * enemy->getComponent<TransformComponent>().scale) / 2.0f;
         deathPosition.y += (enemy->getComponent<TransformComponent>().height * enemy->getComponent<TransformComponent>().scale) / 2.0f;

         std::string orbTextureId = "exp_orb_1";
         if (finalExp >= 500) orbTextureId = "exp_orb_500";
         else if (finalExp >= 200) orbTextureId = "exp_orb_200";
         else if (finalExp >= 100) orbTextureId = "exp_orb_100";
         else if (finalExp >= 50) orbTextureId = "exp_orb_50";
         else if (finalExp >= 10) orbTextureId = "exp_orb_10";

         auto& orb = manager.addEntity();
         orb.addComponent<TransformComponent>(deathPosition.x, deathPosition.y, 16, 16, 1);
         orb.addComponent<SpriteComponent>(orbTextureId);
         orb.addComponent<ColliderComponent>("exp_orb", 16, 16);
         orb.addComponent<ExpOrbComponent>(finalExp);
         orb.addGroup(groupExpOrbs);
    }
    if(playerManager) playerManager->incrementEnemiesDefeated();

}

void Game::handleBossProjectileHitPlayer(Entity* projectile, Uint32 currentTime) {
    ProjectileComponent& projComp = projectile->getComponent<ProjectileComponent>();
    int damage = projComp.getDamage();

    playerEntity->getComponent<HealthComponent>().takeDamage(damage);

    if (playerEntity->hasComponent<SpriteComponent>()) {
         playerEntity->getComponent<SpriteComponent>().isHit = true;
         playerEntity->getComponent<SpriteComponent>().hitTime = currentTime;
    }
    projectile->destroy();
}

void Game::handleEnemySpawning(Uint32 currentTime) {
    Uint32 spawnInterval = 1000;
    if (playerManager) {
        int currentLevel = playerManager->getLevel();
        if (currentLevel <= 5) spawnInterval = 3000;
        else if (currentLevel <= 50) spawnInterval = 2000;
    }

    if (currentTime > lastEnemySpawnTime + spawnInterval) {
        spawnEnemy();
        lastEnemySpawnTime = currentTime;
    }
}

void Game::updateCamera(TransformComponent& playerTransform) {
    int currentWindowWidth = WINDOW_WIDTH, currentWindowHeight = WINDOW_HEIGHT;
    if (renderer) { SDL_GetRendererOutputSize(renderer, &currentWindowWidth, &currentWindowHeight); }
    else { std::cerr << "Warning: Game::renderer is null during camera update!" << std::endl; }

    camera.w = currentWindowWidth;
    camera.h = currentWindowHeight;
    camera.x = static_cast<int>(playerTransform.position.x - (currentWindowWidth / 2.0f));
    camera.y = static_cast<int>(playerTransform.position.y - (currentWindowHeight / 2.0f));

    int mapPixelWidth = MAP_WIDTH * TILE_SIZE;
    int mapPixelHeight = MAP_HEIGHT * TILE_SIZE;
    camera.x = std::max(0, std::min(camera.x, mapPixelWidth - camera.w));
    camera.y = std::max(0, std::min(camera.y, mapPixelHeight - camera.h));
}

void Game::checkPlayerDeath(HealthComponent& playerHealth) {
    if (playerHealth.getHealth() <= 0 && currentState != GameState::GameOver) {
        currentState = GameState::GameOver;
        Mix_HaltMusic();
        if (playerEntity->hasComponent<SoundComponent>()) {
            playerEntity->getComponent<SoundComponent>().playSoundEffect("gameover_sfx");
        } else {
             std::cerr << "Warning: Player has no SoundComponent to play game over SFX!" << std::endl;
        }
    }
}

void Game::initializeEnemyDatabase() {
    allEnemyDatabase.clear();
    allEnemyDatabase.push_back({"zombie", zombieSprite, zombieHealth, zombieDamage, zombieSpeed, zombieExp, 0, 1});
    allEnemyDatabase.push_back({"kfc1", kfc1Sprite, kfc1Health, kfc1Damage, kfc1Speed, kfc1Exp, 0, 5, "kfc2", 15});
    allEnemyDatabase.push_back({"ina1", ina1Sprite, ina1Health, ina1Damage, ina1Speed, ina1Exp, 0, 5, "ina2", 15});
    allEnemyDatabase.push_back({"bear1", bear1Sprite, bear1Health, bear1Damage, bear1Speed, bear1Exp, 0, 5, "bear2", 15});
    allEnemyDatabase.push_back({"skeleton1", skeleton1Sprite, skeleton1Health, skeleton1Damage, skeleton1Speed, skeleton1Exp, 0, 5, "skeleton2", 15});
    allEnemyDatabase.push_back({"aligator1", aligator1Sprite, aligator1Health, aligator1Damage, aligator1Speed, aligator1Exp, 0, 5, "aligator2", 15});
    allEnemyDatabase.push_back({"kfc2", kfc2Sprite, kfc2Health, kfc2Damage, kfc2Speed, kfc2Exp, 0, 15});
    allEnemyDatabase.push_back({"ina2", ina2Sprite, ina2Health, ina2Damage, ina2Speed, ina2Exp, 0, 15, "ina3", 25});
    allEnemyDatabase.push_back({"ina3", ina3Sprite, ina3Health, ina3Damage, ina3Speed, ina3Exp, 0, 25});
    allEnemyDatabase.push_back({"bear2", bear2Sprite, bear2Health, bear2Damage, bear2Speed, bear2Exp, 0, 15});
    allEnemyDatabase.push_back({"skeleton2", skeleton2Sprite, skeleton2Health, skeleton2Damage, skeleton2Speed, skeleton2Exp, 0, 15, "skeleton3", 25});
    allEnemyDatabase.push_back({"skeleton3", skeleton3Sprite, skeleton3Health, skeleton3Damage, skeleton3Speed, skeleton3Exp, 0, 25, "skeleton4", 35});
    allEnemyDatabase.push_back({"skeleton4", skeleton4Sprite, skeleton4Health, skeleton4Damage, skeleton4Speed, skeleton4Exp, 0, 35, "skeleton5", 45});
    allEnemyDatabase.push_back({"skeleton5", skeleton5Sprite, skeleton5Health, skeleton5Damage, skeleton5Speed, skeleton5Exp, 0, 45});
    allEnemyDatabase.push_back({"aligator2", aligator2Sprite, aligator2Health, aligator2Damage, aligator2Speed, aligator2Exp, 0, 15});
    allEnemyDatabase.push_back({"skeleton_shield", skeletonShieldSprite, skeletonShieldHealth, skeletonShieldDamage, skeletonShieldSpeed, skeletonShieldExp, 0, 15});
    allEnemyDatabase.push_back({"eliteskeleton_shield", eliteSkeletonShieldSprite, eliteSkeletonShieldHealth, eliteSkeletonShieldDamage, eliteSkeletonShieldSpeed, eliteSkeletonShieldExp, 0, 25});
}

void Game::updateSpawnPoolAndWeights() {
    if (!playerManager) return;
    int playerLevel = playerManager->getLevel();
    currentSpawnPool.clear();
    currentTotalSpawnWeight = 0;
    std::map<std::string, EnemySpawnInfo*> poolMap;

    for (EnemySpawnInfo& dbEntry : allEnemyDatabase) {
        if (playerLevel >= dbEntry.minLevel) {
             bool isUpgradedVersion = false;
             std::string baseEnemyTag = "";
             for (const auto& checkEntry : allEnemyDatabase) {
                  if (checkEntry.upgradeTag == dbEntry.tag && playerLevel >= checkEntry.upgradeLevelRequirement) {
                       isUpgradedVersion = true;
                       baseEnemyTag = checkEntry.tag;
                       break;
                  }
             }
            if (isUpgradedVersion && poolMap.count(baseEnemyTag)) { poolMap.erase(baseEnemyTag); }
            poolMap[dbEntry.tag] = &dbEntry;
        }
    }

    for (auto const& [tag, enemyInfoPtr] : poolMap) {
        EnemySpawnInfo& enemyInfo = *enemyInfoPtr;
        if (playerLevel <= 5) {
            enemyInfo.currentSpawnWeight = (enemyInfo.tag == "zombie") ? 20 : 0;
        } else {
            bool isBaseLvl5Mob = (enemyInfo.minLevel == 5 && (enemyInfo.tag == "kfc1" || enemyInfo.tag == "ina1" || enemyInfo.tag == "bear1" || enemyInfo.tag == "skeleton1" || enemyInfo.tag == "aligator1"));
            if (enemyInfo.minLevel == 5 && playerLevel == 5) {
                 if(isBaseLvl5Mob) enemyInfo.currentSpawnWeight = 5;
            }
            else if (enemyInfo.minLevel > 5 && playerLevel >= enemyInfo.minLevel && enemyInfo.currentSpawnWeight == 0) {
                 enemyInfo.currentSpawnWeight = 5;
            }
            else if (enemyInfo.tag == "zombie") {
                 enemyInfo.currentSpawnWeight = 20;
            }
            else if (enemyInfo.currentSpawnWeight == 0 && enemyInfo.minLevel <= playerLevel) {
                  if (isBaseLvl5Mob) { enemyInfo.currentSpawnWeight = 5; }
                  else if (enemyInfo.minLevel > 5){ enemyInfo.currentSpawnWeight = 5; }
                  else { enemyInfo.currentSpawnWeight = 0; }
            }
            int increaseCycles = (playerLevel - 5) / 10;
            if (increaseCycles > 0 && enemyInfo.currentSpawnWeight > 0) {
                 enemyInfo.currentSpawnWeight = std::min(25, enemyInfo.currentSpawnWeight + (increaseCycles * 10));
            }
        }
        if (enemyInfo.currentSpawnWeight > 0) {
            currentSpawnPool.push_back(&enemyInfo);
            currentTotalSpawnWeight += enemyInfo.currentSpawnWeight;
        }
    }
}

EnemySpawnInfo* Game::selectEnemyBasedOnWeight() {
    if (currentSpawnPool.empty()) {
        updateSpawnPoolAndWeights();
        if (currentSpawnPool.empty()) { std::cerr << "Spawn pool is empty even after update!" << std::endl; return nullptr; }
    }
    if (currentTotalSpawnWeight <= 0) {
        std::cerr << "Warning: Total spawn weight is zero. Picking random enemy." << std::endl;
        return currentSpawnPool[rand() % currentSpawnPool.size()];
    }
    int randomValue = rand() % currentTotalSpawnWeight;
    int cumulativeWeight = 0;
    for (EnemySpawnInfo* enemyInfo : currentSpawnPool) {
        cumulativeWeight += enemyInfo->currentSpawnWeight;
        if (randomValue < cumulativeWeight) {
            return enemyInfo;
        }
    }
    std::cerr << "Warning: Weighted selection failed. Returning last enemy." << std::endl;
    return currentSpawnPool.back();
}

void Game::spawnEnemy() {
    if (!playerEntity || !playerManager || spawnPoints.empty()) {
        std::cerr << "Cannot spawn enemy, player/spawns incomplete!" << std::endl;
        return;
    }

    EnemySpawnInfo* selectedEnemyInfo = selectEnemyBasedOnWeight();
    if (!selectedEnemyInfo) {
        std::cerr << "Failed to select an enemy from the pool!" << std::endl;
        return;
    }

    Vector2D spawnPosition = spawnPoints[std::rand() % spawnPoints.size()];
    auto& enemy = manager.addEntity();

    float healthDmgModifier = 1.0f;
    int playerLevel = playerManager->getLevel();
    if (playerLevel >= 50) { healthDmgModifier = std::pow(1.20f, (playerLevel - 50) / 5); }
    int finalHealth = static_cast<int>(std::max(1.0f, selectedEnemyInfo->baseHealth * healthDmgModifier));
    int finalDamage = static_cast<int>(std::max(1.0f, selectedEnemyInfo->baseDamage * healthDmgModifier));

    int enemySpriteWidth = 64, enemySpriteHeight = 64; 
    enemy.addComponent<TransformComponent>(spawnPosition.x, spawnPosition.y, enemySpriteWidth, enemySpriteHeight, 2);
    enemy.addComponent<SpriteComponent>(selectedEnemyInfo->tag, true);
    enemy.addComponent<ColliderComponent>(selectedEnemyInfo->tag, 64, 64); 
    enemy.addComponent<HealthComponent>(finalHealth, finalHealth);

    if (!playerEntity->hasComponent<TransformComponent>()) {
         std::cerr << "ERROR in spawnEnemy: Player missing TransformComponent!" << std::endl;
         enemy.destroy(); return;
    }
    Vector2D* playerPosPtr = &playerEntity->getComponent<TransformComponent>().position;

    enemy.addComponent<EnemyAIComponent>(5000, selectedEnemyInfo->speed, playerPosPtr, finalDamage, selectedEnemyInfo->baseExperience, playerEntity);
    enemy.addGroup(groupEnemies);
}

void Game::spawnBoss() { 
    if (spawnPoints.empty() || !playerEntity || !playerManager || !playerEntity->hasComponent<TransformComponent>()) {
        std::cerr << "Cannot spawn boss: No spawn points or player not ready!" << std::endl;
        return;
    }
    for (auto* entity : manager.getGroup(groupEnemies)) {
        if (entity && entity->hasComponent<BossAIComponent>()) {
             std::cout << "Boss already exists, not spawning another." << std::endl;
             return;
        }
    }
    Vector2D spawnPosition = spawnPoints[std::rand() % spawnPoints.size()];
    spawnBossAt(spawnPosition);
}

void Game::spawnBossNearPlayer() { 
    if (!playerEntity || !playerManager || !playerEntity->hasComponent<TransformComponent>()) {
        std::cerr << "Cannot spawn boss (debug): Player not ready." << std::endl;
        return;
    }
    for (auto* entity : manager.getGroup(groupEnemies)) {
        if (entity && entity->hasComponent<BossAIComponent>()) {
             std::cout << "Boss already exists, not spawning another (debug)." << std::endl;
             return;
        }
    }
    Vector2D playerPos = playerEntity->getComponent<TransformComponent>().position;
    Vector2D spawnPos = playerPos; spawnPos.x += 150;
    spawnBossAt(spawnPos);
}


void Game::spawnBossAt(Vector2D spawnPos) { 
    std::cout << "Spawning Boss at (" << spawnPos.x << ", " << spawnPos.y << ")" << std::endl;

    auto& boss = manager.addEntity();
    boss.addComponent<TransformComponent>(spawnPos.x, spawnPos.y, BOSS_SPRITE_WIDTH, BOSS_SPRITE_HEIGHT, 1);
    boss.addComponent<SpriteComponent>("boss_walk", true);
    boss.addComponent<ColliderComponent>("boss", 90, 90); 

    int playerLevel = playerManager ? playerManager->getLevel() : 1;
    float scaleFactor = 1.0f + (static_cast<float>(playerLevel) / 10.0f) + (static_cast<float>(playerLevel) / 20.0f) + (static_cast<float>(playerLevel) / 50.0f);
    int scaledBossHealth = static_cast<int>(std::max((float)BOSS_HEALTH, BOSS_HEALTH * scaleFactor));
    boss.addComponent<HealthComponent>(scaledBossHealth, scaledBossHealth);

    Entity* playerEntPtr = playerEntity;
    if (!playerEntPtr) {
          std::cerr << "FATAL ERROR: Invalid player entity pointer during boss spawn!" << std::endl;
          boss.destroy(); return;
     }

    boss.addComponent<BossAIComponent>(BOSS_SPEED, 100.0f, 90.0f, BOSS_SLAM_DAMAGE, BOSS_PROJECTILE_DAMAGE, BOSS_KNOCKBACK_FORCE, playerEntPtr);
    boss.addGroup(groupEnemies);

    if (ui) { ui->setBossEntity(&boss); }
}

void Game::generateBuffOptions() {
    currentBuffOptions.clear(); 
    if (!playerEntity || !playerManager) { std::cerr << "Cannot generate buffs: player invalid." << std::endl; return; }

    WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
    SpellComponent* fireSpellComp = nullptr;
    SpellComponent* starSpellComp = nullptr;

    for (const auto& compPtr : playerEntity->getAllComponents()) {
         if (SpellComponent* sc = dynamic_cast<SpellComponent*>(compPtr.get())) {

             if (sc->getTag() == "spell") {
                 fireSpellComp = sc;
             } else if (sc->getTag() == "star") {
                 starSpellComp = sc;
             }

         }
    }

    int currentWeaponLevel = weaponComp ? weaponComp->getLevel() : -1;
    int currentFireLevel = fireSpellComp ? fireSpellComp->getLevel() : -1;
    int currentStarLevel = starSpellComp ? starSpellComp->getLevel() : -1; 
    bool isFireLv0 = (currentFireLevel == 0);
    bool isStarLv0 = (currentStarLevel == 0); 

    std::vector<BuffInfo> grantBuffs;

    if (fireSpellComp && isFireLv0) {
        grantBuffs.push_back({"Fire Vortex", "Summon fire around the player", BuffType::FIRE_SPELL_PROJ_PLUS_1, 10.0f}); 
    }
    if (starSpellComp && isStarLv0) { 
        grantBuffs.push_back({"Starfall", "Shoot stars at random direction", BuffType::STAR_SPELL_PROJ_PLUS_1, 10.0f}); 
    }

    std::vector<BuffInfo> randomBuffPool;

    randomBuffPool.push_back({"Heal", "Restore 200 HP", BuffType::PLAYER_HEAL_FLAT, 200.0f});
    randomBuffPool.push_back({"Heal", "Restore 30% Max HP", BuffType::PLAYER_HEAL_PERC_MAX, 30.0f});
    randomBuffPool.push_back({"Heal", "Restore 60% Lost HP", BuffType::PLAYER_HEAL_PERC_LOST, 60.0f});
    randomBuffPool.push_back({"Max HP+", "+100 Max Health", BuffType::PLAYER_MAX_HEALTH_FLAT, 100.0f});
    randomBuffPool.push_back({"Max HP+", "+50% Max Health", BuffType::PLAYER_MAX_HEALTH_PERC_MAX, 50.0f});
    randomBuffPool.push_back({"Max HP+", "+100% Current HP to Max", BuffType::PLAYER_MAX_HEALTH_PERC_CUR, 100.0f});
    randomBuffPool.push_back({"Lifesteal+", "+2% Lifesteal", BuffType::PLAYER_LIFESTEAL, 2.0f});

    if (weaponComp) {
        randomBuffPool.push_back({"Main Weapon Dmg+", "+20% Wpn Damage", BuffType::WEAPON_DAMAGE_FLAT, 20.0f}); 
        randomBuffPool.push_back({"Main Weapon Dmg+", "Increase randomly", BuffType::WEAPON_DAMAGE_RAND_PERC, 0.0f});
        randomBuffPool.push_back({"Main Weapon FireRate+", "-10% Fire Delay", BuffType::WEAPON_FIRE_RATE, 10.0f});
        randomBuffPool.push_back({"Main Weapon Pierce+", "+1 Wpn Pierce", BuffType::WEAPON_PIERCE, 1.0f});
        if (currentWeaponLevel >= 0 && currentWeaponLevel % 5 == 4) { 
             randomBuffPool.push_back({"Main Weapon Spread+", "+1 Spread (DMG -20%)", BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30, 0.0f});
        }
        randomBuffPool.push_back({"Main Weapon Burst+", "+1 Main Weapon Burst", BuffType::WEAPON_BURST_COUNT, 1.0f});
    }

    if (fireSpellComp && !isFireLv0) { 
        randomBuffPool.push_back({"Fire Dmg+", "Increase randomly", BuffType::FIRE_SPELL_DAMAGE, 0.0f}); 
        randomBuffPool.push_back({"Fire Vortex CDR", "-20% Fire Vortex Cooldown", BuffType::FIRE_SPELL_COOLDOWN, 20.0f});
        randomBuffPool.push_back({"Fire Vortex Burst+", "+1 Fire Burst", BuffType::FIRE_SPELL_PROJ_PLUS_1, 1.0f});
    }

    if (starSpellComp && !isStarLv0) { 
        randomBuffPool.push_back({"Starfall Dmg+", "Increase randomly", BuffType::STAR_SPELL_DAMAGE, 0.0f}); 
        randomBuffPool.push_back({"Starfall Cooldown", "-20% Star Cooldown", BuffType::STAR_SPELL_COOLDOWN, 20.0f});
        randomBuffPool.push_back({"Starfall Shot", "+1 Star", BuffType::STAR_SPELL_PROJ_PLUS_1, 1.0f});
    }

    const int totalBuffsToOffer = 4;
    currentBuffOptions = grantBuffs; 

    int randomSlotsNeeded = totalBuffsToOffer - currentBuffOptions.size();
    randomSlotsNeeded = std::max(0, randomSlotsNeeded); 

    int numPossibleRandom = randomBuffPool.size();
    int randomSlotsToFill = std::min(randomSlotsNeeded, numPossibleRandom); 

    if (randomSlotsToFill > 0 && numPossibleRandom > 0) {
        std::vector<int> chosenIndices;

        while (chosenIndices.size() < static_cast<size_t>(randomSlotsToFill)) {
            int randIndex = std::rand() % numPossibleRandom;
            bool alreadyChosen = false;
            for (int chosen : chosenIndices) { if (chosen == randIndex) { alreadyChosen = true; break; } }

            if (!alreadyChosen) {
                chosenIndices.push_back(randIndex);
                currentBuffOptions.push_back(randomBuffPool[randIndex]); 
            }

            if (chosenIndices.size() >= static_cast<size_t>(numPossibleRandom)) break; 

            static int safetyCounter = 0; 
            if (++safetyCounter > numPossibleRandom * 10) { 
                std::cerr << "Warning: Buff selection loop safety break after " << safetyCounter << " attempts." << std::endl;
                safetyCounter = 0; 
                break;
            }
        }
    }

}

void Game::applySelectedBuff(int index) {
    if (!isInBuffSelection || !playerEntity || !playerManager || index < 0 || index >= (int)currentBuffOptions.size()) { if (isInBuffSelection) exitBuffSelection(); return; }
    const BuffInfo& selectedBuff = currentBuffOptions[index];
    int intAmount = static_cast<int>(selectedBuff.amount); float floatAmount = selectedBuff.amount;

    WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
    HealthComponent* healthComp = playerEntity->hasComponent<HealthComponent>() ? &playerEntity->getComponent<HealthComponent>() : nullptr;
    SpellComponent* fireSpellComp = nullptr;
    SpellComponent* starSpellComp = nullptr;
    for (const auto& compPtr : playerEntity->getAllComponents()) { if (SpellComponent* sc = dynamic_cast<SpellComponent*>(compPtr.get())) { if (sc->getTag() == "spell") fireSpellComp = sc; else if (sc->getTag() == "star") starSpellComp = sc; } }

    bool buffApplied = false;
    switch (selectedBuff.type) {

        case BuffType::PLAYER_HEAL_FLAT: if (healthComp) { healthComp->heal(intAmount); buffApplied = true; } break;
        case BuffType::PLAYER_HEAL_PERC_MAX: if (healthComp) { int heal = static_cast<int>(healthComp->getMaxHealth() * (floatAmount / 100.0f)); healthComp->heal(heal); buffApplied = true; } break;
        case BuffType::PLAYER_HEAL_PERC_LOST: if (healthComp) { int lostHP = healthComp->getMaxHealth() - healthComp->getHealth(); int heal = static_cast<int>(lostHP * (floatAmount / 100.0f)); healthComp->heal(heal); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_FLAT: if (healthComp) { healthComp->setMaxHealth(healthComp->getMaxHealth() + intAmount); healthComp->heal(intAmount); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_PERC_MAX: if (healthComp) { int increase = static_cast<int>(healthComp->getMaxHealth() * (floatAmount / 100.0f)); healthComp->setMaxHealth(healthComp->getMaxHealth() + increase); healthComp->heal(increase); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_PERC_CUR: if (healthComp) { int increase = static_cast<int>(healthComp->getHealth() * (floatAmount / 100.0f)); healthComp->setMaxHealth(healthComp->getMaxHealth() + increase); healthComp->heal(increase); buffApplied = true; } break;
        case BuffType::PLAYER_LIFESTEAL: playerManager->setLifestealPercentage(playerManager->getLifestealPercentage() + floatAmount); buffApplied = true; break;

        case BuffType::WEAPON_DAMAGE_FLAT: if (weaponComp) { int increase = std::max(1, static_cast<int>(weaponComp->getDamage() * 0.20f)); weaponComp->increaseDamage(increase); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_DAMAGE_RAND_PERC: if (weaponComp) { int increase = std::max(1, static_cast<int>(weaponComp->getDamage() * ((rand() % 50 + 1) / 100.0f))); weaponComp->increaseDamage(increase); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_FIRE_RATE: if (weaponComp) { weaponComp->decreaseFireRatePercentage(floatAmount); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_PIERCE: if (weaponComp) { weaponComp->increasePierce(intAmount); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_BURST_COUNT: if(weaponComp) { weaponComp->increaseBurstCount(intAmount); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30: if (weaponComp) { int damageReduction = static_cast<int>(weaponComp->getDamage() * 0.20f); weaponComp->increaseDamage(-damageReduction); weaponComp->increaseProjectileCount(1); weaponComp->incrementLevel(); buffApplied = true; } break;

        case BuffType::FIRE_SPELL_DAMAGE: if (fireSpellComp && playerManager) { int increase = std::max(1, 1 * playerManager->getLevel()); fireSpellComp->increaseDamage(increase); fireSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::FIRE_SPELL_COOLDOWN: if (fireSpellComp) { fireSpellComp->decreaseCooldownPercentage(floatAmount); fireSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::FIRE_SPELL_PROJ_PLUS_1: if (fireSpellComp) { int amountToAdd = (fireSpellComp->getLevel() == 0) ? static_cast<int>(selectedBuff.amount) : 1; fireSpellComp->increaseProjectileCount(amountToAdd); fireSpellComp->incrementLevel(); buffApplied = true; } break;

        case BuffType::STAR_SPELL_DAMAGE: if (starSpellComp && playerManager) { int increase = std::max(1, 2 * playerManager->getLevel()); starSpellComp->increaseDamage(increase); starSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::STAR_SPELL_COOLDOWN: if (starSpellComp) { starSpellComp->decreaseCooldownPercentage(floatAmount); starSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::STAR_SPELL_PROJ_PLUS_1: if (starSpellComp) { int amountToAdd = (starSpellComp->getLevel() == 0) ? static_cast<int>(selectedBuff.amount) : 1; starSpellComp->increaseProjectileCount(amountToAdd); starSpellComp->incrementLevel(); buffApplied = true; } break;

        default: std::cerr << "Warning: Invalid BuffType (" << static_cast<int>(selectedBuff.type) << ")." << std::endl; break;
    }

    if (!buffApplied) { std::cerr << "Warning: Buff '" << selectedBuff.name << "' could not be applied." << std::endl; }
    exitBuffSelection();
}

void Game::enterBuffSelection() {
    if (currentState == GameState::Playing) {
        isInBuffSelection = true;
        currentState = GameState::Paused;
        if (Mix_PlayingMusic()) Mix_PauseMusic();
        generateBuffOptions();
    } else {
         std::cerr << "Warning: Tried to enter buff selection when not in Playing state." << std::endl;
    }
}

void Game::exitBuffSelection() {
    if (isInBuffSelection) {
       isInBuffSelection = false;
       currentState = GameState::Playing;
       currentBuffOptions.clear();
       if (Mix_PausedMusic()) Mix_ResumeMusic();
    }
}

void Game::togglePause() {
    if (currentState == GameState::Playing) {
        currentState = GameState::Paused;
        if (Mix_PlayingMusic()) Mix_PauseMusic();
        isDraggingBgmPause = false;
        isDraggingSfxPause = false;
        storedMusicVolumePause = Game::getMusicVolume();
        storedSfxVolumePause = Game::getSfxVolume();
        calculatePauseLayout();
    } else if (currentState == GameState::Paused && !isInBuffSelection) {
        currentState = GameState::Playing;
        if (Mix_PausedMusic()) Mix_ResumeMusic();
    }
}

void Game::setMusicVolume(int volume) {
    musicVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));
    Mix_VolumeMusic(musicVolume);
}

void Game::setSfxVolume(int volume) {
    sfxVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));
    Mix_Volume(-1, sfxVolume);
}

int Game::getMusicVolume() { return musicVolume; }
int Game::getSfxVolume() { return sfxVolume; }

void Game::toggleMute(bool isMusic) {
    if (isMusic) {
        bool wasMuted = (Game::musicVolume == 0);
        if (!wasMuted) { storedMusicVolumePause = Game::musicVolume; Game::setMusicVolume(0); }
        else { Game::setMusicVolume(storedMusicVolumePause > 0 ? storedMusicVolumePause : MIX_MAX_VOLUME / 4); storedMusicVolumePause = Game::getMusicVolume(); }
    } else {
        bool wasMuted = (Game::sfxVolume == 0);
        if (!wasMuted) { storedSfxVolumePause = Game::sfxVolume; Game::setSfxVolume(0); }
        else { Game::setSfxVolume(storedSfxVolumePause > 0 ? storedSfxVolumePause : MIX_MAX_VOLUME / 4); storedSfxVolumePause = Game::getSfxVolume(); }
    }
    calculatePauseLayout(); 
}

Entity& Game::getPlayer() {
    if (!playerEntity) {
         throw std::runtime_error("Error: Game::getPlayer() called when playerEntity is null!");
    }
    return *playerEntity;
}

void Game::calculatePauseLayout() {
    if (!isRunning || !renderer) return;
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    float scaleX = static_cast<float>(windowWidth) / static_cast<float>(1920);
    float scaleY = static_cast<float>(windowHeight) / static_cast<float>(1080);
    float scaleFactor = std::max(std::min(scaleX, scaleY), 0.6f);

    pauseBoxRect.w = static_cast<int>(128 * 1.2f * scaleFactor); pauseBoxRect.h = static_cast<int>(245 * 1.2f * scaleFactor);
    pauseBoxRect.w = std::max(120, pauseBoxRect.w); pauseBoxRect.h = std::max(240, pauseBoxRect.h);
    pauseBoxRect.x = (windowWidth - pauseBoxRect.w) / 2; pauseBoxRect.y = (windowHeight - pauseBoxRect.h) / 2;

    int internalPadding = std::max(5, static_cast<int>(15 * scaleFactor));
    int availableWidth = pauseBoxRect.w - 2 * internalPadding;
    float btnScale = (230 > 0) ? static_cast<float>(availableWidth) / 230 : 1.0f;
    int btnBoxW = availableWidth; int btnBoxH = std::max(15, static_cast<int>(60 * btnScale));
    int volAreaHeightEst = btnBoxH * 2.0f; int iconSize = std::max(15, static_cast<int>(volAreaHeightEst * 0.35f));
    int sliderHeight = std::max(3, iconSize / 3); int sliderWidth = std::max(20, availableWidth - iconSize - internalPadding);
    int sliderButtonW = std::max(5, iconSize / 2); int sliderButtonH = std::max(8, static_cast<int>(sliderHeight * 1.5f));

    int volRowHeight = iconSize + internalPadding / 2;
    int totalElementHeightOnly = (btnBoxH * 3) + (volRowHeight * 2);
    int availableHeightForPadding = pauseBoxRect.h - totalElementHeightOnly;
    int dynamicPaddingY = std::max(3, (availableHeightForPadding > 12) ? availableHeightForPadding / 6 : 1);

    int currentY = pauseBoxRect.y + dynamicPaddingY;
    continueButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH }; currentY += btnBoxH + dynamicPaddingY;
    int volAreaStartY = currentY;
    bgmIconRectPause = {pauseBoxRect.x + internalPadding, volAreaStartY + (volRowHeight - iconSize) / 2, iconSize, iconSize};
    bgmSliderTrackRectPause = {bgmIconRectPause.x + iconSize + internalPadding, volAreaStartY + (volRowHeight - sliderHeight) / 2, sliderWidth, sliderHeight};
    int bgmSliderTrackW = std::max(1, bgmSliderTrackRectPause.w - sliderButtonW); float bgmPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getMusicVolume()) / MIX_MAX_VOLUME;
    bgmSliderButtonRectPause = { bgmSliderTrackRectPause.x + static_cast<int>(bgmPercent * bgmSliderTrackW), bgmSliderTrackRectPause.y + (sliderHeight - sliderButtonH) / 2, sliderButtonW, sliderButtonH };
    int sfxRowStartY = volAreaStartY + volRowHeight;
    sfxIconRectPause = {pauseBoxRect.x + internalPadding, sfxRowStartY + (volRowHeight - iconSize) / 2, iconSize, iconSize};
    sfxSliderTrackRectPause = {sfxIconRectPause.x + iconSize + internalPadding, sfxRowStartY + (volRowHeight - sliderHeight) / 2, sliderWidth, sliderHeight};
    int sfxSliderTrackW = std::max(1, sfxSliderTrackRectPause.w - sliderButtonW); float sfxPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getSfxVolume()) / MIX_MAX_VOLUME;
    sfxSliderButtonRectPause = { sfxSliderTrackRectPause.x + static_cast<int>(sfxPercent * sfxSliderTrackW), sfxSliderTrackRectPause.y + (sliderHeight - sliderButtonH) / 2, sliderButtonW, sliderButtonH };
    currentY = sfxRowStartY + volRowHeight + dynamicPaddingY;
    saveButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH }; currentY += btnBoxH + dynamicPaddingY;
    returnButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH };

    if (continueTextTex) { SDL_QueryTexture(continueTextTex, NULL, NULL, &continueTextRect.w, &continueTextRect.h); continueTextRect.x = continueButtonRect.x + (continueButtonRect.w - continueTextRect.w) / 2; continueTextRect.y = continueButtonRect.y + (continueButtonRect.h - continueTextRect.h) / 2; }
    if (saveTextTex) { SDL_QueryTexture(saveTextTex, NULL, NULL, &saveTextRect.w, &saveTextRect.h); saveTextRect.x = saveButtonRect.x + (saveButtonRect.w - saveTextRect.w) / 2; saveTextRect.y = saveButtonRect.y + (saveButtonRect.h - saveTextRect.h) / 2; }
    if (returnTextTex) { SDL_QueryTexture(returnTextTex, NULL, NULL, &returnTextRect.w, &returnTextRect.h); returnTextRect.x = returnButtonRect.x + (returnButtonRect.w - returnTextRect.w) / 2; returnTextRect.y = returnButtonRect.y + (returnButtonRect.h - returnTextRect.h) / 2; }
}

SDL_Texture* Game::renderPauseText(const std::string& text, SDL_Color color) {
    if (!pauseFont) { std::cerr << "Error: renderPauseText called but pauseFont is null!" << std::endl; return nullptr; }
    if (!renderer) { std::cerr << "Error: renderPauseText called but renderer is null!" << std::endl; return nullptr; }

    SDL_Surface* surface = TTF_RenderText_Blended(pauseFont, text.c_str(), color);
    if (!surface) { std::cerr << "Failed to render pause text surface ('" << text << "'): " << TTF_GetError() << std::endl; return nullptr; }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { std::cerr << "Failed to create pause text texture ('" << text << "'): " << SDL_GetError() << std::endl; }
    SDL_FreeSurface(surface);
    return texture;
}

std::vector<SDL_Rect> Game::getBuffButtonRects() {
    std::vector<SDL_Rect> rects;
    if (!renderer || currentBuffOptions.empty()) return rects;
    int windowWidth, windowHeight; SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    const int numButtons = std::min((int)currentBuffOptions.size(), 4);
    const int refWidth = 800, refHeight = 600; const int baseIconSize = 48, baseBoxW = 180, baseBoxH = 100, baseGap = 25, baseIconOffsetY = -100;
    float scaleX = static_cast<float>(windowWidth) / refWidth; float scaleY = static_cast<float>(windowHeight) / refHeight; float scaleFactor = std::min(scaleX, scaleY);
    int iconDrawSize = std::max(32, static_cast<int>(baseIconSize * scaleFactor));
    int boxW = std::max(100, static_cast<int>(baseBoxW * scaleFactor)); int boxH = std::max(60, static_cast<int>(baseBoxH * scaleFactor)); int gap = std::max(15, static_cast<int>(baseGap * scaleFactor));
    int iconOffsetY = windowHeight / 2 + static_cast<int>(baseIconOffsetY * scaleY);
    int boxOffsetY = iconOffsetY + iconDrawSize + std::max(3, static_cast<int>(5 * scaleFactor));
    int totalWidth = numButtons * boxW + (numButtons - 1) * gap; int startX = (windowWidth - totalWidth) / 2;
    rects.resize(numButtons);
    for (int i = 0; i < numButtons; ++i) { rects[i] = {startX + i * (boxW + gap), boxOffsetY, boxW, boxH}; }
    return rects;
}