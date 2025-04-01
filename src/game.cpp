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
    // Use the renderer provided by main.cpp
    if (Game::renderer) {
        std::cout << "Game::init - Using existing Renderer" << std::endl;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // Initialize UI manager
        delete ui; // Delete previous instance if any
        ui = new UIManager(renderer);
        if (ui) ui->init();
    } else {
         std::cerr << "Error: Game::init called but Game::renderer is null!" << std::endl;
         isRunning = false;
         // Clean up potentially allocated members if init fails early
         if(saveLoadManager) { delete saveLoadManager; saveLoadManager = nullptr; }
         if(assets) { delete assets; assets = nullptr; }
         return;
    }
    currentState = GameState::Playing; // Start in playing state
    std::cout << "Game state initialized to Playing." << std::endl;
    Mix_VolumeMusic(musicVolume); Mix_Volume(-1, sfxVolume);
    manager.refresh();

    // --- Load Textures and Sounds ---
    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");
    // ... (load other assets using member 'assets') ...
     assets->AddTexture("fire", "sprites/projectile/fire.png");
     assets->AddTexture("starproj", "sprites/projectile/star.png");
     assets->AddTexture("exp_orb_1", "sprites/projectile/gunshot.png");
     assets->AddTexture("exp_orb_10", "sprites/projectile/gunshot.png");
     // ... add others ...
     assets->AddSoundEffect("gunshot_sound", "assets/sound/shot.wav");
     assets->AddMusic("level_music", "assets/sound/hlcbg.mp3"); // Should work if file exists now
     assets->AddSoundEffect("fire_spell_sound", "assets/sound/fire_cast.wav"); // Should work if file exists now
     assets->AddSoundEffect("star_spell_sound", "assets/sound/star_cast.wav"); // Should work if file exists now

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
    playerEntity->addComponent<WeaponComponent>( "placeholder", 0, 99999, 0.0f, 0, 0.0f, 0, 1, "projectile", 1, 1, 999);
    playerEntity->addComponent<SpellComponent>( "placeholder_spell", 0, 99999, 0.0f, 0, 1, "fire", 0, SpellTrajectory::SPIRAL, 0.0f, 1 );
    playerEntity->addComponent<SpellComponent>( "placeholder_star", 0, 99999, 0.0f, 0, 1, "starproj", 0, SpellTrajectory::RANDOM_DIRECTION, 0.0f, 1 );
    // --- End Adding Components ---

    playerEntity->addGroup(groupPlayers);

    // --- Create Player Manager ---
    delete playerManager;
    playerManager = new Player(playerEntity); // PlayerManager uses the entity with placeholder components

        // --- Explicitly Set Default Stats on Components ---
        // Health
        if (playerEntity->hasComponent<HealthComponent>()) {
            auto& healthComp = playerEntity->getComponent<HealthComponent>();
            healthComp.maxHealth = playerHealth; // from constants.h
            healthComp.health = playerHealth;    // Full health
        }

        // Weapon
        if (playerEntity->hasComponent<WeaponComponent>()) {
            auto& weaponComp = playerEntity->getComponent<WeaponComponent>();
            weaponComp.tag = "pistol";
            weaponComp.damage = 25;
            weaponComp.fireRate = 1000;
            weaponComp.projectileSpeed = 10.0f;
            weaponComp.projectileRange = 2500;
            weaponComp.spreadAngle = 0.1f;
            weaponComp.projectilesPerShot = 1;
            weaponComp.projectileSize = 32;
            weaponComp.projectileTexture = "projectile";
            weaponComp.projectilePierce = 1;
            weaponComp.shotsPerBurst = 3;
            weaponComp.burstDelay = 75;
        }

        // Spells (Find them, potentially by order or need a better way like tag lookup)
        // This assumes the first SpellComponent added was fire, second was star.
        // A better approach would involve giving spells unique IDs or iterating and checking tags.
        int spellCompCount = 0;
        SpellComponent* fireSpell = nullptr;
        SpellComponent* starSpell = nullptr;
        for(auto& comp : playerEntity->getAllComponents()) { // Assumes getAllComponents exists
            if (SpellComponent* spell = dynamic_cast<SpellComponent*>(comp.get())) {
                 if (!fireSpell) fireSpell = spell; // Assign first found to fire
                 else if (!starSpell) starSpell = spell; // Assign second found to star
            }
        }

        if (fireSpell) {
            fireSpell->tag = "spell";
            fireSpell->damage = 5;
            fireSpell->cooldown = 200;
            fireSpell->projectileSpeed = 3.0f;
            fireSpell->projectilesPerCast = 5;
            fireSpell->projectileSize = 38;
            fireSpell->projectileTexture = "fire";
            fireSpell->duration = 3000;
            fireSpell->trajectoryMode = SpellTrajectory::SPIRAL;
            fireSpell->spiralGrowthRate = 8.0f;
            fireSpell->projectilePierce = 2;
        } else { std::cerr << "Warning: Could not find first SpellComponent to set defaults." << std::endl; }

        if (starSpell) {
            starSpell->tag = "star";
            starSpell->damage = 5;
            starSpell->cooldown = 5000;
            starSpell->projectileSpeed = 3.0f;
            starSpell->projectilesPerCast = 0; // This might be intended (or should be 1?)
            starSpell->projectileSize = 38;
            starSpell->projectileTexture = "starproj";
            starSpell->duration = 3000;
            starSpell->trajectoryMode = SpellTrajectory::RANDOM_DIRECTION;
            starSpell->spiralGrowthRate = 8.0f; // Might not be used by random
            starSpell->projectilePierce = 2;
         } else { std::cerr << "Warning: Could not find second SpellComponent to set defaults." << std::endl; }


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
            }
            // Update mouse world coordinates only when playing
            int mouseX_Screen, mouseY_Screen;
            SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
            Game::mouseX = mouseX_Screen + Game::camera.x;
            Game::mouseY = mouseY_Screen + Game::camera.y;
            // Player movement input is handled by KeyboardController.update()
            break; // End Playing state handling

            case GameState::Paused:
            // --- ADDED: Log entry into Paused state handling ---
            // std::cout << "DEBUG: handleEvents in Paused state." << std::endl; // Can be noisy

            // Handle buff selection input if active while paused
            if (isInBuffSelection) {
                 // --- ADDED: Log entry into Buff Selection block ---
                 std::cout << "DEBUG: Paused state + isInBuffSelection = true. Checking input..." << std::endl;

                 if (Game::event.type == SDL_MOUSEBUTTONDOWN) {
                      // --- ADDED: Log Mouse Down Event ---
                      std::cout << "DEBUG: Buff Selection - MOUSEBUTTONDOWN received." << std::endl;
                     if (Game::event.button.button == SDL_BUTTON_LEFT) {
                         int mouseX_Screen, mouseY_Screen;
                         SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
                         std::cout << "DEBUG: Buff Selection - Left Click at (" << mouseX_Screen << ", " << mouseY_Screen << ")" << std::endl;

                         // Calculate button layout again (ensure this matches UI::renderBuffSelectionUI)
                         // TODO: Move this calculation to a helper or store rects if layout doesn't change
                         int totalButtonWidth = 4 * 180 + 3 * 20;
                         int startX = (WINDOW_WIDTH - totalButtonWidth) / 2;
                         int buttonY = WINDOW_HEIGHT / 2 - 50;
                         int buttonW = 180;
                         int buttonH = 100;
                         int gap = 20;

                          for (size_t i = 0; i < currentBuffOptions.size() && i < 4; ++i) {
                             SDL_Rect buttonRect = {startX + static_cast<int>(i) * (buttonW + gap), buttonY, buttonW, buttonH};
                             // --- ADDED: Log Button Rect Check ---
                             std::cout << "DEBUG: Checking Button " << i << " Rect: x=" << buttonRect.x << " y=" << buttonRect.y << " w=" << buttonRect.w << " h=" << buttonRect.h << std::endl;
                             if (ui && ui->isMouseInside(mouseX_Screen, mouseY_Screen, buttonRect)) {
                                 // --- ADDED: Log Button Hit ---
                                 std::cout << "DEBUG: Click HIT on Button " << i << "! Applying buff..." << std::endl;
                                 applySelectedBuff(static_cast<int>(i));
                                 return; // Buff applied and exited, consume event
                             }
                          }
                          // --- ADDED: Log if click missed all buttons ---
                          std::cout << "DEBUG: Left click did not hit any buff buttons." << std::endl;
                     }
                 } else if (Game::event.type == SDL_KEYDOWN) {
                      // --- ADDED: Log Key Down Event ---
                      std::cout << "DEBUG: Buff Selection - KEYDOWN received. Key: " << SDL_GetKeyName(Game::event.key.keysym.sym) << std::endl;
                     switch (Game::event.key.keysym.sym) {
                         case SDLK_1:
                              std::cout << "DEBUG: Key '1' pressed. Applying buff 0..." << std::endl;
                              applySelectedBuff(0); return;
                         case SDLK_2:
                              std::cout << "DEBUG: Key '2' pressed. Applying buff 1..." << std::endl;
                              applySelectedBuff(1); return;
                         case SDLK_3:
                              std::cout << "DEBUG: Key '3' pressed. Applying buff 2..." << std::endl;
                              applySelectedBuff(2); return;
                         case SDLK_4:
                              std::cout << "DEBUG: Key '4' pressed. Applying buff 3..." << std::endl;
                              applySelectedBuff(3); return;
                         default:
                              std::cout << "DEBUG: Key pressed is not 1-4." << std::endl;
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
            case SDLK_e:      // Resume via 'E' (Legacy, can remove if desired)
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


void Game::spawnEnemy() {
    // Check if playerEntity exists before using it
    if (!playerEntity) {
        std::cerr << "Cannot spawn enemy, playerEntity is null!" << std::endl;
        return;
    }

    if (spawnPoints.empty()) {
        std::cerr << "No spawn points available!" << std::endl;
        return;
    }

    struct EnemyType {
        std::string tag;
        const char* sprite;
        int health;
        int damage;
        float speed;
        int experience;
    };

    std::vector<EnemyType> enemyTypes = {
        {"zombie", zombieSprite, zombieHealth, zombieDamage, zombieSpeed, zombieExp},
        {"aligator1", aligator1Sprite, aligator1Health, aligator1Damage, aligator1Speed, aligator1Exp},
        {"aligator2", aligator2Sprite, aligator2Health, aligator2Damage, aligator2Speed, aligator2Exp},
        {"bear1", bear1Sprite, bear1Health, bear1Damage, bear1Speed, bear1Exp},
        {"bear2", bear2Sprite, bear2Health, bear2Damage, bear2Speed, bear2Exp},
        {"eliteskeleton_shield", eliteSkeletonShieldSprite, eliteSkeletonShieldHealth, eliteSkeletonShieldDamage, eliteSkeletonShieldSpeed, eliteSkeletonShieldExp},
        // {"enemy1", enemy1Sprite, enemy1Health, enemy1Damage, enemy1Speed, enemy1Exp},
        {"ina1", ina1Sprite, ina1Health, ina1Damage, ina1Speed, ina1Exp},
        {"ina2", ina2Sprite, ina2Health, ina2Damage, ina2Speed, ina2Exp},
        {"ina3", ina3Sprite, ina3Health, ina3Damage, ina3Speed, ina3Exp},
        {"kfc1", kfc1Sprite, kfc1Health, kfc1Damage, kfc1Speed, kfc1Exp},
        {"kfc2", kfc2Sprite, kfc2Health, kfc2Damage, kfc2Speed, kfc2Exp},
        {"skeleton1", skeleton1Sprite, skeleton1Health, skeleton1Damage, skeleton1Speed, skeleton1Exp},
        {"skeleton2", skeleton2Sprite, skeleton2Health, skeleton2Damage, skeleton2Speed, skeleton2Exp},
        {"skeleton3", skeleton3Sprite, skeleton3Health, skeleton3Damage, skeleton3Speed, skeleton3Exp},
        {"skeleton4", skeleton4Sprite, skeleton4Health, skeleton4Damage, skeleton4Speed, skeleton4Exp},
        {"skeleton5", skeleton5Sprite, skeleton5Health, skeleton5Damage, skeleton5Speed, skeleton5Exp},
        {"skeleton_shield", skeletonShieldSprite, skeletonShieldHealth, skeletonShieldDamage, skeletonShieldSpeed, skeletonShieldExp}
    };

    // Choose a random enemy type
    int randomEnemyIndex = std::rand() % enemyTypes.size();
    EnemyType selectedEnemy = enemyTypes[randomEnemyIndex];

    // Choose a random spawn point
    int randomIndex = std::rand() % spawnPoints.size();
    Vector2D spawnPosition = spawnPoints[randomIndex];

    // std::cout << "Spawning " << selectedEnemy.tag << " at: ("
    //           << spawnPosition.x << ", " << spawnPosition.y << ")" << std::endl;

    auto& enemy = manager.addEntity();

    assets->AddTexture(selectedEnemy.tag, selectedEnemy.sprite);


    int enemySize = 64;

    enemy.addComponent<TransformComponent>(spawnPosition.x, spawnPosition.y, enemySize, enemySize, 2);
    enemy.addComponent<SpriteComponent>(selectedEnemy.tag, true); // Make enemies animated
    enemy.addComponent<ColliderComponent>(selectedEnemy.tag, 64, 64); // Adjust collider size if needed
    enemy.addComponent<HealthComponent>(selectedEnemy.health, selectedEnemy.health);

    // Pass the member playerEntity's details
    enemy.addComponent<EnemyAIComponent>(
        5000, // Detection range
        selectedEnemy.speed, // Movement speed
        &playerEntity->getComponent<ColliderComponent>().position, // Player position ptr
        selectedEnemy.damage + (playerManager ? playerManager->getLevel() : 0), // Contact damage (scales with player level)
        selectedEnemy.experience, // Experience value
        playerEntity // Player entity ptr
    );

    enemy.addGroup(groupEnemies);
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
                      }
                       if (e->hasComponent<HealthComponent>()) {
                           e->getComponent<HealthComponent>().takeDamage(damage);
                           if (e->getComponent<HealthComponent>().getHealth() <= 0) {
                            // Enemy Death Logic (Exp Orb Spawning)
                            if (playerManager && e->hasComponent<EnemyAIComponent>() && e->hasComponent<ColliderComponent>()) {
                                 int expValue = e->getComponent<EnemyAIComponent>().getExpValue();
                                 Vector2D deathPosition = e->getComponent<ColliderComponent>().position;
                                 std::string orbTextureId = "exp_orb_1"; // Default
                                 if (expValue >= 500) orbTextureId = "exp_orb_500"; else if (expValue >= 250) orbTextureId = "exp_orb_250"; else if (expValue >= 100) orbTextureId = "exp_orb_100"; else if (expValue >= 50) orbTextureId = "exp_orb_50"; else if (expValue >= 10) orbTextureId = "exp_orb_10";

                                 auto& orb = manager.addEntity(); // Use member manager
                                 orb.addComponent<TransformComponent>(deathPosition.x, deathPosition.y, 32, 32, 1);
                                 orb.addComponent<SpriteComponent>(orbTextureId);
                                 // --- ADD COLLIDER COMPONENT FOR ORB ---
                                 orb.addComponent<ColliderComponent>("exp_orb", 16, 16); // Add collider before ExpOrbComponent (adjust size as needed)
                                 // --- END ADD ---
                                 orb.addComponent<ExpOrbComponent>(expValue);
                                 // Orb should be added to groupExpOrbs inside ExpOrbComponent::init now
                            }
                            if(playerManager) playerManager->incrementEnemiesDefeated();
                       }
                       }
                     projComp.recordHit(e);
                     if (projComp.shouldDestroy()) {
                          p->destroy();
                          break; // Exit inner enemy loop
                     }
                 } // End AABB check
             } // End enemy loop
             if (!p->isActive()) continue; // Continue projectile loop if destroyed
        } // End projectile loop


        // Enemy Spawning (consider moving timer logic outside update if needed)
        if (currentTime > lastEnemySpawnTime + 1000) {
            spawnEnemy(); // Uses member manager
            lastEnemySpawnTime = currentTime;
        }

        // Camera Update (uses playerTransform directly)
        camera.x = static_cast<int>(playerTransform.position.x - (WINDOW_WIDTH / 2.0f));
        camera.y = static_cast<int>(playerTransform.position.y - (WINDOW_HEIGHT / 2.0f));

        // Clamp Camera
        if (camera.x < 0) camera.x = 0;
        if (camera.y < 0) camera.y = 0;
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE;
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE;
        if (camera.x > mapPixelWidth - camera.w) camera.x = mapPixelWidth - camera.w;
        if (camera.y > mapPixelHeight - camera.h) camera.y = mapPixelHeight - camera.h;

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
            std::cout << "DEBUG: Played gameover_sfx" << std::endl; // Optional debug log
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
                  std::cout << "DEBUG: Render - Rendering Buff Selection UI" << std::endl;
                  ui->renderBuffSelectionUI(currentBuffOptions);
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


