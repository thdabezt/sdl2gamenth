// Fetched content of Game/src/game.cpp
#include "game.h"
#include <iostream>
#include <SDL.h>
#include "constants.h"
#include <SDL_image.h>
#include "TextureManager.h"
#include "map.h" // Include map.h for Map class
#include "ECS/Components.h"
#include "Vector2D.h"
#include "Collision.h"
#include "AssetManager.h" // Include AssetManager
#include "ECS/EnemyAi.h" // Include EnemyAi
#include <ctime>
#include <vector>
#include <cstdlib> // For std::rand()
#include <sstream>
#include "SaveLoadManager.h" // Include the manager



// Define the static instance
Game* Game::instance = nullptr;
// Map *map;
SDL_Event Game::event;
// std::vector<Vector2D> spawnPoints;
SDL_Renderer *Game::renderer = nullptr;
int Game::mouseX = 0;
int Game::mouseY = 0;
// Manager manager; // Note: This manager is global/static relative to this file. Consider ownership.
int Game::musicVolume = MIX_MAX_VOLUME / 2; // Default value
int Game::sfxVolume = MIX_MAX_VOLUME / 2;   // Default value
SDL_Rect Game::camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

// Note: AssetManager is static. Be mindful of its lifecycle if resetting games.
// AssetManager *Game::assets = new AssetManager(&manager);
bool Game::isRunning = false;
bool godmode = false;
// auto& player(manager.addEntity()); // REMOVED global player

// --- Game Constructor ---
Game::Game() {
    if (instance != nullptr) {
        std::cerr << "Warning: Multiple Game instances detected!" << std::endl;
    }
    instance = this;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    // Pointers initialized to nullptr via member initialization in .h or here
    playerEntity = nullptr;
    playerManager = nullptr;
    ui = nullptr;
    map = nullptr;
    assets = nullptr; // Ensure explicitly null before creation
    saveLoadManager = nullptr; // Ensure explicitly null before creation

    // Initialize AssetManager using the MEMBER manager
    assets = new AssetManager(&manager); // Uses the 'manager' member variable
    // Initialize SaveLoadManager
    saveLoadManager = new SaveLoadManager(this);
}


// --- Game Destructor ---
Game::~Game(){
    // Destructor calls clean()
    std::cout << "Game destructor (~Game) called." << std::endl;
    clean(); // clean() should delete saveLoadManager
    instance = nullptr; // Reset static instance pointer
}


// Full Game::init function from src/game.cpp with latest changes:

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen){
    // std::cout << "--- Entering Game::init ---" << std::endl; // LOG START

    // Use the renderer provided by main.cpp
    if (Game::renderer) {
        // std::cout << "Game::init - Using existing Renderer" << std::endl;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    } else {
        // std::cerr << "Error: Game::init called but Game::renderer is null!" << std::endl;
        isRunning = false;
        // Clean up potentially allocated members if init fails early
        if(saveLoadManager) { delete saveLoadManager; saveLoadManager = nullptr; }
        if(assets) { delete assets; assets = nullptr; }
        return;
    }
    currentState = GameState::Playing;
    // std::cout << "Game state initialized to Playing." << std::endl; // Your last successful log

    // --- Add Logging Here ---
    // std::cout << "Game::init - Setting volumes..." << std::endl;
    Mix_VolumeMusic(musicVolume);
    Mix_Volume(-1, sfxVolume);
    // std::cout << "Game::init - Refreshing manager..." << std::endl;
    manager.refresh();

    // --- Initialize Enemy Database ---
    // std::cout << "Game::init - Initializing enemy database..." << std::endl;
    initializeEnemyDatabase();
    // std::cout << "Game::init - Enemy database initialized (" << allEnemyDatabase.size() << " entries)." << std::endl;

    // std::cout << "Game::init - Loading base assets..." << std::endl;
    // --- Load Textures and Sounds ---
    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");
    // ... (load other assets using member 'assets') ...
     assets->AddTexture("fire", "sprites/projectile/fire.png");
     assets->AddTexture("starproj", "sprites/projectile/star.png");
     // --- Load EXP Orb Textures ---
     assets->AddTexture("exp_orb_1", "sprites/projectile/exp_orb1.png");     // Assuming this path and naming
     assets->AddTexture("exp_orb_10", "sprites/projectile/exp_orb10.png");
     assets->AddTexture("exp_orb_50", "sprites/projectile/exp_orb50.png");
     assets->AddTexture("exp_orb_100", "sprites/projectile/exp_orb100.png");
     assets->AddTexture("exp_orb_200", "sprites/projectile/exp_orb200.png"); // Added 200
     assets->AddTexture("exp_orb_500", "sprites/projectile/exp_orb500.png");

     assets->AddSoundEffect("gunshot_sound", "assets/sound/shot.wav");
     assets->AddMusic("level_music", "assets/sound/hlcbg.mp3"); // Should work if file exists now
     assets->AddSoundEffect("fire_spell_sound", "assets/sound/fire.wav"); // Should work if file exists now
     assets->AddSoundEffect("star_spell_sound", "assets/sound/star.wav"); // Should work if file exists now

     assets->AddTexture("pausebox", "assets/menu/pausebox.png");
     assets->AddTexture("buttonbox", "assets/menu/box.png"); // Reuse box.png
     assets->AddTexture("soundon", "assets/menu/soundon.png");
     assets->AddTexture("soundoff", "assets/menu/soundoff.png");
     assets->AddTexture("slidebar", "assets/menu/slidebar.png");
     assets->AddTexture("slidebutton", "assets/menu/slidebutton.png");
     assets->AddTexture("gameover", "assets/menu/gameover.png"); // Load game over image
     assets->AddSoundEffect("gameover_sfx", "assets/sound/gameover.wav");
      // --- ADDED: Load Menu SFX ---
    assets->AddSoundEffect("game_start", "assets/sound/start.wav");     // Assume path is correct
    assets->AddSoundEffect("button_click", "assets/sound/buttonclick.wav"); // Assume path is correct
    // --- Buff Icon
    assets->AddTexture("weapon_icon", "assets/menu/weaponicon.png");
    assets->AddTexture("fire_icon", "assets/menu/fireicon.png");
    assets->AddTexture("star_icon", "assets/menu/staricon.png");
    assets->AddTexture("health_icon", "assets/menu/healthicon.png");
    assets->AddTexture("lifesteal_icon", "assets/menu/lifestealicon.png"); // Assuming path
    assets->AddTexture("default_buff_icon", "assets/menu/defaultbufficon.png"); // Fallback icon

    int enemyTexLoaded = 0;
    int enemyTexFailed = 0;
    for (const auto& enemyData : allEnemyDatabase) {
        if (!enemyData.sprite) continue;
        assets->AddTexture(enemyData.tag, enemyData.sprite);
    }
    std::cout << "Game::init - Enemy textures loading finished (Loaded: " << enemyTexLoaded << ", Failed: " << enemyTexFailed << ")." << std::endl;
    if (enemyTexFailed > 0) {
         std::cerr << "!!! ERROR: Failed to load one or more enemy textures. Check paths in constants.h and file existence. !!!" << std::endl;
         // Maybe set isRunning = false and return?
    }
    delete ui; // Delete previous instance if any
    ui = new UIManager(renderer);
    if (ui) {
            ui->init(); // Now ui->init() runs AFTER AddTexture for icons
    } else {
            std::cerr << "Error: Failed to create UIManager!" << std::endl;
            // Handle error, maybe set isRunning = false; return;
    }
        // --- End UI Manager Init ---
    // --- Create Player Entity Structure ---
    playerEntity = &manager.addEntity();

    // --- Add ALL Player Components (Stats ones with Placeholders) ---
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
    // Add Health/Weapon/Spells with PLACEHOLDER values (e.g., 0 or 1)
    playerEntity->addComponent<HealthComponent>(1, 1); // Start with 1 health (avoid 0 max health)
      // Corrected WeaponComponent addComponent call (Removed extra range argument)
      playerEntity->addComponent<WeaponComponent>(
        "placeholder",      // weaponTag (string)
        0,                  // dmg (int)
        99999,              // rate (int)
        0.0f,               // speed (float)
        // 0,               // <<< DELETE THIS LINE (This was the extra range argument)
        0.0f,               // spread (float)
        1,                  // count (int)
        1,                  // size (int)
        "projectile",       // texId (string)
        1,                  // pierce (int)
        1,                  // burstCount (int)
        50                  // burst_Delay (int)
    );

    // Corrected SpellComponent addComponent calls (Should be okay from previous fix)
    playerEntity->addComponent<SpellComponent>(
        "placeholder_spell", // spellTag (string)
        5,                   // dmg (int)
        100,                 // cool (int)
        1.5f,                // speed (float)
        1,                   // count (int)
        16,                  // size (int)
        "fire",              // texId (string)
        SpellTrajectory::SPIRAL, // mode
        0.5f,                // growthRate (float)
        10                   // pierce (int)
    );
    playerEntity->addComponent<SpellComponent>(
        "placeholder_star",  // spellTag (string)
        0,                   // dmg (int)
        99999,               // cool (int)
        0.0f,                // speed (float)
        1,                   // count (int)
        1,                   // size (int)
        "starproj",          // texId (string)
        SpellTrajectory::RANDOM_DIRECTION, // mode
        0.0f,                // growthRate
        1                    // pierce
    );


    playerEntity->addGroup(groupPlayers);

    // --- Create Player Manager ---
    delete playerManager;
    playerManager = new Player(playerEntity); // PlayerManager uses the entity with placeholder components
        // --- Load Default Stats from File --- ADD THIS BLOCK
        if (saveLoadManager) {
            std::cout << "Attempting to load default stats from default.state..." << std::endl;
            if (!saveLoadManager->loadGameState("saves/default.state")) {
                std::cerr << "Warning: Failed to load default.state. Using component constructor defaults." << std::endl;
                // Optional: Add fallback explicit defaults here if default.state MUST exist
            } else {
                std::cout << "Successfully loaded stats from default.state." << std::endl;
            }
        } else {
            std::cerr << "Error: Cannot load default stats, SaveLoadManager is null!" << std::endl;
        }
        // Set default volumes if load failed
        Mix_VolumeMusic(Game::musicVolume);
        Mix_Volume(-1, Game::sfxVolume);
        // --- End Setting Default Stats ---


    // --- Load Map ---
    delete map;
    map = new Map(manager, "terrain", 1, 32);
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10, spawnPoints);



        // --- Pause init
        // --- Initialize Pause Menu Resources ---
        std::cout << "Initializing Pause Menu resources..." << std::endl;
        pauseFont = TTF_OpenFont("assets/font.ttf", 12); // Adjust size as needed
        if (!pauseFont) {
            std::cerr << "Failed to load pause font! SDL_ttf Error: " << TTF_GetError() << std::endl;
            // Use default font from UIManager as fallback?
            if(ui && ui->getFont()) pauseFont = ui->getFont();
        }
    
        // Get textures from Asset Manager
        pauseBoxTex = assets->GetTexture("pausebox");
        buttonBoxTex = assets->GetTexture("buttonbox");
        soundOnTex = assets->GetTexture("soundon");
        soundOffTex = assets->GetTexture("soundoff");
        sliderTrackTex = assets->GetTexture("slidebar");
        sliderButtonTex = assets->GetTexture("slidebutton");
    
        // Pre-render button text
        SDL_Color textColor = { 0, 0, 0, 255 }; // Black text
        continueTextTex = renderPauseText("Continue", textColor);
        saveTextTex = renderPauseText("Save Game", textColor);
        returnTextTex = renderPauseText("Return to Title", textColor);
    
        // Initialize pause menu volume state based on current game volume
        // isMusicMutedPause = (musicVolume == 0);
        // storedMusicVolumePause = isMusicMutedPause ? (MIX_MAX_VOLUME / 2) : musicVolume; // Store sensible value if muted
        // isSfxMutedPause = (sfxVolume == 0);
        // storedSfxVolumePause = isSfxMutedPause ? (MIX_MAX_VOLUME / 2) : sfxVolume;
        isDraggingBgmPause = false;
        isDraggingSfxPause = false;

    // --- Initialize Game Over Resources ---
    std::cout << "Initializing Game Over resources..." << std::endl;
    gameOverTex = assets->GetTexture("gameover");
    // Use pauseFont or load a specific one
    gameOverFont = pauseFont; // Reusing pause font for simplicity
    if (!gameOverFont) {
         std::cerr << "Error: Font not available for Game Over text!" << std::endl;
    } else {
         SDL_Color textColor = { 255, 255, 255, 220 }; // White, slightly transparent
         // Destroy previous texture if any
         if (gameOverTextTex) { SDL_DestroyTexture(gameOverTextTex); gameOverTextTex = nullptr; }
         // Use renderPauseText helper or direct calls
         SDL_Surface* surface = TTF_RenderText_Blended(gameOverFont, "Click anywhere to return to Title", textColor);
         if(surface) {
             gameOverTextTex = SDL_CreateTextureFromSurface(renderer, surface);
             SDL_FreeSurface(surface);
             if (!gameOverTextTex) { std::cerr << "Failed to create game over text texture!" << std::endl; }
         } else { std::cerr << "Failed to render game over text surface!" << std::endl; }
    }
    updateSpawnPoolAndWeights();
    // --- End Game Over Resources ---

    isRunning = true;
}

