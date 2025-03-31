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

    // --- Load default state ---
    // loadGameState will attempt to overwrite the placeholder values in the components
    bool loadedSuccessfully = false;
    if (saveLoadManager) {
        loadedSuccessfully = saveLoadManager->loadGameState("saves/default.state");
    } else {
         std::cerr << "Error: SaveLoadManager not initialized!" << std::endl;
    }

    // --- Handle Load Success/Failure ---
    if (!loadedSuccessfully) {
        std::cerr << "Warning: Could not load default.state. Initializing components with default game start values." << std::endl;

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
        musicVolume = MIX_MAX_VOLUME / 2;
        sfxVolume = MIX_MAX_VOLUME / 2;
        Mix_VolumeMusic(musicVolume);
        Mix_Volume(-1, sfxVolume);
        // --- End Setting Default Stats ---

    } else {
        // Load successful. Data was loaded into components.
        std::cout << "Loaded default.state successfully. Settings: MusicVol=" << musicVolume << ", SfxVol=" << sfxVolume << std::endl;
        // Ensure health isn't somehow invalid after load (e.g. 0 max health)
        if (playerEntity->hasComponent<HealthComponent>()) {
             auto& healthComp = playerEntity->getComponent<HealthComponent>();
             if (healthComp.maxHealth <= 0) healthComp.maxHealth = 1; // Ensure maxHealth > 0
             if (healthComp.health > healthComp.maxHealth) healthComp.health = healthComp.maxHealth;
             if (healthComp.health <= 0) healthComp.health = 1; // Ensure health > 0 if max is > 0
        }
    }
    // --- End Handling ---

    // --- Load Map ---
    delete map;
    map = new Map(manager, "terrain", 1, 32);
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10, spawnPoints);

    isRunning = true;
}