void Game::generateBuffOptions() {
    currentBuffOptions.clear();

    // Define buff amounts (consider making these constants in game.h or constants.h)
    const int SPELL_DAMAGE_INCREASE = 3;
    const int SPELL_COOLDOWN_REDUCTION = 500; // milliseconds
    const int SPELL_PIERCE_INCREASE = 1;
    const int SPELL_COUNT_INCREASE = 1; // For star spell
    const int SPELL_DURATION_INCREASE = 500; // For fire spell (milliseconds)

    const int WEAPON_DAMAGE_INCREASE = 5;
    const int WEAPON_FIRERATE_REDUCTION = 50;
    const int WEAPON_PROJ_COUNT_INCREASE = 1;
    const int WEAPON_PIERCE_INCREASE = 1;
    const int WEAPON_BURST_COUNT_INCREASE = 1;
    const float PLAYER_HEAL_PERCENT = 10.0f;

    // Define all possible buffs using the new specific BuffTypes
    std::vector<BuffInfo> allPossibleBuffs = {
        // --- Fire Spell Buffs ---
        {"Fire Dmg+", "+ Fire Spell Damage", BuffType::FIRE_SPELL_DAMAGE, static_cast<float>(SPELL_DAMAGE_INCREASE)},
        {"Fire CDR", "- Fire Spell Cooldown", BuffType::FIRE_SPELL_COOLDOWN, static_cast<float>(SPELL_COOLDOWN_REDUCTION)},
        {"Fire Pierce+", "+ Fire Spell Pierce", BuffType::FIRE_SPELL_PIERCE, static_cast<float>(SPELL_PIERCE_INCREASE)},
        {"Fire Duration+", "+ Fire Spell Duration", BuffType::FIRE_SPELL_DURATION, static_cast<float>(SPELL_DURATION_INCREASE)},

        // --- Star Spell Buffs ---
        {"Star Dmg+", "+ Star Spell Damage", BuffType::STAR_SPELL_DAMAGE, static_cast<float>(SPELL_DAMAGE_INCREASE)},
        {"Star CDR", "- Star Spell Cooldown", BuffType::STAR_SPELL_COOLDOWN, static_cast<float>(SPELL_COOLDOWN_REDUCTION)},
        {"Star Pierce+", "+ Star Spell Pierce", BuffType::STAR_SPELL_PIERCE, static_cast<float>(SPELL_PIERCE_INCREASE)},
        {"Star Count+", "+ Star Spell Proj. Count", BuffType::STAR_SPELL_PROJ_COUNT, static_cast<float>(SPELL_COUNT_INCREASE)},

        // --- Weapon Buffs ---
        {"Wpn Dmg+", "+ Weapon Damage", BuffType::WEAPON_DAMAGE, static_cast<float>(WEAPON_DAMAGE_INCREASE)},
        {"Wpn FireRate+", "+ Weapon Fire Rate", BuffType::WEAPON_FIRE_RATE, static_cast<float>(WEAPON_FIRERATE_REDUCTION)}, // Note: Amount is reduction
        {"Wpn Count+", "+ Weapon Proj. Count", BuffType::WEAPON_PROJ_COUNT, static_cast<float>(WEAPON_PROJ_COUNT_INCREASE)},
        {"Wpn Pierce+", "+ Weapon Pierce", BuffType::WEAPON_PIERCE, static_cast<float>(WEAPON_PIERCE_INCREASE)},
        {"Wpn Burst+", "+ Weapon Burst Count", BuffType::WEAPON_BURST_COUNT, static_cast<float>(WEAPON_BURST_COUNT_INCREASE)},

        // --- Player Buffs ---
        {"Heal", "Heal 10% Max HP", BuffType::PLAYER_HEAL, PLAYER_HEAL_PERCENT}
    };

    int numPossible = allPossibleBuffs.size();
    if (numPossible == 0) return;

    // Randomly select buffs (ensure uniqueness) - unchanged logic
    std::vector<int> chosenIndices;
    while(currentBuffOptions.size() < 4 && chosenIndices.size() < static_cast<size_t>(numPossible)) {
         int randIndex = std::rand() % numPossible;
         bool alreadyChosen = false;
         for(int chosen : chosenIndices) {
             if (chosen == randIndex) {
                 alreadyChosen = true;
                 break;
             }
         }
         if (!alreadyChosen) {
             chosenIndices.push_back(randIndex);
             currentBuffOptions.push_back(allPossibleBuffs[randIndex]);
         }
         if (chosenIndices.size() >= static_cast<size_t>(numPossible)) break;
    }

    // std::cout << "Generated " << currentBuffOptions.size() << " buff options." << std::endl; // Optional debug
}