// --- Game::handleEvents (Main function using state machine) ---
void Game::handleEvents() {
    // Assuming event is polled in the main loop and passed to SceneManager,
    // which passes it to the active scene's handleEvents (e.g., GameScene::handleEvents),
    // which then calls this Game::handleEvents function.
    // The static Game::event might be redundant if the event is passed down.
    // We will use the static Game::event for now as previous code relies on it.

    // Handle global events first (Quit, F11)
    if (Game::event.type == SDL_QUIT) {
        isRunning = false; // Use the Game instance's isRunning flag
        return;
    }
    // F11 handled in main.cpp loop preferably

    // --- State-Specific Input Handling ---
    switch (currentState) {
        case GameState::Playing:
            // Handle keys that work only during active play
            if (Game::event.type == SDL_KEYDOWN) {
                if (Game::event.key.keysym.sym == SDLK_ESCAPE) { // Pause Key
                    togglePause();
                    return; // Consume pause event
                }
                if (Game::event.key.keysym.sym == SDLK_l) { // Pause Key
                    playerManager->levelUp();
                    return; // Consume pause event
                }
            }
            
            // Update mouse world coordinates only when playing
            int mouseX_Screen, mouseY_Screen;
            SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
            Game::mouseX = mouseX_Screen + Game::camera.x;
            Game::mouseY = mouseY_Screen + Game::camera.y;
            // Player movement input is handled by KeyboardController.update()
            break; // End Playing state handling

            case GameState::Paused:
            if (isInBuffSelection) {
                 if (Game::event.type == SDL_MOUSEBUTTONDOWN) {
                    //  std::cout << "DEBUG: Buff Selection - MOUSEBUTTONDOWN received." << std::endl; // Existing log
                     if (Game::event.button.button == SDL_BUTTON_LEFT) {
                         int mouseX_Screen_Buff, mouseY_Screen_Buff; // Use specific names
                         SDL_GetMouseState(&mouseX_Screen_Buff, &mouseY_Screen_Buff);

                         // --- Log Mouse Coords ---
                        //  std::cout << "DEBUG: Checking Click at Screen Coords: (" << mouseX_Screen_Buff << ", " << mouseY_Screen_Buff << ")" << std::endl;
                         // ------------------------

                         std::vector<SDL_Rect> buffButtonRects = getBuffButtonRects();

                         for (size_t i = 0; i < buffButtonRects.size() && i < currentBuffOptions.size(); ++i) {
                             SDL_Rect buttonRect = buffButtonRects[i]; // Use the calculated rect

                             // --- Log Button Rect ---
                            //  std::cout << "DEBUG: Checking Button [" << i << "] Rect: x=" << buttonRect.x
                            //            << ", y=" << buttonRect.y << ", w=" << buttonRect.w << ", h=" << buttonRect.h << std::endl;
                             // -----------------------

                             if (ui && ui->isMouseInside(mouseX_Screen_Buff, mouseY_Screen_Buff, buttonRect)) {
                                //  std::cout << "DEBUG: Click HIT on Button " << i << "! Applying buff..." << std::endl; // Existing log
                                 applySelectedBuff(static_cast<int>(i));
                                 return;
                             }
                         }
                         // Add log if no button was hit after checking all
                         std::cout << "DEBUG: Click did not register inside any calculated button rect." << std::endl;
                     }
                 } else if (Game::event.type == SDL_KEYDOWN) {
                      // --- ADDED: Log Key Down Event ---
                    //   std::cout << "DEBUG: Buff Selection - KEYDOWN received. Key: " << SDL_GetKeyName(Game::event.key.keysym.sym) << std::endl;
                     switch (Game::event.key.keysym.sym) {
                         case SDLK_1:
                            //   std::cout << "DEBUG: Key '1' pressed. Applying buff 0..." << std::endl;
                              applySelectedBuff(0); return;
                         case SDLK_2:
                            //   std::cout << "DEBUG: Key '2' pressed. Applying buff 1..." << std::endl;
                              applySelectedBuff(1); return;
                         case SDLK_3:
                            //   std::cout << "DEBUG: Key '3' pressed. Applying buff 2..." << std::endl;
                              applySelectedBuff(2); return;
                         case SDLK_4:
                            //   std::cout << "DEBUG: Key '4' pressed. Applying buff 3..." << std::endl;
                              applySelectedBuff(3); return;
                         default:
                            //   std::cout << "DEBUG: Key pressed is not 1-4." << std::endl;
                              break; // Don't apply buff for other keys
                     }
                 } else {
                      // --- ADDED: Log other events received during buff selection ---
                     // std::cout << "DEBUG: Buff Selection - Received non-input event type: " << Game::event.type << std::endl; // Can be very noisy
                 }

                 // If we are paused AND in buff selection, consume the event here
                 // so it doesn't fall through to the pause menu UI handling below.
                 return; // Consume event

            } else { // Paused, but NOT in buff selection
                 // Handle pause menu UI input
                 handlePauseMenuEvents(); // Use helper function
            }
            break; // End Paused state handling

        case GameState::GameOver:
            // Handle input specific to the GAME OVER screen
            if (Game::event.type == SDL_MOUSEBUTTONDOWN && Game::event.button.button == SDL_BUTTON_LEFT) {
                std::cout << "Game Over screen clicked. Returning to Title." << std::endl;
                // Game state will reset when GameScene::init runs again
                if(SceneManager::instance) {
                     SceneManager::instance->switchToScene(SceneType::Menu);
                } else {
                     std::cerr << "Error: SceneManager::instance is null, cannot return to title from Game Over!" << std::endl;
                     // Fallback? Maybe just quit?
                     // isRunning = false;
                }
                return; // Consume click event
            }
            // Ignore other input during Game Over
            break; // End GameOver state handling
    } // End switch (currentState)

} // End Game::handleEvents


// --- ADD/REPLACE Helper: Handle Pause Menu Events ---
// Add or Replace this helper function definition in game.cpp

void Game::handlePauseMenuEvents() {
    int mouseX_Screen, mouseY_Screen;
    SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen); // Use screen coordinates for UI clicks
    SDL_Point mousePoint = {mouseX_Screen, mouseY_Screen};

    // Use the static Game::event for input processing
    SDL_Event& currentEvent = Game::event; // Use a reference for convenience

    // Handle Mouse Clicks for Pause Menu UI
    if (currentEvent.type == SDL_MOUSEBUTTONDOWN && currentEvent.button.button == SDL_BUTTON_LEFT) {
        bool clickHandled = false; // Flag to prevent multiple sounds per click

        // Helper lambda to play click sound via AssetManager
        auto playPauseClickSound = [&]() {
            if (!clickHandled && Game::instance && Game::instance->assets) { // Check instance too
                Mix_Chunk* chunk = Game::instance->assets->GetSoundEffect("button_click");
                if (chunk) {
                    Mix_PlayChannel(-1, chunk, 0);
                    clickHandled = true; // Mark sound as played for this click event
                } else {
                     std::cerr << "Warning: Could not get 'button_click' sound from AssetManager." << std::endl;
                }
            }
        };

        // Continue Button
        if (SDL_PointInRect(&mousePoint, &continueButtonRect)) {
            playPauseClickSound();
            togglePause(); // Unpause the game
            return; // Handled
        }
        // Save Button
        if (SDL_PointInRect(&mousePoint, &saveButtonRect)) {
            playPauseClickSound();
            if (saveLoadManager) {
                saveLoadManager->saveGameState(); // Save with timestamp
                std::cout << "Game Saved (Timestamped)." << std::endl;
            } else {
                 std::cerr << "Error: Cannot save, SaveLoadManager is null!" << std::endl;
            }
            return; // Handled (stay paused)
        }
        // Return to Title Button
        if (SDL_PointInRect(&mousePoint, &returnButtonRect)) {
            playPauseClickSound();
            togglePause(); // Unpause before switching might be cleaner
            if(SceneManager::instance) {
                 SceneManager::instance->switchToScene(SceneType::Menu);
            } else {
                 std::cerr << "Error: SceneManager::instance is null, cannot return to title!" << std::endl;
            }
            return; // Handled
        }
        // Mute Icons (Use static functions, manage stored value here)
        if (SDL_PointInRect(&mousePoint, &bgmIconRectPause)) {
            playPauseClickSound();
            bool wasMuted = (Game::musicVolume == 0);
            if (!wasMuted) { // Muting
                storedMusicVolumePause = Game::musicVolume; // Store current before muting
                Game::setMusicVolume(0);
            } else { // Unmuting
                // Restore using stored value. If stored was 0, set to a default level.
                Game::setMusicVolume(storedMusicVolumePause > 0 ? storedMusicVolumePause : MIX_MAX_VOLUME / 4);
                // Update stored value in case we unmuted to the default level
                storedMusicVolumePause = Game::getMusicVolume();
            }
            calculatePauseLayout(); // Update slider button pos
            return; // Handled
        }
         if (SDL_PointInRect(&mousePoint, &sfxIconRectPause)) {
            playPauseClickSound();
            bool wasMuted = (Game::sfxVolume == 0);
             if (!wasMuted) { // Muting
                storedSfxVolumePause = Game::sfxVolume; // Store current before muting
                Game::setSfxVolume(0);
            } else { // Unmuting
                Game::setSfxVolume(storedSfxVolumePause > 0 ? storedSfxVolumePause : MIX_MAX_VOLUME / 4);
                storedSfxVolumePause = Game::getSfxVolume();
            }
            calculatePauseLayout(); // Update slider button pos
            return; // Handled
        }
        // Slider Button Drag Start
        if (SDL_PointInRect(&mousePoint, &bgmSliderButtonRectPause)) {
             playPauseClickSound(); // Sound on initial click
             isDraggingBgmPause = true;
             sliderDragXPause = mouseX_Screen - bgmSliderButtonRectPause.x;
             return; // Handled
        }
         if (SDL_PointInRect(&mousePoint, &sfxSliderButtonRectPause)) {
             playPauseClickSound(); // Sound on initial click
             isDraggingSfxPause = true;
             sliderDragXPause = mouseX_Screen - sfxSliderButtonRectPause.x;
             return; // Handled
        }
    }
    // Handle Mouse Button Up (Stop Dragging)
    else if (currentEvent.type == SDL_MOUSEBUTTONUP && currentEvent.button.button == SDL_BUTTON_LEFT) {
         if (isDraggingBgmPause || isDraggingSfxPause) {
              isDraggingBgmPause = false;
              isDraggingSfxPause = false;
         }
    }
    // Handle Mouse Motion (Slider Drag)
    else if (currentEvent.type == SDL_MOUSEMOTION) {
        int w, h; // Get window size for layout recalculation
        if(Game::renderer) SDL_GetRendererOutputSize(Game::renderer, &w, &h); else { w = 800; h = 600;}

         if (isDraggingBgmPause) {
              int trackX = bgmSliderTrackRectPause.x;
              int trackButtonW = bgmSliderButtonRectPause.w;
              int trackW = bgmSliderTrackRectPause.w - trackButtonW;
              trackW = std::max(1, trackW);
              int targetButtonX = mouseX_Screen - sliderDragXPause;
              targetButtonX = std::max(trackX, std::min(targetButtonX, trackX + trackW));
              float percent = static_cast<float>(targetButtonX - trackX) / trackW;
              int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
              if (newVolume != Game::musicVolume) { // Compare with static volume
                   Game::setMusicVolume(newVolume); // Use static setter
                   // Update stored volume only if unmuting via slider
                   if (newVolume > 0) storedMusicVolumePause = newVolume;
                   calculatePauseLayout(); // Update button visual
              }
         }
          else if (isDraggingSfxPause) {
               int trackX = sfxSliderTrackRectPause.x;
               int trackButtonW = sfxSliderButtonRectPause.w;
               int trackW = sfxSliderTrackRectPause.w - trackButtonW;
               trackW = std::max(1, trackW);
               int targetButtonX = mouseX_Screen - sliderDragXPause;
               targetButtonX = std::max(trackX, std::min(targetButtonX, trackX + trackW));
               float percent = static_cast<float>(targetButtonX - trackX) / trackW;
               int newVolume = static_cast<int>(std::round(percent * MIX_MAX_VOLUME));
               if (newVolume != Game::sfxVolume) { // Compare with static volume
                    Game::setSfxVolume(newVolume); // Use static setter
                    if (newVolume > 0) storedSfxVolumePause = newVolume;
                    calculatePauseLayout(); // Update button visual
               }
         }
    }
    // Handle Key Presses for Pause Menu
    else if (currentEvent.type == SDL_KEYDOWN) {
        switch (currentEvent.key.keysym.sym) {
            
        // --- END DEBUG ---
            case SDLK_ESCAPE: // Resume via 'Escape'
                togglePause();
                // Use break or return depending on whether other keys might be processed later
                // If only Escape/E unpauses, return is fine.
                return; // Consume event and exit
            // Add number keys or Up/Down selection for buttons later if desired
            default:
                break; // Ignore other keys
        }
    }
} // End handlePauseMenuEvents