// --- Modify Game::handleEvents ---
void Game::handleEvents() {
    // Event polling happens in main loop first (SDL_PollEvent(&Game::event))

    // Handle general events like quit FIRST
    if (Game::event.type == SDL_QUIT) {
        Game::isRunning = false;
        return; // Exit handling if quitting
    }

    // --- Check for PAUSE/UNPAUSE key ('E') ---
    // Handle this early and return immediately after toggling.
    if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_e) {
        togglePause();
        return; // Return here to prevent 'E' key affecting other logic in the same frame
    }

    // --- Handle Buff Selection Input (If Active) ---
    if (isInBuffSelection) { // Check Buff Selection FIRST
        if (Game::event.type == SDL_MOUSEBUTTONDOWN) {
            if (Game::event.button.button == SDL_BUTTON_LEFT) {
                int mouseX_Screen, mouseY_Screen;
                SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);

                // Calculate button layout again (ensure this matches UI::renderBuffSelectionUI)
                int totalButtonWidth = 4 * 180 + 3 * 20;
                int startX = (WINDOW_WIDTH - totalButtonWidth) / 2;
                int buttonY = WINDOW_HEIGHT / 2 - 50;
                int buttonW = 180;
                int buttonH = 100;
                int gap = 20;

                 for (size_t i = 0; i < currentBuffOptions.size() && i < 4; ++i) {
                    SDL_Rect buttonRect = {startX + static_cast<int>(i) * (buttonW + gap), buttonY, buttonW, buttonH};
                    if (ui && ui->isMouseInside(mouseX_Screen, mouseY_Screen, buttonRect)) {
                        applySelectedBuff(static_cast<int>(i));
                        // Buff applied, exit event handling for this frame
                        return; // <<< CRUCIAL: Return after handling mouse click
                    }
                 }
            }
        }
        if (Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
                case SDLK_1: applySelectedBuff(0); return; // <<< CRUCIAL: Return
                case SDLK_2: applySelectedBuff(1); return; // <<< CRUCIAL: Return
                case SDLK_3: applySelectedBuff(2); return; // <<< CRUCIAL: Return
                case SDLK_4: applySelectedBuff(3); return; // <<< CRUCIAL: Return
                // case SDLK_ESCAPE: exitBuffSelection(); return; // Optional cancel
            }
        }
        // If we are in buff selection mode, but the specific event wasn't
        // a buff selection action (e.g., mouse move, other key),
        // still stop processing input here. Do not fall through.
        return; // <<< CRUCIAL: Add this return at the end of the block
    }

    // --- Handle PAUSED state input ---
    // This block is ONLY reached if isPaused is true AND isInBuffSelection is false
    if (isPaused) {
        if (Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
                // Music Volume Control
                case SDLK_UP:
                    std::cout << "DEBUG: SDLK_UP detected while paused." << std::endl;
                    changeMusicVolume(VOLUME_STEP);
                    break;
                case SDLK_DOWN:
                    std::cout << "DEBUG: SDLK_DOWN detected while paused." << std::endl;
                    changeMusicVolume(-VOLUME_STEP);
                    break;
                // SFX Volume Control
                case SDLK_LEFT:
                    std::cout << "DEBUG: SDLK_LEFT detected while paused." << std::endl;
                    changeSfxVolume(-VOLUME_STEP);
                    break;
                case SDLK_RIGHT:
                    std::cout << "DEBUG: SDLK_RIGHT detected while paused." << std::endl;
                    changeSfxVolume(VOLUME_STEP);
                    break;
                // --- Save/Load Keys ---
                case SDLK_F5: // Quick Save
                    if (saveLoadManager) {
                        saveLoadManager->saveGameState(); // Use the manager to save
                    } else {
                         std::cerr << "Error: Cannot save, SaveLoadManager is null!" << std::endl;
                    }
                    break;
                case SDLK_F9: // Quick Load
                    if (saveLoadManager) {
                        // Quick load needs logic to find the latest save file.
                        // For now, let's load a specific file like 'quicksave.state'
                        // or just print a message.
                        std::cout << "Attempting Quick Load (loading quicksave.state)..." << std::endl;
                        if (saveLoadManager->loadGameState("quicksave.state")) {
                           // Optional: Add feedback on successful load
                           std::cout << "Quicksave loaded." << std::endl;
                           // Ensure game is unpaused after load
                           if(isPaused) togglePause();
                        } else {
                           std::cout << "Failed to load quicksave.state." << std::endl;
                        }
                    } else {
                         std::cerr << "Error: Cannot load, SaveLoadManager is null!" << std::endl;
                    }
                    break;
                // --- End Save/Load Keys ---
                 case SDLK_ESCAPE:
                     // Optional: unpause before switching? togglePause();
                     SceneManager::instance->switchToScene(SceneType::Menu);
                     break;
                default:
                    break;
            }
        }
        // If the game is paused (and not in buff selection), prevent fall-through.
        return; // <<< CRUCIAL: Stops input from reaching game logic while paused
    }

    // --- Handle GAMEPLAY input (Only if NOT paused and NOT in buff selection) ---
    int mouseX_Screen, mouseY_Screen;
    SDL_GetMouseState(&mouseX_Screen, &mouseY_Screen);
    Game::mouseX = mouseX_Screen + Game::camera.x;
    Game::mouseY = mouseY_Screen + Game::camera.y;

    // Gameplay actions (movement, firing) are likely handled in component updates.
    // Handle any specific single-press gameplay keys here if necessary.

} // End Game::handleEvents


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
    if (!isRunning || !playerEntity) return; // Check running state and player

    // --- Use member 'manager' ---
    manager.refresh();

    Uint32 currentTime = SDL_GetTicks();
    if(!isPaused && !isInBuffSelection){ // Check buff selection state too

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


void Game::render(){
    if (!Game::renderer) return;
    SDL_RenderClear(Game::renderer);

    // --- Get groups using member 'manager' ---
    auto& tiles = manager.getGroup(Game::groupMap);
    // auto& players = manager.getGroup(Game::groupPlayers); // Only one player, use playerEntity
    auto& projectiles = manager.getGroup(Game::groupProjectiles);
    auto& enemies = manager.getGroup(Game::groupEnemies);
    auto& expOrbs = manager.getGroup(Game::groupExpOrbs);
    // auto& colliders = manager.getGroup(Game::groupColliders); // Only needed for debug drawing

    // Render game world only if not in buff selection AND not paused
    if (!isInBuffSelection && !isPaused) {
        for(auto* t : tiles) if(t && t->isActive()) t->draw();
        for(auto* p : projectiles) if(p && p->isActive()) p->draw();
        for(auto* e : enemies) if(e && e->isActive()) e->draw(); // Draw enemy sprites
        if(playerEntity && playerEntity->isActive()) playerEntity->draw(); // Draw player sprite
        for(auto* o : expOrbs) if(o && o->isActive()) o->draw();

        // Draw health bars AFTER sprites
        for(auto* e : enemies) {
            if (e && e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>()) {
                 // Use ColliderComponent's position member which is updated in its update()
                 Vector2D enemyPos = e->getComponent<ColliderComponent>().position;
                 renderHealthBar(*e, enemyPos);
             }
        }
        // Draw UI last
        if (ui && playerManager) {
            ui->renderUI(playerManager);
        }
    }
    // --- PAUSE MENU / BUFF SELECTION RENDERING ---
    else {
         // Render world dimly underneath
         SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Enable blending for dimming overlay
         for(auto* t : tiles) if(t && t->isActive()) t->draw();
         for(auto* p : projectiles) if(p && p->isActive()) p->draw();
         for(auto* e : enemies) if(e && e->isActive()) e->draw();
         if(playerEntity && playerEntity->isActive()) playerEntity->draw();
         for(auto* o : expOrbs) if(o && o->isActive()) o->draw();
         for(auto* e : enemies) {
             if (e && e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>()) {
                  Vector2D enemyPos = e->getComponent<ColliderComponent>().position;
                  renderHealthBar(*e, enemyPos);
              }
         }
         if (ui && playerManager) { // Render UI underneath overlay too
             ui->renderUI(playerManager);
         }

         // Draw semi-transparent overlay
         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Semi-transparent black
         SDL_Rect fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
         SDL_RenderFillRect(Game::renderer, &fullscreen);
         SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE); // Disable blending


        // Render Buff Selection UI if active (drawn ON TOP of overlay)
        if (isInBuffSelection && ui) {
            ui->renderBuffSelectionUI(currentBuffOptions);
        }
        // Render General Pause Overlay (if paused BUT not in buff selection)
        else if (isPaused && ui) {
             // ... (Draw "PAUSED", Volume Controls, Save/Load text - unchanged) ...
             SDL_Color white = {255, 255, 255, 255};
             TTF_Font* fontToUse = ui->getLargeFont() ? ui->getLargeFont() : ui->getFont();
             if (fontToUse) {
                  int textW, textH;
                  SDL_Texture* tempTex = ui->renderTextToTexture("PAUSED", white, fontToUse, textW, textH);
                  if(tempTex) {
                      ui->drawText("PAUSED", WINDOW_WIDTH / 2 - textW / 2, WINDOW_HEIGHT / 3 - textH / 2, white, fontToUse);
                      SDL_DestroyTexture(tempTex);
                  }
                  int volY = WINDOW_HEIGHT / 2;
                  int volX = WINDOW_WIDTH / 2 - 150;
                  std::stringstream ssMusic;
                  ssMusic << "Music Volume: " << static_cast<int>(round(musicVolume * 100.0 / MIX_MAX_VOLUME)) << "% (Up/Down)";
                  ui->drawText(ssMusic.str(), volX, volY, white, ui->getFont());
                  volY += 30;
                  std::stringstream ssSfx;
                  ssSfx << "SFX Volume:   " << static_cast<int>(round(sfxVolume * 100.0 / MIX_MAX_VOLUME)) << "% (Left/Right)";
                  ui->drawText(ssSfx.str(), volX, volY, white, ui->getFont());
                  volY += 40;
                  ui->drawText("F5: Save Game | F9: Load Quicksave", volX, volY, white, ui->getFont()); // Show Save/Load Keys
                   volY += 40;
                   ui->drawText("Press 'E' to Resume", volX + 50, volY, white, ui->getFont()); // Adjusted X offset
             }
        }
    }

    SDL_RenderPresent(Game::renderer);
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
void Game::enterBuffSelection() {
    if (!isInBuffSelection) {
        // std::cout << "Entering Buff Selection..." << std::endl;
        isInBuffSelection = true;
        isPaused = true; // Use instance member pause flag
        generateBuffOptions(); // Call instance method

    }
 }
void Game::exitBuffSelection() {
     if (isInBuffSelection) {
        // std::cout << "Exiting Buff Selection..." << std::endl;
        isInBuffSelection = false;
        isPaused = false; // Use instance member pause flag
        currentBuffOptions.clear(); // Use instance member

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
    isPaused = !isPaused;
    std::cout << "togglePause called, new state: " << (isPaused ? "Paused" : "Running") << std::endl;

    if (isPaused) {
        // Pause the music when the game is paused
        if (Mix_PlayingMusic()) { // Check if music is playing before pausing
            Mix_PauseMusic();
            std::cout << "Music Paused." << std::endl;
        }
    } else {
        // Resume the music when the game is unpaused
        if (Mix_PausedMusic()) { // Check if music was paused before resuming
            Mix_ResumeMusic();
            std::cout << "Music Resumed." << std::endl;
        }
    }

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