// --- enterBuffSelection & exitBuffSelection (Unchanged) ---
// In game.cpp:
void Game::enterBuffSelection() {
    // Should only enter if currently playing
    if (currentState == GameState::Playing) {
        std::cout << "Entering Buff Selection..." << std::endl;
        isInBuffSelection = true;
        currentState = GameState::Paused; // <<< SET GameState to Paused
        // REMOVED: isPaused = true;
        if (Mix_PlayingMusic()) Mix_PauseMusic(); // Pause music for buff selection
        generateBuffOptions();
        // Optional: Call calculatePauseLayout if buff UI depends on it? Unlikely.
    } else {
         std::cout << "Warning: Tried to enter buff selection when not in Playing state." << std::endl;
    }
}
void Game::exitBuffSelection() {
    if (isInBuffSelection) { // Only act if we are actually in buff selection
       std::cout << "Exiting Buff Selection..." << std::endl;
       isInBuffSelection = false;
       currentState = GameState::Playing; // <<< SET GameState back to Playing
       // REMOVED: isPaused = false;
       currentBuffOptions.clear();
       if (Mix_PausedMusic()) Mix_ResumeMusic(); // Resume music after selection
    }
}


// --- Updated applySelectedBuff (Uses Enum Switch) ---
void Game::applySelectedBuff(int index) {
    if (!isInBuffSelection || !playerEntity || index < 0 || index >= static_cast<int>(currentBuffOptions.size())) {
        // Exit if not in buff selection, player doesn't exist, or index is invalid
        if (isInBuffSelection) exitBuffSelection(); // Still exit if selection is active but apply failed
        return;
    }

    const BuffInfo& selectedBuff = currentBuffOptions[index];
    int intAmount = static_cast<int>(selectedBuff.amount);
    float floatAmount = selectedBuff.amount;

    std::cout << "Applying buff: " << selectedBuff.name << " (Type: " << static_cast<int>(selectedBuff.type) << ")" << std::endl; // Debug

    // Handle buffs that don't need component iteration first (Weapon, Player)
    WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
    HealthComponent* healthComp = playerEntity->hasComponent<HealthComponent>() ? &playerEntity->getComponent<HealthComponent>() : nullptr;

    bool buffApplied = false; // Flag to track if any buff was successfully applied

    switch (selectedBuff.type) {
        // --- Weapon Buffs ---
        case BuffType::WEAPON_DAMAGE:
            if(weaponComp) { weaponComp->increaseDamage(intAmount); buffApplied = true; }
            break;
        case BuffType::WEAPON_FIRE_RATE:
            if(weaponComp) { weaponComp->decreaseFireRate(intAmount); buffApplied = true; }
            break;
        case BuffType::WEAPON_PROJ_COUNT:
            if(weaponComp) { weaponComp->increaseProjectileCount(intAmount); buffApplied = true; }
            break;
        case BuffType::WEAPON_PIERCE:
            if(weaponComp) { weaponComp->increasePierce(intAmount); buffApplied = true; }
            break;
        case BuffType::WEAPON_BURST_COUNT:
            if(weaponComp) { weaponComp->increaseBurstCount(intAmount); buffApplied = true; }
            break;

        // --- Player Buffs ---
        case BuffType::PLAYER_HEAL:
            if (healthComp) {
                int maxHP = healthComp->getMaxHealth();
                int healAmount = static_cast<int>(maxHP * (floatAmount / 100.0f));
                healthComp->heal(healAmount);
                std::cout << "Player healed for " << healAmount << " HP." << std::endl; // Debug
                buffApplied = true;
            } else {
                 std::cerr << "Error: Player has no HealthComponent to heal!" << std::endl;
            }
            break;

        // --- Spell Buffs (These require iteration) ---
        case BuffType::FIRE_SPELL_DAMAGE:
        case BuffType::FIRE_SPELL_COOLDOWN:
        case BuffType::FIRE_SPELL_PIERCE:
        case BuffType::FIRE_SPELL_DURATION: // Assuming you added increaseDuration to SpellComponent
        case BuffType::STAR_SPELL_DAMAGE:
        case BuffType::STAR_SPELL_COOLDOWN:
        case BuffType::STAR_SPELL_PIERCE:
        case BuffType::STAR_SPELL_PROJ_COUNT:
            { // Scope for spellComp iteration
                // Use the new getAllComponents() method (requires adding it to Entity class)
                for (const auto& compPtr : playerEntity->getAllComponents()) {
                    // Try to cast the component pointer to SpellComponent*
                    if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
                        // Check the tag and apply the buff if it matches the type
                        switch (selectedBuff.type) {
                            // Fire Spell ("spell" tag)
                            case BuffType::FIRE_SPELL_DAMAGE:   if (spellComp->tag == "spell") { spellComp->increaseDamage(intAmount); buffApplied = true; } break;
                            case BuffType::FIRE_SPELL_COOLDOWN: if (spellComp->tag == "spell") { spellComp->decreaseCooldown(intAmount); buffApplied = true; } break;
                            case BuffType::FIRE_SPELL_PIERCE:   if (spellComp->tag == "spell") { spellComp->increasePierce(intAmount); buffApplied = true; } break;
                            case BuffType::FIRE_SPELL_DURATION: if (spellComp->tag == "spell") { /* spellComp->increaseDuration(intAmount); buffApplied = true; */ } break; // Add increaseDuration if needed

                            // Star Spell ("star" tag)
                            case BuffType::STAR_SPELL_DAMAGE:   if (spellComp->tag == "star") { spellComp->increaseDamage(intAmount); buffApplied = true; } break;
                            case BuffType::STAR_SPELL_COOLDOWN: if (spellComp->tag == "star") { spellComp->decreaseCooldown(intAmount); buffApplied = true; } break;
                            case BuffType::STAR_SPELL_PIERCE:   if (spellComp->tag == "star") { spellComp->increasePierce(intAmount); buffApplied = true; } break;
                            case BuffType::STAR_SPELL_PROJ_COUNT:if (spellComp->tag == "star") { spellComp->increaseProjectileCount(intAmount); buffApplied = true; } break;

                            default: break; // Ignore other buff types handled outside the loop
                        }
                    }
                }
            } // End scope for spellComp iteration
            break; // Break from the outer switch for spell buff types

        // --- Invalid/Default ---
        case BuffType::INVALID: // Fallthrough intended
        default:
            std::cerr << "Warning: Invalid or unknown BuffType selected (" << static_cast<int>(selectedBuff.type) << ")!" << std::endl;
            break;
    }

    if (!buffApplied) {
         std::cerr << "Warning: Selected buff '" << selectedBuff.name << "' could not be applied (missing component or incorrect type)." << std::endl;
    }

    // Exit selection state regardless of whether buff was applied (user made a choice)
    exitBuffSelection();
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