// --- Modified Game::spawnEnemy Implementation ---
void Game::spawnEnemy() {
    // 1. Checks (Player, Spawn Points)
    if (!playerEntity || !playerManager) {
        std::cerr << "Cannot spawn enemy, player setup incomplete!" << std::endl;
        return;
    }
    if (spawnPoints.empty()) {
        std::cerr << "No spawn points available!" << std::endl;
        return;
    }

    // 2. Select Enemy Type using Weighted Random Selection
    // updateSpawnPoolAndWeights(); // Consider calling this less frequently (e.g., on level up) unless needed every spawn
    EnemySpawnInfo* selectedEnemyInfo = selectEnemyBasedOnWeight();
    if (!selectedEnemyInfo) {
        std::cerr << "Failed to select an enemy from the pool!" << std::endl;
        return;
    }

    // 3. Choose Random Spawn Point
    int randomIndex = std::rand() % spawnPoints.size();
    Vector2D spawnPosition = spawnPoints[randomIndex];

    // 4. Create Entity
    auto& enemy = manager.addEntity();

    // 5. Ensure Texture is Loaded (Should be handled in init)
    // assets->AddTexture(selectedEnemyInfo->tag, selectedEnemyInfo->sprite); // Redundant if done in init

    // 6. Apply Enemy Scaling (Health & Damage)
    // --- Apply Enemy Scaling (Health & Damage) ---
    float healthDmgModifier = 1.0f;
    int playerLevel = playerManager->getLevel();

    if (playerLevel >= 50) {
        int scalingSteps = (playerLevel - 50) / 5;
        // Apply 20% compound increase for each 5 levels above 50
        healthDmgModifier = std::pow(1.20f, scalingSteps);
    }

    // Ensure the first argument to std::max is also a float (1.0f)
    int finalHealth = static_cast<int>(std::max(1.0f, selectedEnemyInfo->baseHealth * healthDmgModifier)); // Use 1.0f
    int finalDamage = static_cast<int>(std::max(1.0f, selectedEnemyInfo->baseDamage * healthDmgModifier)); // Use 1.0f
    // Note: Speed and Experience currently don't scale per request


    // 7. Add Components using selected info and scaled stats
    int enemySpriteWidth = 64; // Assume default size, adjust if enemies have different dimensions
    int enemySpriteHeight = 64;
    // You might need a way to get specific dimensions if they vary significantly
    // e.g., query texture size or store dimensions in EnemySpawnInfo

    enemy.addComponent<TransformComponent>(spawnPosition.x, spawnPosition.y, enemySpriteWidth, enemySpriteHeight, 2); // Assuming scale 2
    enemy.addComponent<SpriteComponent>(selectedEnemyInfo->tag, true); // Animated = true
    enemy.addComponent<ColliderComponent>(selectedEnemyInfo->tag, 64, 64); // Adjust collider size as needed (e.g., 32x37 used for player?)
    enemy.addComponent<HealthComponent>(finalHealth, finalHealth); // Use scaled health

    // Ensure player's TransformComponent exists before getting its position pointer
    if (!playerEntity->hasComponent<TransformComponent>()) {
         std::cerr << "ERROR in spawnEnemy: Player missing TransformComponent! Cannot create AI." << std::endl;
         enemy.destroy(); // Clean up the partially created enemy
         return;
    }
    Vector2D* playerPosPtr = &playerEntity->getComponent<TransformComponent>().position;

    enemy.addComponent<EnemyAIComponent>(
        5000, // Detection range
        selectedEnemyInfo->speed, // Base speed (not scaled)
        playerPosPtr,             // Pointer to player position
        finalDamage,              // Use scaled contact damage
        selectedEnemyInfo->baseExperience, // Base experience value (not scaled)
        playerEntity              // Pointer to player entity
    );

    enemy.addGroup(groupEnemies); // Add to the correct group

    // Optional: Log the spawn
    // std::cout << "Spawned " << selectedEnemyInfo->tag << " (L" << playerLevel << ") HP:" << finalHealth << " DMG:" << finalDamage << " at (" << spawnPosition.x << "," << spawnPosition.y << ")" << std::endl;
}



void Game::update(){
    // --- Check Game State ---
    // Only update gameplay elements if playing
    if (currentState != GameState::Playing) {
            // Still allow manager refresh? Maybe not if completely frozen.
            // manager.refresh(); // Optional: refresh even if paused/gameover?
        return; // Skip updates if Paused or GameOver
    }
    // If we reach here, currentState == GameState::Playing

    if (!isRunning || !playerEntity) return; // Check running state and player

    // --- Use member 'manager' ---
    manager.refresh();

    Uint32 currentTime = SDL_GetTicks();
    if(currentState != GameState::Paused && !isInBuffSelection){ // Check buff selection state too

        // --- Get groups using member 'manager' ---
        auto& colliders = manager.getGroup(Game::groupColliders);
        auto& projectiles = manager.getGroup(Game::groupProjectiles);
        auto& enemies = manager.getGroup(Game::groupEnemies);
        auto& expOrbs = manager.getGroup(Game::groupExpOrbs); // Needed for ExpOrbComponent update

        // Update player pos (needed for camera, potentially other logic)
        Vector2D playerPosStart; // Store position before manager.update()
        if(playerEntity->hasComponent<TransformComponent>()) { // Check component exists
            playerPosStart = playerEntity->getComponent<TransformComponent>().position;
        }

        // --- Use member 'manager' to update ---
        manager.update(); // Update all active entities and components

        // Collision Handling needs player Collider & Transform
        if (!playerEntity->hasComponent<ColliderComponent>() || !playerEntity->hasComponent<TransformComponent>()) {
             std::cerr << "Error in Game::update: Player missing Collider or Transform for collision checks!" << std::endl;
             return; // Cannot do collisions without these
        }
        ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();
        TransformComponent& playerTransform = playerEntity->getComponent<TransformComponent>();
        SDL_Rect playerCol = playerCollider.collider;



        // Player vs Terrain collision
        for (auto* c : colliders) { // Iterate over pointers
             if (!c || !c->isActive() || !c->hasComponent<ColliderComponent>()) continue; // Safety checks
            ColliderComponent& obstacleCollider = c->getComponent<ColliderComponent>();
            // Check tag AFTER getting component
            if (obstacleCollider.tag != "terrain") continue;

            SDL_Rect cCol = obstacleCollider.collider;
            if (Collision::AABB(playerCol, cCol)) {
                // Resolve collision (push-out logic from before)
                 float overlapX = 0.0f, overlapY = 0.0f;
                 Vector2D centerPlayer(playerCol.x + playerCol.w / 2.0f, playerCol.y + playerCol.h / 2.0f);
                 Vector2D centerObstacle(cCol.x + cCol.w / 2.0f, cCol.y + cCol.h / 2.0f);
                 overlapX = (playerCol.w / 2.0f + cCol.w / 2.0f) - std::abs(centerPlayer.x - centerObstacle.x);
                 overlapY = (playerCol.h / 2.0f + cCol.h / 2.0f) - std::abs(centerPlayer.y - centerObstacle.y);

                 if (overlapX > 0 && overlapY > 0) {
                     if (overlapX < overlapY) {
                         playerTransform.position.x += (centerPlayer.x < centerObstacle.x ? -overlapX : overlapX);
                     } else {
                         playerTransform.position.y += (centerPlayer.y < centerObstacle.y ? -overlapY : overlapY);
                     }
                     playerCollider.update(); // Update collider component immediately
                     playerCol = playerCollider.collider; // Update local SDL_Rect
                 }
            }
        }

        // Projectile vs Enemy collision
        for (auto* p : projectiles) { // Iterate over pointers
             if (!p || !p->isActive() || !p->hasComponent<ColliderComponent>() || !p->hasComponent<ProjectileComponent>() || !p->hasComponent<TransformComponent>()) continue;
             SDL_Rect projectileCollider = p->getComponent<ColliderComponent>().collider;
             ProjectileComponent& projComp = p->getComponent<ProjectileComponent>();
            
             for (auto* e : enemies) { // Iterate over pointers
                 if (!e || !e->isActive() || !e->hasComponent<ColliderComponent>()) continue;
                 if (projComp.hasHit(e)) continue; // Check before getting collider

                 SDL_Rect enemyCollider = e->getComponent<ColliderComponent>().collider;
                 if (Collision::AABB(enemyCollider, projectileCollider)) {
                     // ... (Damage, Knockback, Hit Tint, Death logic - mostly unchanged) ...
                     int damage = projComp.getDamage();
                      if (e->hasComponent<TransformComponent>()) { /* Knockback */
                          Vector2D knockbackDir = p->getComponent<TransformComponent>().velocity.Normalize();
                          float knockbackForce = 35.0f;
                          e->getComponent<TransformComponent>().position.x += knockbackDir.x * knockbackForce;
                          e->getComponent<TransformComponent>().position.y += knockbackDir.y * knockbackForce;
                          if(e->hasComponent<ColliderComponent>()) e->getComponent<ColliderComponent>().update();
                      }
                      if (e->hasComponent<SpriteComponent>()) { /* Hit Tint */
                         e->getComponent<SpriteComponent>().isHit = true;
                         e->getComponent<SpriteComponent>().hitTime = currentTime;
                      }// --- Apply Lifesteal to Player ---
                    if (playerManager) { // Ensure playerManager is valid
                        float lifestealPercent = playerManager->getLifestealPercentage();
                        if (lifestealPercent > 0 && damage > 0) {
                            int healAmount = static_cast<int>(std::round(damage * (lifestealPercent / 100.0f)));
                            if (healAmount > 0) {
                                playerManager->heal(healAmount); // Heal the player
                            }
                        }
                    }
                    if (e->hasComponent<HealthComponent>()) {
                        e->getComponent<HealthComponent>().takeDamage(damage); // Apply damage

                        // --- Check for Enemy Death ---
                        if (e->getComponent<HealthComponent>().getHealth() <= 0) {

                             // --- Calculate EXP based on new formula ---
                             int finalExp = 0; // Default EXP
                             int baseExp = 0;
                             int scaledMaxHp = 0;
                             int playerLevel = 1; // Default level

                             if (e->hasComponent<EnemyAIComponent>()) {
                                  baseExp = e->getComponent<EnemyAIComponent>().getExpValue(); // Get base EXP
                             } else {
                                //   std::cerr << "Warning: Defeated enemy missing EnemyAIComponent, cannot get base EXP." << std::endl;
                             }

                             // We already know HealthComponent exists from the outer check
                             scaledMaxHp = e->getComponent<HealthComponent>().getMaxHealth(); // Get the actual max HP (already scaled when enemy spawned)

                             if (playerManager) {
                                  playerLevel = playerManager->getLevel(); // Get current player level
                             } else {
                                //   std::cerr << "Warning: playerManager is null, cannot get player level for EXP calc." << std::endl;
                             }

                             // Apply the formula: base + (maxHP/100) + (level/5)
                             // Ensure integer division is handled as intended (truncates decimals)
                             finalExp = baseExp + (scaledMaxHp / 100) + (playerLevel / 5);
                             finalExp = std::max(1, finalExp); // Ensure at least 1 EXP is granted

                            //  std::cout << "Enemy Defeated! BaseExp: " << baseExp << ", ScaledMaxHP: " << scaledMaxHp << ", PlayerLvl: " << playerLevel << " -> FinalExp: " << finalExp << std::endl;
                             // --- End EXP Calculation ---


                             // --- Spawn Exp Orb using calculated finalExp ---
                             if (playerManager && e->hasComponent<TransformComponent>()) { // Use Transform for position now
                                  Vector2D deathPosition = e->getComponent<TransformComponent>().position;
                                  // Adjust position slightly if needed (e.g., center of transform)
                                  deathPosition.x += (e->getComponent<TransformComponent>().width * e->getComponent<TransformComponent>().scale) / 2.0f;
                                  deathPosition.y += (e->getComponent<TransformComponent>().height * e->getComponent<TransformComponent>().scale) / 2.0f;

                                  // --- Select Orb Texture based on finalExp ---
                                     std::string orbTextureId = "exp_orb_1"; // Default to smallest
                                     if (finalExp >= 500) {
                                         orbTextureId = "exp_orb_500";
                                     } else if (finalExp >= 200) { // Check thresholds from highest to lowest
                                         orbTextureId = "exp_orb_200";
                                     } else if (finalExp >= 100) {
                                         orbTextureId = "exp_orb_100";
                                     } else if (finalExp >= 50) {
                                         orbTextureId = "exp_orb_50";
                                     } else if (finalExp >= 10) {
                                         orbTextureId = "exp_orb_10";
                                     }
                                     // If less than 10, it remains "exp_orb_1"
                                     // --- End Orb Texture Selection ---


                                     auto& orb = manager.addEntity();
                                     // Use the selected orbTextureId
                                     orb.addComponent<TransformComponent>(deathPosition.x, deathPosition.y, 16, 16, 1);
                                     orb.addComponent<SpriteComponent>(orbTextureId); // Use the selected texture ID
                                     orb.addComponent<ColliderComponent>("exp_orb", 16, 16);
                                     orb.addComponent<ExpOrbComponent>(finalExp);
                                }
                                // --- End Orb Spawning ---

                             if(playerManager) playerManager->incrementEnemiesDefeated();
                             // Note: The enemy entity is destroyed automatically by HealthComponent::takeDamage when health reaches 0 if it's not the player.
                        }
                        // --- End Death Check ---
                    } // End if has HealthComponent check

                  projComp.recordHit(e); // Record hit regardless of death
                  if (projComp.shouldDestroy()) {
                       p->destroy();
                       break; // Exit inner enemy loop if projectile is destroyed
                  }
              } // End AABB check
          } // End enemy loop
          // if (!p->isActive()) continue; // This line might cause issues if break is used above. Check logic flow. Better to let outer loop handle inactive 'p'.
     } // End projectile loop


        // Enemy Spawning (consider moving timer logic outside update if needed)
        Uint32 spawnInterval = 1000; // Default to 1 second (for level 50+)
        if (playerManager) { // Check if playerManager exists
            int currentLevel = playerManager->getLevel();
            if (currentLevel <= 20) {
                spawnInterval = 3000; // 3 seconds for levels 1-20
            } else if (currentLevel <= 50) {
                spawnInterval = 2000; // 2 seconds for levels 21-50
            }
            // Levels 50+ use the default 1000ms (1 second)
        } else {
            // Fallback if playerManager is somehow null (shouldn't happen)
            spawnInterval = 3000;
        }

        if (currentTime > lastEnemySpawnTime + spawnInterval) {
            spawnEnemy(); // Uses member manager
            lastEnemySpawnTime = currentTime;
        }

        
        // --- Camera Update (uses playerTransform directly) ---
        int currentWindowWidth, currentWindowHeight;
        // Get the actual current output size of the renderer
        if (renderer) { // Ensure renderer is valid
            SDL_GetRendererOutputSize(renderer, &currentWindowWidth, &currentWindowHeight);
        } else {
            // Fallback or error handling if renderer is somehow null
            currentWindowWidth = WINDOW_WIDTH; // Use constants as a fallback
            currentWindowHeight = WINDOW_HEIGHT;
            std::cerr << "Warning: Game::renderer is null during camera update!" << std::endl;
        }

        // Update the camera's dimensions to match the current window size
        camera.w = currentWindowWidth;
        camera.h = currentWindowHeight;

        // Center the camera on the player using the current window dimensions
        camera.x = static_cast<int>(playerTransform.position.x - (currentWindowWidth / 2.0f));
        camera.y = static_cast<int>(playerTransform.position.y - (currentWindowHeight / 2.0f));

        // Clamp Camera to map bounds (using updated camera.w and camera.h)
        if (camera.x < 0) camera.x = 0;
        if (camera.y < 0) camera.y = 0;
        // Ensure map and TILE_SIZE are accessible or calculate map dimensions appropriately
        // Assuming TILE_SIZE and MAP_WIDTH/HEIGHT are available from constants.h or map object
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE;   // Defined in constants.h
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE; // Defined in constants.h
        if (camera.x > mapPixelWidth - camera.w) camera.x = mapPixelWidth - camera.w;
        if (camera.y > mapPixelHeight - camera.h) camera.y = mapPixelHeight - camera.h;

        // --- End Camera Update ---

    } else {
        // Game is paused
    }
    // --- Check for Player Death AFTER updates and collisions ---
    if (playerEntity && playerEntity->hasComponent<HealthComponent>() && playerEntity->getComponent<HealthComponent>().getHealth() <= 0) {
        std::cout << "Player died! Switching to GameOver state." << std::endl;
        currentState = GameState::GameOver; // Change state
        Mix_HaltMusic(); // Stop game music
        // --- ADDED: Play Game Over Sound Effect ---
        if (playerEntity->hasComponent<SoundComponent>()) {
            // Ensure the sound component exists before trying to play
            playerEntity->getComponent<SoundComponent>().playSoundEffect("gameover_sfx");
            // std::cout << "DEBUG: Played gameover_sfx" << std::endl; // Optional debug log
        } else {
             std::cerr << "Warning: Player has no SoundComponent to play game over SFX!" << std::endl;
        }
        // --- END ADDED ---
    }
    // --- End Death Check ---
} // End Game::update




// Updated clean method (called by destructor)
// --- Modify Game::clean ---

void Game::clean(){
    std::cout << "Game::clean() called." << std::endl;

    // Destroy entities via manager BEFORE deleting other resources they might use
    // Mark player first if it exists
    if (playerEntity) {
        playerEntity->destroy();
        playerEntity = nullptr;
    }
    // Mark all other entities for destruction (optional, refresh should handle)
    // for(auto& entityVec : manager.getAllEntities()) { // Assuming Manager has such a method
    //    for(auto& entity : entityVec) { if(entity) entity->destroy(); }
    // }
    manager.refresh(); // Process destruction flags and delete entities

    // Clean up dynamically allocated members
     if (map) {
         delete map;
         map = nullptr;
     }
     if (ui) {
         delete ui;
         ui = nullptr;
     }
     if (playerManager) {
         delete playerManager;
         playerManager = nullptr;
     }
     // Delete SaveLoadManager
     if (saveLoadManager) {
         delete saveLoadManager;
         saveLoadManager = nullptr;
     }
      // Delete AssetManager (now a member)
      if (assets) {
          delete assets;
          assets = nullptr;
      }
    // --- ADDED: Clean up pause menu resources ---
    if (pauseFont) { TTF_CloseFont(pauseFont); pauseFont = nullptr; }
    // Textures are likely managed by AssetManager, but if loaded directly or pre-rendered:
    if (continueTextTex) { SDL_DestroyTexture(continueTextTex); continueTextTex = nullptr; }
    if (saveTextTex) { SDL_DestroyTexture(saveTextTex); saveTextTex = nullptr; }
    if (returnTextTex) { SDL_DestroyTexture(returnTextTex); returnTextTex = nullptr; }
    // Textures obtained via assets->GetTexture() don't need to be destroyed here.
    // pauseBoxTex, buttonBoxTex, soundOnTex, etc. will be cleaned by ~AssetManager()
    // --- END ADDED ---
    if (gameOverTextTex) { SDL_DestroyTexture(gameOverTextTex); gameOverTextTex = nullptr; }

    // Window, Renderer, SDL subsystems are cleaned in main.cpp
    std::cout<< "Game instance resources cleaned."<< std::endl;
}



void Game::rezero(){ // This might be obsolete or need rethinking with load game
    if (playerEntity) { // Check if player exists
        playerEntity->getComponent<TransformComponent>().position.x = 400;
        playerEntity->getComponent<TransformComponent>().position.y = 320;
        godmode = true; // If godmode is still used
    }
}




// Updated getPlayer - returns reference, ensure playerEntity is valid
Entity& Game::getPlayer() {
    if (!playerEntity) {
         // This case should ideally not happen if code flow is correct.
         // Throw an exception or handle error robustly.
         throw std::runtime_error("Error: Game::getPlayer() called when playerEntity is null!");
    }
    return *playerEntity;
}

// renderHealthBar remains largely the same, uses the passed entity
void Game::renderHealthBar(Entity& entity, Vector2D position) {
    if (!entity.hasComponent<HealthComponent>()) {
        // std::cout << "Entity missing HealthComponent!" << std::endl; // Reduce log spam
        return;
    }

    const HealthComponent& health = entity.getComponent<HealthComponent>();
    if (health.getHealth() <= 0 || health.getHealth() == health.getMaxHealth()) return; // Don't render for dead or full health entities


    // Calculate health bar dimensions and position
    int barWidth = 40;
    int barHeight = 5;
    // Position relative to entity's transform (adjust offsets as needed)
    if (!entity.hasComponent<TransformComponent>()) return; // Need transform for position
    

    int xPos = static_cast<int>(position.x) - camera.x + (entity.getComponent<ColliderComponent>().collider.w / 2) - (barWidth / 2); // Centered above collider
    int yPos = static_cast<int>(position.y) - camera.y - 15; // Position above the entity

    // Simple visibility check (could be refined)
    if (xPos < -barWidth || xPos > WINDOW_WIDTH ||
        yPos < -barHeight || yPos > WINDOW_HEIGHT) {
        return; // Don't render if likely offscreen
    }

    // Draw background (black or dark gray)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect bgRect = {xPos - 1, yPos - 1, barWidth + 2, barHeight + 2};
    SDL_RenderFillRect(renderer, &bgRect);

    // Calculate health percentage
    float healthPercent = static_cast<float>(health.getHealth()) / health.getMaxHealth();
    int currentBarWidth = static_cast<int>(barWidth * healthPercent);

    // Draw health bar (green to red gradient)
    int r = static_cast<int>(255 * (1.0f - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, barHeight};
    SDL_RenderFillRect(renderer, &healthRect);
}


// In game.cpp:


void Game::render(){
    // Ensure renderer exists
    if (!renderer) {
         std::cerr << "Error: Game::render called but renderer is null!" << std::endl;
         return;
    }

    // Start with a default clear color (e.g., black)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    // --- Render based on state ---
    // Render game world elements first, potentially dimmed if Paused or GameOver
    if (currentState == GameState::Playing || currentState == GameState::Paused || currentState == GameState::GameOver) {
         bool dimWorld = (currentState == GameState::Paused || currentState == GameState::GameOver);

         // --- Render Gameplay Elements ---
         // Set dimming blend mode if needed (applied later with overlay)
         // For now, render elements normally. The overlay achieves the dimming.

         // Tiles first
         for(auto* t : manager.getGroup(Game::groupMap)) if(t && t->isActive()) t->draw();

         // Other gameplay elements
         for(auto* o : manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->draw();
         for(auto* p : manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->draw();
         for(auto* e : manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->draw(); // Draw enemy sprites
         if(playerEntity && playerEntity->isActive()) playerEntity->draw(); // Draw player sprite

         // Health Bars (on top of characters)
         for(auto* e : manager.getGroup(Game::groupEnemies)) {
             if (e && e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>()) {
                 renderHealthBar(*e, e->getComponent<ColliderComponent>().position);
             }
         }
         // --- End Render Gameplay Elements ---


         // Render HUD / Gameplay UI (only if Playing)
         if (currentState == GameState::Playing && ui && playerManager) {
              ui->renderUI(playerManager);
         }


         // --- Render Pause State UI ---
         if (currentState == GameState::Paused) {
             // Draw semi-transparent overlay first
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
             SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Semi-transparent black
             SDL_Rect fullscreen = {0, 0, 0, 0}; // Initialize
             if(renderer) SDL_GetRendererOutputSize(renderer, &fullscreen.w, &fullscreen.h); else { fullscreen.w = 800; fullscreen.h = 600; } // Get actual window size
             SDL_RenderFillRect(renderer, &fullscreen);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // IMPORTANT: Disable blending

             // Render Buff Selection UI OR Pause Menu UI on top of overlay
             if (isInBuffSelection && ui) {
                  // Render Buff Selection UI if active
                //   std::cout << "DEBUG: Render - Rendering Buff Selection UI" << std::endl;
                  ui->renderBuffSelectionUI(currentBuffOptions, windowWidth, windowHeight);
             } else {
                  // Render Pause Menu if not in buff selection
                //   std::cout << "DEBUG: Render - Rendering Pause Menu UI" << std::endl;
                  // Draw Pause Box Background
                  if (pauseBoxTex) SDL_RenderCopy(renderer, pauseBoxTex, NULL, &pauseBoxRect);

                  // Draw Buttons (Box + Text)
                  if (buttonBoxTex) {
                      SDL_RenderCopy(renderer, buttonBoxTex, NULL, &continueButtonRect);
                      SDL_RenderCopy(renderer, buttonBoxTex, NULL, &saveButtonRect);
                      SDL_RenderCopy(renderer, buttonBoxTex, NULL, &returnButtonRect);
                  }
                  if (continueTextTex) SDL_RenderCopy(renderer, continueTextTex, NULL, &continueTextRect);
                  if (saveTextTex) SDL_RenderCopy(renderer, saveTextTex, NULL, &saveTextRect);
                  if (returnTextTex) SDL_RenderCopy(renderer, returnTextTex, NULL, &returnTextRect);

                  // Draw Volume Controls (Icons Only)
                  SDL_Texture* bgmIcon = (Game::getMusicVolume() == 0) ? soundOffTex : soundOnTex;
                  SDL_Texture* sfxIcon = (Game::getSfxVolume() == 0) ? soundOffTex : soundOnTex;
                  if (bgmIcon) SDL_RenderCopy(renderer, bgmIcon, NULL, &bgmIconRectPause);
                  if (sfxIcon) SDL_RenderCopy(renderer, sfxIcon, NULL, &sfxIconRectPause);
                  if (sliderTrackTex) {
                    SDL_RenderCopy(renderer, sliderTrackTex, NULL, &bgmSliderTrackRectPause);
                    SDL_RenderCopy(renderer, sliderTrackTex, NULL, &sfxSliderTrackRectPause);
                }
                if (sliderButtonTex) {
                    SDL_Color origColor; // Store original tint
                    SDL_GetTextureColorMod(sliderButtonTex, &origColor.r, &origColor.g, &origColor.b);
                    if(isDraggingBgmPause || isDraggingSfxPause) SDL_SetTextureColorMod(sliderButtonTex, 200, 200, 200); // Tint if dragging
                    SDL_RenderCopy(renderer, sliderButtonTex, NULL, &bgmSliderButtonRectPause);
                    SDL_RenderCopy(renderer, sliderButtonTex, NULL, &sfxSliderButtonRectPause);
                    SDL_SetTextureColorMod(sliderButtonTex, origColor.r, origColor.g, origColor.b); // Reset tint
                }
             }
         }
         // --- Render Game Over Screen UI ---
         else if (currentState == GameState::GameOver) {
             // Draw semi-transparent overlay
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
             SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Darker overlay for game over
             SDL_Rect fullscreen = {0, 0, 0, 0};
             if(renderer) SDL_GetRendererOutputSize(renderer, &fullscreen.w, &fullscreen.h); else { fullscreen.w = 800; fullscreen.h = 600; }
             SDL_RenderFillRect(renderer, &fullscreen);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

             // Render Game Over Image
             if (gameOverTex) {
                 int texW_orig, texH_orig;
                 SDL_QueryTexture(gameOverTex, NULL, NULL, &texW_orig, &texH_orig);
                 float gameOverScale = 0.8f;
                 int windowW_render, windowH_render;
                 SDL_GetRendererOutputSize(renderer, &windowW_render, &windowH_render);
                 gameOverRect.w = static_cast<int>(texW_orig * gameOverScale);
                 gameOverRect.h = static_cast<int>(texH_orig * gameOverScale);
                 gameOverRect.w = std::min(gameOverRect.w, windowW_render * 4/5 );
                 gameOverRect.h = std::min(gameOverRect.h, windowH_render * 4/5 );
                 gameOverRect.x = (windowW_render - gameOverRect.w) / 2;
                 gameOverRect.y = (windowH_render - gameOverRect.h) / 3;
                 SDL_RenderCopy(renderer, gameOverTex, NULL, &gameOverRect);
             }
             // Render Game Over Text
             if (gameOverTextTex) {
                 int textW_orig, textH_orig;
                 SDL_QueryTexture(gameOverTextTex, NULL, NULL, &textW_orig, &textH_orig);
                 float goTextScale = 0.9f;
                 gameOverTextRect.w = static_cast<int>(textW_orig * goTextScale);
                 gameOverTextRect.h = static_cast<int>(textH_orig * goTextScale);
                 int currentWindowWidth_text;
                 SDL_GetRendererOutputSize(renderer, &currentWindowWidth_text, NULL); // Get current width
                 gameOverTextRect.x = (currentWindowWidth_text - gameOverTextRect.w) / 2; // Center based on current width
                 gameOverTextRect.y = gameOverRect.y + gameOverRect.h + 30; // Below image
                 SDL_RenderCopy(renderer, gameOverTextTex, NULL, &gameOverTextRect);
             }
        } // End if/else if for Paused/GameOver states

    } // End if (Playing or Paused or GameOver) - Renders nothing if in a different state

    SDL_RenderPresent(renderer);
} // End render

// In Game/src/game.cpp

// --- generateBuffOptions (Revised replacement logic) ---
void Game::generateBuffOptions() {
    currentBuffOptions.clear();
    if (!playerEntity || !playerManager) { /* Error check */ return; }
    int currentLevel = playerManager->getLevel();
    // Get Current Component Levels
    WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
    SpellComponent* fireSpellComp = nullptr;
    SpellComponent* starSpellComp = nullptr;
    for (const auto& compPtr : playerEntity->getAllComponents()) { if (SpellComponent* sc = dynamic_cast<SpellComponent*>(compPtr.get())) { if (sc->getTag() == "spell") fireSpellComp = sc; else if (sc->getTag() == "star") starSpellComp = sc; } }
    int currentWeaponLevel = weaponComp ? weaponComp->getLevel() : -1;
    int currentFireLevel = fireSpellComp ? fireSpellComp->getLevel() : -1;
    int currentStarLevel = starSpellComp ? starSpellComp->getLevel() : -1;
    bool isFireLv0 = (currentFireLevel == 0);
    bool isStarLv0 = (currentStarLevel == 0);

    // Define Buff Amounts (as before)
    const int SPELL_DAMAGE_INCREASE = 3, SPELL_COOLDOWN_REDUCTION = 500, SPELL_PIERCE_INCREASE = 1, SPELL_PROJ_PLUS_1 = 1, SPELL_DURATION_INCREASE = 500;
    const int WEAPON_DAMAGE_INCREASE = 5, WEAPON_FIRERATE_REDUCTION = 50, WEAPON_PROJ_PLUS_1 = 1, WEAPON_PIERCE_INCREASE = 1, WEAPON_BURST_COUNT_INCREASE = 1;
    const float PLAYER_HEAL_PERCENT = 10.0f; const int PLAYER_MAX_HEALTH_INCREASE = 10; const float PLAYER_LIFESTEAL_INCREASE = 1.0f;
    const float WEAPON_PROJ_DMG_MOD = 1.0f;

    // Build the FULL Pool of Potentially Available Buffs (as before)
    std::vector<BuffInfo> allPossibleBuffs;

    // Player Buffs
    allPossibleBuffs.push_back({"Heal", "Restore 100 HP", BuffType::PLAYER_HEAL_FLAT, 100.0f});
    allPossibleBuffs.push_back({"Heal", "Restore 30% Max HP", BuffType::PLAYER_HEAL_PERC_MAX, 30.0f});
    allPossibleBuffs.push_back({"Heal", "Restore 60% Lost HP", BuffType::PLAYER_HEAL_PERC_LOST, 60.0f});
    allPossibleBuffs.push_back({"Max HP+", "+50 Max Health", BuffType::PLAYER_MAX_HEALTH_FLAT, 50.0f});
    allPossibleBuffs.push_back({"Max HP+", "+25% Max Health", BuffType::PLAYER_MAX_HEALTH_PERC_MAX, 25.0f});
    allPossibleBuffs.push_back({"Max HP+", "+50% Current HP to Max", BuffType::PLAYER_MAX_HEALTH_PERC_CUR, 50.0f});
    allPossibleBuffs.push_back({"Lifesteal+", "+1% Lifesteal", BuffType::PLAYER_LIFESTEAL, 1.0f});

    // Weapon Buffs
    if (weaponComp) {
        allPossibleBuffs.push_back({"Main Weapon Dmg+", "+10% Wpn Damage", BuffType::WEAPON_DAMAGE_FLAT, 10.0f}); // Represents 10%
        // --- Modify Random Weapon Damage Description ---
        int currentDmg = weaponComp->getDamage();
        if (currentDmg > 0) { // Only calculate range if damage is positive
            int minIncrease = std::max(1, static_cast<int>(currentDmg * 0.01f)); // 1% min, at least 1
            int maxIncrease = std::max(1, static_cast<int>(currentDmg * 0.20f)); // 20% min, at least 1
            std::stringstream randDesc;
            randDesc << "+" << minIncrease << "-" << maxIncrease << " Wpn Damage"; // e.g., "+5-100 Wpn Damage"
            allPossibleBuffs.push_back({"Main Weapon Dmg+", randDesc.str(), BuffType::WEAPON_DAMAGE_RAND_PERC, 0.0f});
        } else {
            // Fallback description if current damage is 0 or less
             allPossibleBuffs.push_back({"Main Weapon Dmg+", "+(1-20)% Wpn Damage", BuffType::WEAPON_DAMAGE_RAND_PERC, 0.0f});
        } // Amount calculated on apply
        allPossibleBuffs.push_back({"Main Weapon FireRate+", "-10% Fire Delay", BuffType::WEAPON_FIRE_RATE, 10.0f}); // Represents 10%
        allPossibleBuffs.push_back({"Main Weapon Pierce+", "+1 Wpn Pierce", BuffType::WEAPON_PIERCE, 1.0f});
        // Projectile Count Buff - Apply restriction as requested
        if (currentWeaponLevel >= 0 && currentWeaponLevel % 5 == 4) { // Appears every 5 weapon levels (starting level 4 -> 5)
            allPossibleBuffs.push_back({"Main Weapon Spread+", "+1 Spread (DMG -30%)", BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30, 0.0f});
        }
        // Burst count - maybe add restriction too? For now, always possible if weapon exists.
        allPossibleBuffs.push_back({"Main Weapon Burst+", "+1 Main Weapon Burst", BuffType::WEAPON_BURST_COUNT, 1.0f});
    }

    // Fire Spell Buffs (No Pierce, No Proj restriction)
    if (fireSpellComp) {
        int fireDmgIncrease = std::max(1, 1 * currentLevel);
        std::stringstream fireDesc;
        fireDesc << "+" << fireDmgIncrease << " Fire Damage";
        allPossibleBuffs.push_back({"Fire Dmg+", fireDesc.str(), BuffType::FIRE_SPELL_DAMAGE, 0.0f}); // Scaled on apply
        allPossibleBuffs.push_back({"Fire Vortex CDR", "-10% Fire Vortex Cooldown", BuffType::FIRE_SPELL_COOLDOWN, 10.0f}); // Represents 10%
        allPossibleBuffs.push_back({"Fire Vortex Burst+", "+1 Fire Burst", BuffType::FIRE_SPELL_PROJ_PLUS_1, 1.0f}); // No level restriction
    }

    // Star Spell Buffs (No Pierce, No Proj restriction)
    if (starSpellComp) {
        int starDmgIncrease = std::max(1, 3 * currentLevel); // Calculate damage increase
        std::stringstream starDesc;
        starDesc << "+" << starDmgIncrease << " Star Damage"; // Create dynamic description
        
        allPossibleBuffs.push_back({"Starfall Dmg+", starDesc.str(), BuffType::STAR_SPELL_DAMAGE, 0.0f}); // Scaled on apply
        allPossibleBuffs.push_back({"Starfall Cooldown", "-10% Star Cooldown", BuffType::STAR_SPELL_COOLDOWN, 10.0f}); // Represents 10%
        allPossibleBuffs.push_back({"Starfall Shot", "+1 Star", BuffType::STAR_SPELL_PROJ_PLUS_1, 1.0f}); // No level restriction
    }


    // Select 4 Random Buffs from the pool (as before)
    int numPossible = allPossibleBuffs.size();
    if (numPossible == 0) { /* Error check */ return; }
    std::vector<int> chosenIndices;
    int buffsToOffer = std::min(4, numPossible);
    while(currentBuffOptions.size() < buffsToOffer && chosenIndices.size() < (size_t)numPossible) { /* Random selection logic */
        int randIndex = std::rand() % numPossible;
        bool alreadyChosen = false;
        for(int chosen : chosenIndices) if (chosen == randIndex) { alreadyChosen = true; break; }
        if (!alreadyChosen) { chosenIndices.push_back(randIndex); currentBuffOptions.push_back(allPossibleBuffs[randIndex]); }
        if (chosenIndices.size() >= (size_t)numPossible && currentBuffOptions.size() < buffsToOffer) break;
     }


    // --- POST-SELECTION REPLACEMENT for Level 0 Spells (Corrected Type Setting) ---
    // std::cout << "  >> Checking selected buffs (" << currentBuffOptions.size() << ") for Level 0 spell replacements..." << std::endl;
    for (BuffInfo& offeredBuff : currentBuffOptions) {
        // Check Fire Spell
        for (BuffInfo& offeredBuff : currentBuffOptions) {
            bool isAnyFireSpellBuff = ( offeredBuff.type >= BuffType::FIRE_SPELL_DAMAGE && offeredBuff.type <= BuffType::FIRE_SPELL_PROJ_PLUS_1 );
            if (isFireLv0 && isAnyFireSpellBuff) {
                std::cout << "     Replacing offered Fire buff [" << offeredBuff.name << "] with Get Fire Lvl 1 (Proj+5)." << std::endl;
                offeredBuff.name = "Fire Vortex";
                offeredBuff.description = "Grants Fire Vortex"; // Update description
                offeredBuff.type = BuffType::FIRE_SPELL_PROJ_PLUS_1; // Use the Proj+1 type
                offeredBuff.amount = 5.0f; // Special amount for first get
                continue;
            }

        // Check Star Spell
        bool isAnyStarSpellBuff = ( offeredBuff.type >= BuffType::STAR_SPELL_DAMAGE && offeredBuff.type <= BuffType::STAR_SPELL_PROJ_PLUS_1 );
    if (isStarLv0 && isAnyStarSpellBuff) {
        // std::cout << "     Replacing offered Star buff [" << offeredBuff.name << "] with Get Star Lvl 1 (Proj+5)." << std::endl;
        offeredBuff.name = "Starfall";
        offeredBuff.description = "Grants Starfall"; // Update description
        offeredBuff.type = BuffType::STAR_SPELL_PROJ_PLUS_1; // Use the Proj+1 type
        offeredBuff.amount = 5.0f; // Special amount for first get
    }
}
    // --- End Post-Selection Replacement ---

    std::cout << "Generated " << currentBuffOptions.size() << " final buff options." << std::endl;
}
}


// --- applySelectedBuff (Added Debug Logging for Level Increment) ---
void Game::applySelectedBuff(int index) {
    if (!isInBuffSelection || !playerEntity || !playerManager || index < 0 || index >= (int)currentBuffOptions.size()) { if (isInBuffSelection) exitBuffSelection(); return; }
    const BuffInfo& selectedBuff = currentBuffOptions[index];
    int intAmount = static_cast<int>(selectedBuff.amount); float floatAmount = selectedBuff.amount;
    // std::cout << "Applying buff: " << selectedBuff.name << " (Type: " << static_cast<int>(selectedBuff.type) << ")" << std::endl;

    // Get components
    WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
    HealthComponent* healthComp = playerEntity->hasComponent<HealthComponent>() ? &playerEntity->getComponent<HealthComponent>() : nullptr;
    SpellComponent* fireSpellComp = nullptr;
    SpellComponent* starSpellComp = nullptr;
    for (const auto& compPtr : playerEntity->getAllComponents()) { if (SpellComponent* sc = dynamic_cast<SpellComponent*>(compPtr.get())) { if (sc->getTag() == "spell") fireSpellComp = sc; else if (sc->getTag() == "star") starSpellComp = sc; } }

    bool buffApplied = false;

    // Apply the buff based on its type
    switch (selectedBuff.type) {
        // --- Player Buffs ---
        case BuffType::PLAYER_HEAL_FLAT:
            if (healthComp) { healthComp->heal(intAmount); buffApplied = true; } break;
        case BuffType::PLAYER_HEAL_PERC_MAX:
            if (healthComp) { int heal = static_cast<int>(healthComp->getMaxHealth() * (floatAmount / 100.0f)); healthComp->heal(heal); buffApplied = true; } break;
        case BuffType::PLAYER_HEAL_PERC_LOST:
            if (healthComp) { int lostHP = healthComp->getMaxHealth() - healthComp->getHealth(); int heal = static_cast<int>(lostHP * (floatAmount / 100.0f)); healthComp->heal(heal); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_FLAT:
            if (healthComp) { healthComp->setMaxHealth(healthComp->getMaxHealth() + intAmount); healthComp->heal(intAmount); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_PERC_MAX:
             if (healthComp) { int increase = static_cast<int>(healthComp->getMaxHealth() * (floatAmount / 100.0f)); healthComp->setMaxHealth(healthComp->getMaxHealth() + increase); healthComp->heal(increase); buffApplied = true; } break;
        case BuffType::PLAYER_MAX_HEALTH_PERC_CUR:
             if (healthComp) { int increase = static_cast<int>(healthComp->getHealth() * (floatAmount / 100.0f)); healthComp->setMaxHealth(healthComp->getMaxHealth() + increase); healthComp->heal(increase); buffApplied = true; } break;
        case BuffType::PLAYER_LIFESTEAL:
            playerManager->setLifestealPercentage(playerManager->getLifestealPercentage() + floatAmount); buffApplied = true; break;
    
        // --- Weapon Buffs ---
        case BuffType::WEAPON_DAMAGE_FLAT:
            if (weaponComp) { int currentDmg = weaponComp->getDamage(); int increase = static_cast<int>(currentDmg * 0.10f); weaponComp->increaseDamage(std::max(1, increase)); weaponComp->incrementLevel(); buffApplied = true; } break; // Ensure at least +1 dmg
        case BuffType::WEAPON_DAMAGE_RAND_PERC:
            if (weaponComp) { int currentDmg = weaponComp->getDamage(); int percent = (rand() % 20) + 1; int increase = static_cast<int>(currentDmg * (percent / 100.0f)); weaponComp->increaseDamage(std::max(1, increase)); weaponComp->incrementLevel(); buffApplied = true; } break; // Ensure at least +1 dmg
        case BuffType::WEAPON_FIRE_RATE:
            if (weaponComp) { weaponComp->decreaseFireRatePercentage(floatAmount); weaponComp->incrementLevel(); buffApplied = true; } break; // Needs new method in WeaponComponent
        case BuffType::WEAPON_PIERCE:
            if (weaponComp) { weaponComp->increasePierce(intAmount); weaponComp->incrementLevel(); buffApplied = true; } break; // Keep +1 pierce
        // case BuffType::WEAPON_PROJ_COUNT: // Simple +1 proj removed/merged? Keep if separate buff needed
        //     if(weaponComp) { weaponComp->increaseProjectileCount(intAmount); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_BURST_COUNT:
             if(weaponComp) { weaponComp->increaseBurstCount(intAmount); weaponComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30: // Apply the damage reduction penalty
            if (weaponComp) { int damageReduction = static_cast<int>(weaponComp->getDamage() * 0.30f); weaponComp->increaseDamage(-damageReduction); weaponComp->increaseProjectileCount(1); weaponComp->incrementLevel(); buffApplied = true; } break;
    
    
        // --- Fire Spell Buffs ---
        case BuffType::FIRE_SPELL_DAMAGE:
             if (fireSpellComp && playerManager) { int increase = 1 * playerManager->getLevel(); fireSpellComp->increaseDamage(std::max(1, increase)); fireSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::FIRE_SPELL_COOLDOWN:
             if (fireSpellComp) { fireSpellComp->decreaseCooldownPercentage(floatAmount); fireSpellComp->incrementLevel(); buffApplied = true; } break; // Needs new method in SpellComponent
        case BuffType::FIRE_SPELL_PROJ_PLUS_1:
            if (fireSpellComp) {
                int amountToAdd = (fireSpellComp->getLevel() == 0) ? static_cast<int>(selectedBuff.amount) : 1; // Use amount (5) if level 0, else 1
                // std::cout << "  >> Applying FIRE_SPELL_PROJ_PLUS_1. Level before: " << fireSpellComp->getLevel() << ", Adding: " << amountToAdd << std::endl;
                fireSpellComp->increaseProjectileCount(amountToAdd);
                fireSpellComp->incrementLevel();
                buffApplied = true;
                // std::cout << "  >> Fire Spell Leveled Up to: " << fireSpellComp->getLevel() << std::endl;
             } else { 
                // std::cout << "  >> ERROR: fireSpellComp is null!" << std::endl;
            }
             break;
    
    
        // --- Star Spell Buffs ---
        case BuffType::STAR_SPELL_DAMAGE:
             if (starSpellComp && playerManager) { int increase = 3 * playerManager->getLevel(); starSpellComp->increaseDamage(std::max(1, increase)); starSpellComp->incrementLevel(); buffApplied = true; } break;
        case BuffType::STAR_SPELL_COOLDOWN:
             if (starSpellComp) { starSpellComp->decreaseCooldownPercentage(floatAmount); starSpellComp->incrementLevel(); buffApplied = true; } break; // Needs new method in SpellComponent
        case BuffType::STAR_SPELL_PROJ_PLUS_1:
            if (starSpellComp) {
                int amountToAdd = (starSpellComp->getLevel() == 0) ? static_cast<int>(selectedBuff.amount) : 1; // Use amount (5) if level 0, else 1
                std::cout << "  >> Applying STAR_SPELL_PROJ_PLUS_1. Level before: " << starSpellComp->getLevel() << ", Adding: " << amountToAdd << std::endl;
                starSpellComp->increaseProjectileCount(amountToAdd);
                starSpellComp->incrementLevel();
                buffApplied = true;
                std::cout << "  >> Star Spell Leveled Up to: " << starSpellComp->getLevel() << std::endl;
             } else { std::cout << "  >> ERROR: starSpellComp is null!" << std::endl;}
             break;
    
        default:
             std::cerr << "Warning: Invalid BuffType (" << static_cast<int>(selectedBuff.type) << ") reached switch default." << std::endl;
             break;
    }

    // Check buffApplied flag *after* the switch
    if (!buffApplied) {
        std::cerr << "Warning: Selected buff '" << selectedBuff.name << "' (Type: " << static_cast<int>(selectedBuff.type) << ") could not be applied (Component missing or Invalid Type)." << std::endl;
    }
    exitBuffSelection();
}
void Game::exitBuffSelection() {
    if (isInBuffSelection) { // Only act if we are actually in buff selection
    //    std::cout << "Exiting Buff Selection..." << std::endl;
       isInBuffSelection = false;
       currentState = GameState::Playing; // <<< SET GameState back to Playing
       // REMOVED: isPaused = false;
       currentBuffOptions.clear();
       if (Mix_PausedMusic()) Mix_ResumeMusic(); // Resume music after selection
    }
}

void Game::enterBuffSelection() {
    // Should only enter if currently playing
    if (currentState == GameState::Playing) {
        // std::cout << "Entering Buff Selection..." << std::endl;
        isInBuffSelection = true;
        currentState = GameState::Paused; // Set GameState to Paused
        if (Mix_PlayingMusic()) Mix_PauseMusic(); // Pause music for buff selection
        generateBuffOptions(); // Generate the options to be displayed
        // Optional: Call calculatePauseLayout if buff UI depends on it? Unlikely.
    } else {
         std::cout << "Warning: Tried to enter buff selection when not in Playing state." << std::endl;
    }
}

void Game::togglePause() {
    // Check current state to toggle correctly
    if (currentState == GameState::Playing) {
        currentState = GameState::Paused; // Enter Paused state
        std::cout << "togglePause: Paused" << std::endl;
        if (Mix_PlayingMusic()) Mix_PauseMusic(); // Pause SDL_mixer music
        // Update pause menu layout and state when entering pause
        isDraggingBgmPause = false; // Ensure dragging stops
        isDraggingSfxPause = false;
        // Update stored volumes based on current game volume when pausing
        storedMusicVolumePause = Game::getMusicVolume();
        storedSfxVolumePause = Game::getSfxVolume();
        calculatePauseLayout(); // Calculate positions for drawing pause UI
    } else if (currentState == GameState::Paused && !isInBuffSelection) {
        // IMPORTANT: Only unpause if not in buff selection screen
        currentState = GameState::Playing; // Return to Playing state
        // std::cout << "togglePause: Resumed" << std::endl;
        if (Mix_PausedMusic()) Mix_ResumeMusic(); // Resume SDL_mixer music
        // No need to exit buff selection here, as we only unpause if NOT in it
    } else if (currentState == GameState::Paused && isInBuffSelection) {
         std::cout << "togglePause: Cannot unpause while buff selection is active." << std::endl;
         // Or maybe Escape should cancel buff selection AND unpause? Requires different logic.
    }
    // If currentState is GameOver, pressing pause key does nothing here
}

void Game::changeMusicVolume(int delta) {
    musicVolume += delta;
    // Clamp volume between 0 and MIX_MAX_VOLUME (128)
    if (musicVolume < 0) musicVolume = 0;
    if (musicVolume > MIX_MAX_VOLUME) musicVolume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(musicVolume);
    std::cout << "Music Volume changed to: " << musicVolume << std::endl; // Debug
}

void Game::changeSfxVolume(int delta) {
    sfxVolume += delta;
    // Clamp volume
    if (sfxVolume < 0) sfxVolume = 0;
    if (sfxVolume > MIX_MAX_VOLUME) sfxVolume = MIX_MAX_VOLUME;
    // Set volume for all channels (-1)
    // Note: This might affect sounds already playing differently depending on SDL_mixer version.
    // A more robust way might involve setting volume per chunk before playing or per channel after playing.
    Mix_Volume(-1, sfxVolume);
    std::cout << "SFX Volume changed to: " << sfxVolume << std::endl; // Debug

    // Optional: Adjust volume of existing sound chunks if AssetManager holds them
    // This is more complex and depends on how sounds are managed.
    // For now, Mix_Volume(-1, ...) affects future sounds reliably.
}


// --- ADD Helper: Calculate Pause Layout ---
void Game::calculatePauseLayout() {
    if (!isRunning || !renderer) return; // Need renderer for window size

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // --- Scaling Factor (Based on Game's main scaling logic) ---
    const int referenceWidth = 1920;
    const int referenceHeight = 1080;
    float scaleX = static_cast<float>(windowWidth) / static_cast<float>(referenceWidth);
    float scaleY = static_cast<float>(windowHeight) / static_cast<float>(referenceHeight);
    float scaleFactor = std::min(scaleX, scaleY);
    scaleFactor = std::max(scaleFactor, 0.6f); // Example minimum scale for pause elements

    // --- Pause Box (Slightly Bigger & Scaled) ---
    int boxW_orig = 128;
    int boxH_orig = 245;
    pauseBoxRect.w = static_cast<int>(boxW_orig * 1.2f * scaleFactor);
    pauseBoxRect.h = static_cast<int>(boxH_orig * 1.2f * scaleFactor);
    pauseBoxRect.w = std::max(120, pauseBoxRect.w); // Adjusted min width
    pauseBoxRect.h = std::max(240, pauseBoxRect.h); // Adjusted min height
    pauseBoxRect.x = (windowWidth - pauseBoxRect.w) / 2;
    pauseBoxRect.y = (windowHeight - pauseBoxRect.h) / 2;

    // --- Calculate Element Sizes ---
    // --- INCREASED BASE PADDING ---
    int internalPaddingBase = 15; // Increased from 10 for more spacing
    // --- END INCREASE ---
    int internalPadding = std::max(5, static_cast<int>(internalPaddingBase * scaleFactor));
    int availableWidth = pauseBoxRect.w - 2 * internalPadding;

    // Button Box Dimensions (Scaled to fit availableWidth)
    int btnBoxW_orig = 230;
    int btnBoxH_orig = 60;
    float btnScale = (btnBoxW_orig > 0) ? static_cast<float>(availableWidth) / btnBoxW_orig : 1.0f;
    int btnBoxW = availableWidth;
    int btnBoxH = static_cast<int>(btnBoxH_orig * btnScale);
    btnBoxH = std::max(15, btnBoxH); // Ensure minimum button height

    // Volume Control Sizes
    // Estimate needed height based on button height now
    int volAreaHeightEst = btnBoxH * 2.0f; // Reduced height estimate relative to buttons
    int iconSize = static_cast<int>(volAreaHeightEst * 0.35f); // Icon size relative to area height
    iconSize = std::max(15, iconSize); // Minimum icon size
    int sliderHeight = std::max(3, iconSize / 3); // Slider track height
    int sliderWidth = std::max(20, availableWidth - iconSize - internalPadding); // Slider track width
    int sliderButtonW = std::max(5, iconSize / 2); // Slider button width
    int sliderButtonH = std::max(8, static_cast<int>(sliderHeight * 1.5f)); // Slider button height

    // --- Calculate Total Required Height & Dynamic Padding ---
    // Calculate height needed just for the main elements themselves
    int volRowHeight = iconSize + internalPadding / 2; // Height for one volume row + minimal gap
    int totalElementHeightOnly = (btnBoxH * 3) + (volRowHeight * 2);

    // Calculate available space for padding inside the box
    int availableHeightForPadding = pauseBoxRect.h - totalElementHeightOnly;
    int numGaps = 6; // Gaps: top, btn1-vol, vol-vol, vol-btn2, btn2-btn3, bottom
    int dynamicPaddingY = 1; // Default padding if space is tight

    if (availableHeightForPadding > numGaps * 2) { // Only distribute if significant space available
        dynamicPaddingY = availableHeightForPadding / numGaps; // Distribute remaining space evenly
    } else {
        std::cout << "Warning: Pause elements might be cramped. Minimal Padding." << std::endl;
    }
    // Ensure some minimum padding
    dynamicPaddingY = std::max(3, dynamicPaddingY);


    // --- Position Elements Vertically using Dynamic Padding ---
    int currentY = pauseBoxRect.y + dynamicPaddingY; // Start with top padding

    // Continue Button
    continueButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH };
    currentY += btnBoxH + dynamicPaddingY; // Add padding below

    // Volume Controls Area Start
    int volAreaStartY = currentY;

    // BGM Row (Position elements within this row)
    bgmIconRectPause = {pauseBoxRect.x + internalPadding, volAreaStartY + (volRowHeight - iconSize) / 2, iconSize, iconSize};
    bgmSliderTrackRectPause = {bgmIconRectPause.x + iconSize + internalPadding, volAreaStartY + (volRowHeight - sliderHeight) / 2, sliderWidth, sliderHeight};
    int bgmSliderTrackW = std::max(1, bgmSliderTrackRectPause.w - sliderButtonW);
    float bgmPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getMusicVolume()) / MIX_MAX_VOLUME;
    bgmSliderButtonRectPause = { bgmSliderTrackRectPause.x + static_cast<int>(bgmPercent * bgmSliderTrackW),
                                 bgmSliderTrackRectPause.y + (sliderHeight - sliderButtonH) / 2,
                                 sliderButtonW, sliderButtonH };

    // SFX Row (Position elements within this row, offset from BGM row)
    int sfxRowStartY = volAreaStartY + volRowHeight; // Start below BGM row
    sfxIconRectPause = {pauseBoxRect.x + internalPadding, sfxRowStartY + (volRowHeight - iconSize) / 2, iconSize, iconSize};
    sfxSliderTrackRectPause = {sfxIconRectPause.x + iconSize + internalPadding, sfxRowStartY + (volRowHeight - sliderHeight) / 2, sliderWidth, sliderHeight};
    int sfxSliderTrackW = std::max(1, sfxSliderTrackRectPause.w - sliderButtonW);
    float sfxPercent = (MIX_MAX_VOLUME == 0) ? 0.0f : static_cast<float>(Game::getSfxVolume()) / MIX_MAX_VOLUME;
    sfxSliderButtonRectPause = { sfxSliderTrackRectPause.x + static_cast<int>(sfxPercent * sfxSliderTrackW),
                                 sfxSliderTrackRectPause.y + (sliderHeight - sliderButtonH) / 2,
                                 sliderButtonW, sliderButtonH };

    // Advance Y position past both volume rows + padding
    currentY = sfxRowStartY + volRowHeight + dynamicPaddingY;

    // Save Button
    saveButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH };
    currentY += btnBoxH + dynamicPaddingY; // Add padding below

    // Return Button
    returnButtonRect = { pauseBoxRect.x + internalPadding, currentY, btnBoxW, btnBoxH };
    // currentY += btnBoxH + dynamicPaddingY; // Add padding below (optional if needed)


    // --- Calculate Text Positions (Centered in Buttons) ---
    // Ensure text textures exist before querying
    if (continueTextTex) {
        SDL_QueryTexture(continueTextTex, NULL, NULL, &continueTextRect.w, &continueTextRect.h);
        // Scale text size? Maybe use a fixed smaller font for pause menu buttons?
        // Let's assume text fits for now, just center it.
        continueTextRect.x = continueButtonRect.x + (continueButtonRect.w - continueTextRect.w) / 2;
        continueTextRect.y = continueButtonRect.y + (continueButtonRect.h - continueTextRect.h) / 2;
    }
    if (saveTextTex) {
        SDL_QueryTexture(saveTextTex, NULL, NULL, &saveTextRect.w, &saveTextRect.h);
        saveTextRect.x = saveButtonRect.x + (saveButtonRect.w - saveTextRect.w) / 2;
        saveTextRect.y = saveButtonRect.y + (saveButtonRect.h - saveTextRect.h) / 2;
    }
    if (returnTextTex) {
        SDL_QueryTexture(returnTextTex, NULL, NULL, &returnTextRect.w, &returnTextRect.h);
        returnTextRect.x = returnButtonRect.x + (returnButtonRect.w - returnTextRect.w) / 2;
        returnTextRect.y = returnButtonRect.y + (returnButtonRect.h - returnTextRect.h) / 2;
    }
}




SDL_Texture* Game::renderPauseText(const std::string& text, SDL_Color color) { // Ensure "Game::" is present
    if (!pauseFont) { // Check if pauseFont is loaded
        std::cerr << "Error: renderPauseText called but pauseFont is null!" << std::endl;
        return nullptr;
    }
    if (!renderer) { // Check if renderer is valid
         std::cerr << "Error: renderPauseText called but renderer is null!" << std::endl;
         return nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(pauseFont, text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to render pause text surface ('" << text << "'): " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create pause text texture ('" << text << "'): " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(surface); // Free surface regardless of texture creation success
    return texture;
}

// --- Update Volume Setters/Getters (Make Static) ---
void Game::setMusicVolume(int volume) { // Now static
    musicVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));
    Mix_VolumeMusic(musicVolume);
    std::cout << "Static Music Volume set to: " << musicVolume << std::endl;
}

void Game::setSfxVolume(int volume) { // Now static
    sfxVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));
    Mix_Volume(-1, sfxVolume);
    std::cout << "Static SFX Volume set to: " << sfxVolume << std::endl;
}

int Game::getMusicVolume() { // Now static
    return musicVolume;
}

int Game::getSfxVolume() { // Now static
    return sfxVolume;
}

// Helper function to get current buff button rectangles (points to the colored boxes)
// Replace the entire Game::getBuffButtonRects function in game.cpp with this:
// Helper function to get current buff button rectangles (points to the colored boxes)
std::vector<SDL_Rect> Game::getBuffButtonRects() {
    std::vector<SDL_Rect> rects;
    if (!renderer) return rects; // Need renderer for size

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Replicate layout calculation logic from UIManager::renderBuffSelectionUI
    const int numButtons = std::min((int)currentBuffOptions.size(), 4);
    if (numButtons == 0) return rects;

    // Define base sizes and padding (MUST MATCH renderBuffSelectionUI)
    const int refWidth = 800;
    const int refHeight = 600;
    const int baseIconSize = 48; // MUST MATCH the value in renderBuffSelectionUI
    const int baseBoxW = 180;
    const int baseBoxH = 100;
    const int baseGap = 25;
    const int baseIconOffsetY = -100; // Y position for icons (relative to center)
    // Note: baseBoxOffsetY calculation depends on iconDrawSize below

    // Calculate scale factors (MUST MATCH renderBuffSelectionUI)
    float scaleX = static_cast<float>(windowWidth) / refWidth;
    float scaleY = static_cast<float>(windowHeight) / refHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    // --- Calculate sizes needed for positioning (MUST MATCH renderBuffSelectionUI) ---
    int iconDrawSize = std::max(32, static_cast<int>(baseIconSize * scaleFactor)); // Calculate the square size
    int boxW = std::max(100, static_cast<int>(baseBoxW * scaleFactor));
    int boxH = std::max(60, static_cast<int>(baseBoxH * scaleFactor));
    int gap = std::max(15, static_cast<int>(baseGap * scaleFactor));
    int iconOffsetY = windowHeight / 2 + static_cast<int>(baseIconOffsetY * scaleY);
    // Calculate boxOffsetY using the square iconDrawSize
    int boxOffsetY = iconOffsetY + iconDrawSize + std::max(3, static_cast<int>(5 * scaleFactor));
    // --- End size calculations ---

    // Calculate total width needed and starting position for centering
    int totalWidth = numButtons * boxW + (numButtons - 1) * gap;
    int startX = (windowWidth - totalWidth) / 2;

    rects.resize(numButtons);
    for (int i = 0; i < numButtons; ++i) {
        // Calculate the rectangle for the COLORED box area (used for clicking)
        // Its Y position depends on the iconDrawSize calculation above
        rects[i] = {startX + i * (boxW + gap), boxOffsetY, boxW, boxH};
    }

    return rects; // Returns rects for the colored boxes
}

// --- New Function Implementation: initializeEnemyDatabase ---
void Game::initializeEnemyDatabase() {
    allEnemyDatabase.clear(); // Start fresh

        // First set of enemies
    allEnemyDatabase.push_back({"zombie", zombieSprite, zombieHealth, zombieDamage, zombieSpeed, zombieExp, 0, 1});
    allEnemyDatabase.push_back({"kfc1", kfc1Sprite, kfc1Health, kfc1Damage, kfc1Speed, kfc1Exp, 0, 5, "kfc2", 15});
    allEnemyDatabase.push_back({"ina1", ina1Sprite, ina1Health, ina1Damage, ina1Speed, ina1Exp, 0, 5, "ina2", 15});
    allEnemyDatabase.push_back({"bear1", bear1Sprite, bear1Health, bear1Damage, bear1Speed, bear1Exp, 0, 5, "bear2", 15});
    allEnemyDatabase.push_back({"skeleton1", skeleton1Sprite, skeleton1Health, skeleton1Damage, skeleton1Speed, skeleton1Exp, 0, 5, "skeleton2", 15});
    allEnemyDatabase.push_back({"aligator1", aligator1Sprite, aligator1Health, aligator1Damage, aligator1Speed, aligator1Exp, 0, 5, "aligator2", 15});

    // Second set of enemies
    allEnemyDatabase.push_back({"kfc2", kfc2Sprite, kfc2Health, kfc2Damage, kfc2Speed, kfc2Exp, 0, 15});
    allEnemyDatabase.push_back({"ina2", ina2Sprite, ina2Health, ina2Damage, ina2Speed, ina2Exp, 0, 15, "ina3", 25});
    allEnemyDatabase.push_back({"ina3", ina3Sprite, ina3Health, ina3Damage, ina3Speed, ina3Exp, 0, 25});
    allEnemyDatabase.push_back({"bear2", bear2Sprite, bear2Health, bear2Damage, bear2Speed, bear2Exp, 0, 15});
    allEnemyDatabase.push_back({"skeleton2", skeleton2Sprite, skeleton2Health, skeleton2Damage, skeleton2Speed, skeleton2Exp, 0, 15, "skeleton3", 25});
    allEnemyDatabase.push_back({"skeleton3", skeleton3Sprite, skeleton3Health, skeleton3Damage, skeleton3Speed, skeleton3Exp, 0, 25, "skeleton4", 35});
    allEnemyDatabase.push_back({"skeleton4", skeleton4Sprite, skeleton4Health, skeleton4Damage, skeleton4Speed, skeleton4Exp, 0, 35, "skeleton5", 45});
    allEnemyDatabase.push_back({"skeleton5", skeleton5Sprite, skeleton5Health, skeleton5Damage, skeleton5Speed, skeleton5Exp, 0, 45});
    allEnemyDatabase.push_back({"aligator2", aligator2Sprite, aligator2Health, aligator2Damage, aligator2Speed, aligator2Exp, 0, 15});

    // Additional enemies
    allEnemyDatabase.push_back({"skeleton_shield", skeletonShieldSprite, skeletonShieldHealth, skeletonShieldDamage, skeletonShieldSpeed, skeletonShieldExp, 0, 20});
    allEnemyDatabase.push_back({"eliteskeleton_shield", eliteSkeletonShieldSprite, eliteSkeletonShieldHealth, eliteSkeletonShieldDamage, eliteSkeletonShieldSpeed, eliteSkeletonShieldExp, 0, 40});

}

// --- New Function Implementation: updateSpawnPoolAndWeights ---
void Game::updateSpawnPoolAndWeights() {
    if (!playerManager) return; // Need player level
    int playerLevel = playerManager->getLevel();

    currentSpawnPool.clear();
    currentTotalSpawnWeight = 0; // Reset total weight

    std::map<std::string, EnemySpawnInfo*> poolMap; // Use map to handle upgrades easily

    // Iterate through the database to determine initial eligibility and base weights
    for (EnemySpawnInfo& dbEntry : allEnemyDatabase) {
        if (playerLevel >= dbEntry.minLevel) {
             // Check if this enemy is an upgrade and if its base version is already handled
             bool isUpgradedVersion = false;
             std::string baseEnemyTag = "";
             for (const auto& checkEntry : allEnemyDatabase) {
                  if (checkEntry.upgradeTag == dbEntry.tag && playerLevel >= checkEntry.upgradeLevelRequirement) {
                       isUpgradedVersion = true;
                       baseEnemyTag = checkEntry.tag;
                       break; // Found the base enemy that upgrades to this one
                  }
             }

            // If it's an upgrade, remove the base version from the pool map if present
            if (isUpgradedVersion && poolMap.count(baseEnemyTag)) {
                poolMap.erase(baseEnemyTag);
            }

            // Add the current entry (either base or the valid upgrade) to the map
            // This ensures only the highest eligible tier is considered for weighting
            poolMap[dbEntry.tag] = &dbEntry;
        }
    }

    // Now process the eligible enemies in the map to set weights
    for (auto const& [tag, enemyInfoPtr] : poolMap) {
        EnemySpawnInfo& enemyInfo = *enemyInfoPtr; // Dereference pointer

        // Rule 1: Lv 1-5, only zombie
        if (playerLevel <= 5) {
            if (enemyInfo.tag == "zombie") {
                enemyInfo.currentSpawnWeight = 20;
            } else {
                enemyInfo.currentSpawnWeight = 0; // Others have 0 chance
            }
        }
        // Rule 2 & 3: Lv 5+ specific mobs and weight increases
        else { // playerLevel > 5
            // Set base weights for newly added mobs at level 5
            if (enemyInfo.minLevel == 5 && playerLevel == 5) { // Specifically when hitting level 5
                 if (enemyInfo.tag == "kfc1" || enemyInfo.tag == "ina1" || enemyInfo.tag == "bear1" || enemyInfo.tag == "skeleton1" || enemyInfo.tag == "aligator1") {
                      enemyInfo.currentSpawnWeight = 5;
                 }
            }
            // Set base weight for upgrades when they first appear
            else if (enemyInfo.minLevel > 5 && playerLevel >= enemyInfo.minLevel && enemyInfo.currentSpawnWeight == 0) { // Assign initial weight if just unlocked
                 enemyInfo.currentSpawnWeight = 5; // Base weight for new upgrades
            }
            // Keep zombie weight consistent after level 5
            else if (enemyInfo.tag == "zombie") {
                 enemyInfo.currentSpawnWeight = 20;
            }
             // Ensure other valid mobs maintain a base weight if they were previously added
             else if (enemyInfo.currentSpawnWeight == 0 && enemyInfo.minLevel <= playerLevel) {
                  // This case handles mobs added at level 5 that persist, or other special mobs
                  // Re-assign base weight if it somehow got reset (shouldn't happen with map logic)
                  if (enemyInfo.minLevel == 5 && (enemyInfo.tag == "kfc1" || enemyInfo.tag == "ina1" || enemyInfo.tag == "bear1" || enemyInfo.tag == "skeleton1" || enemyInfo.tag == "aligator1")) {
                       enemyInfo.currentSpawnWeight = 5;
                  } else if (enemyInfo.minLevel > 5){ // Base weight for other later-game mobs
                      enemyInfo.currentSpawnWeight = 5;
                  } else {
                      enemyInfo.currentSpawnWeight = 0; // Default to 0 if no rule matches
                  }
             }


            // Apply weight increase based on 10-level milestones AFTER level 5
            int increaseCycles = (playerLevel - 5) / 10;
            if (increaseCycles > 0 && enemyInfo.currentSpawnWeight > 0) { // Only increase weight if it's supposed to be in the pool
                 int increaseAmount = increaseCycles * 10;
                 enemyInfo.currentSpawnWeight = std::min(25, enemyInfo.currentSpawnWeight + increaseAmount);
            }
        }

        // Add to the final pool if weight > 0
        if (enemyInfo.currentSpawnWeight > 0) {
            currentSpawnPool.push_back(&enemyInfo);
            currentTotalSpawnWeight += enemyInfo.currentSpawnWeight;
        }
    }
    //  std::cout << "Updated Spawn Pool. Level: " << playerLevel << ", Pool Size: " << currentSpawnPool.size() << ", Total Weight: " << currentTotalSpawnWeight << std::endl;
}

// --- New Function Implementation: selectEnemyBasedOnWeight ---
EnemySpawnInfo* Game::selectEnemyBasedOnWeight() {
    if (currentSpawnPool.empty()) {
        std::cerr << "Spawn pool is empty!" << std::endl;
        // Optionally try to repopulate? Or return null.
        updateSpawnPoolAndWeights(); // Attempt to repopulate
        if (currentSpawnPool.empty()) return nullptr;
    }

    if (currentTotalSpawnWeight <= 0) {
        // Fallback: If total weight is 0 (e.g., only zombie < lv 5, but somehow weight is 0), pick randomly
        std::cerr << "Warning: Total spawn weight is zero. Picking random enemy from pool." << std::endl;
        if (currentSpawnPool.empty()) return nullptr; // Still check if empty after warning
        return currentSpawnPool[rand() % currentSpawnPool.size()];
    }

    int randomValue = rand() % currentTotalSpawnWeight;
    int cumulativeWeight = 0;
    for (EnemySpawnInfo* enemyInfo : currentSpawnPool) {
        cumulativeWeight += enemyInfo->currentSpawnWeight;
        if (randomValue < cumulativeWeight) {
            return enemyInfo; // Return the selected enemy's data
        }
    }

    // Fallback in case of rounding errors or unexpected issues
    std::cerr << "Warning: Weighted selection did not pick an enemy. Returning last." << std::endl;
    return currentSpawnPool.back();
}