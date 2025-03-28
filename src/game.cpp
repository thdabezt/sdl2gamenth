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


// Define the static instance
Game* Game::instance = nullptr;
Map *map;
SDL_Event Game::event;
std::vector<Vector2D> spawnPoints;
SDL_Renderer *Game::renderer = nullptr;
int Game::mouseX = 0;
int Game::mouseY = 0;
Manager manager; // Note: This manager is global/static relative to this file. Consider ownership.

SDL_Rect Game::camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

// Note: AssetManager is static. Be mindful of its lifecycle if resetting games.
AssetManager *Game::assets = new AssetManager(&manager);
bool Game::isRunning = false;
bool godmode = false;
// auto& player(manager.addEntity()); // REMOVED global player

Game::Game() {
    instance = this;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    playerEntity = nullptr; // Initialize member pointer
    playerManager = nullptr; // Initialize member pointer
    ui = nullptr; // Initialize member pointer
    // window member is removed
}

Game::~Game(){
    // Destructor calls clean()
    std::cout << "Game destructor (~Game) called." << std::endl;
    clean();
    instance = nullptr; // Reset static instance pointer
}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen){
    int flags = 0;
    if(fullscreen){
        flags = SDL_WINDOW_FULLSCREEN;
    }
    // SDL_Init, TTF_Init, IMG_Init, Window/Renderer creation are now handled in main.cpp

    // Use the renderer provided by main.cpp (via static Game::renderer)
    if (Game::renderer) {
        std::cout << "Game::init - Using existing Renderer" << std::endl;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // Initialize UI manager
        ui = new UIManager(renderer);
        ui->init();
        isRunning = true;
    } else {
         std::cerr << "Error: Game::init called but Game::renderer is null!" << std::endl;
         isRunning = false;
         return; // Cannot proceed without a renderer
    }
    Mix_VolumeMusic(musicVolume);
    Mix_Volume(-1, sfxVolume); // Set default volume for all sound effect channels
    // Refresh manager before adding entities for this game instance
    manager.refresh(); // Clean up entities from previous game if manager persists

    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");
    assets->AddTexture("fire", "sprites/projectile/fire.png");
    assets->AddTexture("starproj", "sprites/projectile/star.png");
    // Create and setup player entity as member
    playerEntity = &manager.addEntity(); // Create the entity for this Game instance
    playerEntity->addComponent<TransformComponent>(400.0f, 320.0f, CHAR_W, CHAR_H, 2);
    playerEntity->addComponent<SpriteComponent>("player", true);
    playerEntity->addComponent<KeyboardController>();
    playerEntity->addComponent<ColliderComponent>("player", 32 , 37);
    playerEntity->addComponent<HealthComponent>(99999999, 99999999); // High health for testing?
    
    // Create a pistol (small bullets)
    assets->AddSoundEffect("gunshot_sound", "assets/sound/shot.wav"); // Assuming path
    assets->AddMusic("level_music", "assets/sound/hlcbg.mp3"); // Assuming path

    // Add SoundComponent to player
    playerEntity->addComponent<SoundComponent>();
    playerEntity->getComponent<SoundComponent>().addSoundEffect("shoot", "gunshot_sound"); // Map internal name "shoot"
    playerEntity->getComponent<SoundComponent>().setBackgroundMusic("level_music", true, -1); // Set background music to play on start
 
    playerEntity->addComponent<WeaponComponent>(
        "pistol",     // tag
        25,           // damage
        500,          // fireRate (time between bursts)
        5.0f,         // projectileSpeed
        500,          // projectileRange
        0.1f,         // spreadAngle (small spread for burst accuracy?)
        1,            // projectilesPerShot (1 projectile per shot in the burst)
        32,           // projectileSize
        "projectile", // projectileTexture
        1,            // projectilePierce
        3,            // shotsPerBurst (Fires 3 shots per trigger pull)
        75            // burstDelay (75ms between shots in the burst)
    );
    playerEntity->addGroup(groupPlayers);

    playerEntity->addComponent<SpellComponent>(
        "spell",       // tag
        5,                    // damage
        100,                  // cooldown (ms) - Maybe faster cooldown for visible spiral?
        3.0f,                 // projectileSpeed
        0,                    // projectilesPerCast
        38,                   // projectileSize
        "fire",         // projectileTexture
        3000,                 // duration (ms)
        SpellTrajectory::SPIRAL, // trajectoryMode
        8.0f,
        2                // spiralGrowthRate (pixels per radian, adjust as needed)                
    );
    playerEntity->addComponent<SpellComponent>(
        "star",       // tag
        5,                    // damage
        100,                  // cooldown (ms) - Maybe faster cooldown for visible spiral?
        3.0f,                 // projectileSpeed
        1,                    // projectilesPerCast
        38,                   // projectileSize
        "starproj",         // projectileTexture
        3000,                 // duration (ms)
        SpellTrajectory::RANDOM_DIRECTION, // trajectoryMode
        8.0f,
        2                // spiralGrowthRate (pixels per radian, adjust as needed)                
    );
    // Increase damage of spiral bolts by 1
    
    // Initialize the player manager with the member entity
    // Delete old one first if it somehow exists (shouldn't if constructor is clean)
    delete playerManager;
    playerManager = new Player(playerEntity);

    map = new Map("terrain", 1, 32);
    // Ensure map file path is correct relative to executable
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10);


}

// Note: These manager.getGroup calls might be problematic if manager is truly global
// and game instances are created/destroyed. Consider passing manager or getting groups inside methods.
auto& tiles(manager.getGroup(Game::groupMap));
auto& players(manager.getGroup(Game::groupPlayers));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& projectiles(manager.getGroup(Game::groupProjectiles));
auto& enemies(manager.getGroup(Game::groupEnemies));




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

    // --- Handle PAUSED state input (Volume controls, etc.) ---
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
                // Allow ESC to go to Menu from pause screen?
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

    // Add the enemy sprite texture
    assets->AddTexture(selectedEnemy.tag, selectedEnemy.sprite);


    int enemySize = 64;

    enemy.addComponent<TransformComponent>(spawnPosition.x, spawnPosition.y, enemySize, enemySize, 2);
    enemy.addComponent<SpriteComponent>(selectedEnemy.tag, true);
    enemy.addComponent<ColliderComponent>(selectedEnemy.tag, 64, 64);
    enemy.addComponent<HealthComponent>(selectedEnemy.health, selectedEnemy.health);

    // Pass the member playerEntity's details
    enemy.addComponent<EnemyAIComponent>(
        5000, // Detection range
        selectedEnemy.speed, // Movement speed
        &playerEntity->getComponent<TransformComponent>().position, // Player position ptr
        selectedEnemy.damage + (playerManager ? playerManager->getLevel() : 0), // Contact damage
        selectedEnemy.experience, // Experience value
        playerEntity // Player entity ptr
    );

    enemy.addGroup(groupEnemies);
}



void Game::update(){
    // std::cout << "Game::update called, isPaused = " << isPaused << std::endl;
    if (!playerEntity) return;

    manager.refresh(); // Refresh entities at the start of update

    Uint32 currentTime = SDL_GetTicks();
    if(!isPaused){


        if (currentTime > lastEnemySpawnTime + 1000) { // Reuse existing timer check for simplicity
            // Or use SDL_GetTicks() % 3000 == 0 to check every few seconds
        if (Mix_PlayingMusic()) {
        std::cout << "DEBUG: Mix_PlayingMusic() returns TRUE." << std::endl;
        } else {
        std::cout << "DEBUG: Mix_PlayingMusic() returns FALSE." << std::endl;
        }
        // Mix_VolumeMusic(-1) gets the current volume without changing it.
        std::cout << "DEBUG: Music Volume is " << Mix_VolumeMusic(-1) << std::endl;
        }

        // Use playerEntity
        Vector2D playerPosStart = playerEntity->getComponent<TransformComponent>().position;
        ColliderComponent& playerCollider = playerEntity->getComponent<ColliderComponent>();
        Vector2D realposition;
        realposition.x = playerCollider.collider.x;
        realposition.y = playerCollider.collider.y;

        manager.update(); // Update all active entities and components

        // --- Collision Handling ---
        SDL_Rect playerCol = playerCollider.collider;
        TransformComponent& playerTransform = playerEntity->getComponent<TransformComponent>();

        // Player vs Terrain collision
        for (auto& c : colliders) {
            ColliderComponent& obstacleCollider = c->getComponent<ColliderComponent>();
            SDL_Rect cCol = obstacleCollider.collider;
            // Ensure it's terrain and collision occurs
            if (obstacleCollider.tag == "terrain" && Collision::AABB(playerCol, cCol)) {
                // --- (Simplified Collision Resolution - replace with your preferred method) ---
                 float overlapX = std::min(playerCol.x + playerCol.w, cCol.x + cCol.w) - std::max(playerCol.x, cCol.x);
                 float overlapY = std::min(playerCol.y + playerCol.h, cCol.y + cCol.h) - std::max(playerCol.y, cCol.y);

                 // Simple push-out based on smaller overlap
                 if (overlapX < overlapY) {
                     playerTransform.position.x += (playerTransform.position.x > playerPosStart.x ? -overlapX : overlapX);
                 } else {
                     playerTransform.position.y += (playerTransform.position.y > playerPosStart.y ? -overlapY : overlapY);
                 }
                 playerCollider.update(); // Update collider position after adjustment
                 playerCol = playerCollider.collider; // Update rect for subsequent checks
                // --- (End Simplified Collision Resolution) ---
            }
        }

        // Projectile vs Enemy collision
        for (auto& p : projectiles) {
            // Ensure projectile p and its components are valid before accessing
            if (!p || !p->isActive() || !p->hasComponent<ColliderComponent>() || !p->hasComponent<ProjectileComponent>() || !p->hasComponent<TransformComponent>()) continue;

            // Iterate through enemies to check for collisions
            for (auto& e : enemies) {
                 // Ensure enemy e and its components are valid before accessing
                if (!e || !e->isActive() || !e->hasComponent<ColliderComponent>()) continue;

                SDL_Rect enemyCollider = e->getComponent<ColliderComponent>().collider;
                SDL_Rect projectileCollider = p->getComponent<ColliderComponent>().collider;

                if (Collision::AABB(enemyCollider, projectileCollider)) {
                    ProjectileComponent& projComp = p->getComponent<ProjectileComponent>();
                    Entity* enemyEntity = e; // Get pointer to the enemy entity

                    // --- Check if this enemy has ALREADY been hit by this projectile ---
                    // Note: Assumes ProjectileComponent has hasHit(), recordHit(), shouldDestroy() methods
                    // You might need to implement these in ProjectileComponent.h/.cpp if not already done.
                    // Example implementation: hasHit uses a std::set<Entity*>, recordHit adds to it, shouldDestroy compares set size to a pierce count.
                    if (!projComp.hasHit(enemyEntity)) {
                        // --- Enemy NOT hit before by this projectile ---

                        // std::cout << "Projectile hit NEW enemy!" << std::endl;
                        int damage = projComp.getDamage();

                        // --- Apply effects to enemy ---

                        // Apply Knockback
                        if (e->hasComponent<TransformComponent>()) {
                            Vector2D knockbackDir = p->getComponent<TransformComponent>().velocity.Normalize();
                            float knockbackForce = 35.0f;
                            // Optional: Add knockback vs terrain check here if desired
                            e->getComponent<TransformComponent>().position.x += knockbackDir.x * knockbackForce;
                            e->getComponent<TransformComponent>().position.y += knockbackDir.y * knockbackForce;
                            e->getComponent<ColliderComponent>().update(); // Update collider after knockback
                        }

                        // Apply Hit Tint
                        if (e->hasComponent<SpriteComponent>()) {
                           e->getComponent<SpriteComponent>().isHit = true;
                           e->getComponent<SpriteComponent>().hitTime = currentTime;
                        }

                        // Apply Damage
                        if (e->hasComponent<HealthComponent>()) {
                            e->getComponent<HealthComponent>().takeDamage(damage);
                            if (e->getComponent<HealthComponent>().getHealth() <= 0) {
                                // Enemy died
                                if (playerManager && e->hasComponent<EnemyAIComponent>()) {
                                     playerManager->addExperience(e->getComponent<EnemyAIComponent>().getExpValue());
                                     playerManager->incrementEnemiesDefeated();
                                     
                                }
                                // Note: Enemy entity is marked for destruction by takeDamage when health <= 0
                            }
                        }
                        // --- End Apply Effects ---

                        // --- Record Hit & Check Pierce ---
                        projComp.recordHit(enemyEntity); // Record hit on THIS specific enemy

                        if (projComp.shouldDestroy()) { // Check if unique hit count reached pierce limit
                             p->destroy();
                             break; // Exit inner enemy loop for this projectile (it's destroyed)
                        }
                        // --- End Record Hit & Check Pierce ---

                    } // End if (!projComp.hasHit(enemyEntity))

                } // End if (Collision::AABB...)

            } // End enemy loop (for auto& e : enemies)

            // Important: If the projectile was destroyed inside the enemy loop (by break),
            // we need to continue to the next projectile without finishing this iteration.
            if (!p->isActive()) continue;

        } // End projectile loop (for auto& p : projectiles)

        // Enemy Spawning
        if (currentTime > lastEnemySpawnTime + 1000) { // Spawn interval
            spawnEnemy();
            lastEnemySpawnTime = currentTime;
        }

        // Camera Update (Track playerEntity)
        camera.x = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.x - (WINDOW_WIDTH / 2.0f));
        camera.y = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.y - (WINDOW_HEIGHT / 2.0f));

        // Clamp Camera
        if (camera.x < 0) camera.x = 0;
        if (camera.y < 0) camera.y = 0;
        // Use constants for map dimensions if available, otherwise calculate
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE; // Assuming TILE_SIZE is accessible or defined
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE;
        if (camera.x > mapPixelWidth - camera.w) camera.x = mapPixelWidth - camera.w;
        if (camera.y > mapPixelHeight - camera.h) camera.y = mapPixelHeight - camera.h;

    } else {
        // Game is paused - handle pause logic if any (e.g., update pause menu animations)

    }
}



// Updated clean method (called by destructor)
void Game::clean(){
    std::cout << "Game::clean() called." << std::endl;

    // Mark the player entity for destruction
    if (playerEntity) {
        playerEntity->destroy(); // Manager will remove it on next refresh
        playerEntity = nullptr; // Reset pointer
        std::cout << "Player entity marked for destruction." << std::endl;
    }
    for(auto& e : enemies) {
        e->destroy();
    }
    for(auto& p : projectiles) {
        p->destroy();
    }
     if (assets) {

         std::cout << "AssetManager cleanup skipped (assuming static)." << std::endl;
     }
     if (map) {
         delete map;
         map = nullptr;
         std::cout << "Map cleaned." << std::endl;
     }
     if (ui) {
         delete ui;
         ui = nullptr;
         std::cout << "UIManager cleaned." << std::endl;
     }
     if (playerManager) {
         delete playerManager;
         playerManager = nullptr;
         std::cout << "Player manager cleaned." << std::endl;
     }

     // Consider clearing or resetting the manager if entities shouldn't persist
     // manager.clearAllEntities(); // Example if such a method exists

    // Window, Renderer, SDL subsystems are cleaned in main.cpp
    std::cout<< "Game instance resources cleaned."<< std::endl;
}


void Game::rezero(){
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
    if (health.getHealth() <= 0) return; // Don't render for dead entities

    // Calculate health bar dimensions and position
    int barWidth = 40;
    int barHeight = 5;
    // Use entity's collider position for rendering bar relative to it
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

    // ... (get entity groups) ...

    // Render game world only if not in buff selection AND not paused (or optionally render world dimly if paused)
    if (!isInBuffSelection && !isPaused) { // Render normally if not paused/in buff select
        for(auto& t : tiles) if(t->isActive()) t->draw();
        // for(auto& c : colliders) if(c->isActive()) c->draw(); // Draw terrain colliders if needed for debug
        for(auto& p : projectiles) if(p->isActive()) p->draw();
        for(auto& e : enemies) if(e->isActive()) {e->draw();}
        if(playerEntity && playerEntity->isActive()) playerEntity->draw();

        for(auto& e : enemies) {
            if (e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>()) {
                Vector2D enemyPos = e->getComponent<ColliderComponent>().position;
                renderHealthBar(*e, enemyPos);
            }
        }
        if (ui && playerManager) {
            ui->renderUI(playerManager);
        }
    }
    // --- PAUSE MENU / BUFF SELECTION RENDERING ---
    else {
        // Optionally render the game world dimmed underneath menus
        // SDL_SetTextureColorMod(gameWorldTexture, 100, 100, 100); // Example dimming
        // SDL_RenderCopy(renderer, gameWorldTexture, ...);
        // SDL_SetTextureColorMod(gameWorldTexture, 255, 255, 255); // Reset tint

         // Draw semi-transparent overlay regardless of buff selection or simple pause
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Semi-transparent black
         SDL_Rect fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
         SDL_RenderFillRect(Game::renderer, &fullscreen);
         SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);


        // Render Buff Selection UI if active (drawn ON TOP of overlay)
        if (isInBuffSelection && ui) {
            ui->renderBuffSelectionUI(currentBuffOptions);
        }
        // Render General Pause Overlay (if paused BUT not in buff selection)
        else if (isPaused && ui) { // This 'else if' ensures it doesn't draw over buff selection
            // Draw "PAUSED" text
            SDL_Color white = {255, 255, 255, 255};
            TTF_Font* fontToUse = ui->getLargeFont() ? ui->getLargeFont() : ui->getFont();
            if (fontToUse) {
                 int textW, textH;
                 // Use renderTextToTexture to check size, then drawText
                 SDL_Texture* tempTex = ui->renderTextToTexture("PAUSED", white, fontToUse, textW, textH);
                 if(tempTex) {
                     ui->drawText("PAUSED", WINDOW_WIDTH / 2 - textW / 2, WINDOW_HEIGHT / 3 - textH / 2, white, fontToUse); // Position higher
                     SDL_DestroyTexture(tempTex);
                 } else {
                     std::cerr << "Failed to render PAUSED text texture." << std::endl;
                 }

                 // --- DRAW VOLUME CONTROLS ---
                 int volY = WINDOW_HEIGHT / 2; // Start position for volume text
                 int volX = WINDOW_WIDTH / 2 - 150; // Center alignment helper

                 // Music Volume Display
                 std::stringstream ssMusic;
                 ssMusic << "Music Volume: " << static_cast<int>(round(musicVolume * 100.0 / MIX_MAX_VOLUME)) << "% (Up/Down)";
                 ui->drawText(ssMusic.str(), volX, volY, white, ui->getFont()); // Use smaller font

                 // SFX Volume Display
                 volY += 30; // Move down for next line
                 std::stringstream ssSfx;
                 ssSfx << "SFX Volume:   " << static_cast<int>(round(sfxVolume * 100.0 / MIX_MAX_VOLUME)) << "% (Left/Right)";
                 ui->drawText(ssSfx.str(), volX, volY, white, ui->getFont()); // Use smaller font

                 // Add instructions for resuming
                 volY += 60;
                 ui->drawText("Press 'E' to Resume", volX + 50, volY, white, ui->getFont());

            }
        }
    }


    SDL_RenderPresent(Game::renderer);
} // End render
void Game::generateBuffOptions() {
    currentBuffOptions.clear();

    // Define using BuffType enum
    std::vector<BuffInfo> allPossibleBuffs = {
        // Spell Buffs
        {"Spell Dmg+", "+ Spell Damage", BuffType::SPELL_DAMAGE, static_cast<float>(BUFF_DAMAGE_AMOUNT)},
        {"Spell CDR", "- Spell Cooldown", BuffType::SPELL_COOLDOWN, static_cast<float>(BUFF_COOLDOWN_AMOUNT)},
        {"Spell Count+", "+ Spell Proj. Count", BuffType::SPELL_PROJ_COUNT, static_cast<float>(BUFF_PROJ_COUNT_AMOUNT)},
        {"Spell Pierce+", "+ Spell Pierce", BuffType::SPELL_PIERCE, static_cast<float>(BUFF_PIERCE_AMOUNT)},
        // Weapon Buffs
        {"Wpn Dmg+", "+ Weapon Damage", BuffType::WEAPON_DAMAGE, static_cast<float>(BUFF_DAMAGE_AMOUNT)},
        {"Wpn FireRate+", "+ Weapon Fire Rate", BuffType::WEAPON_FIRE_RATE, static_cast<float>(BUFF_FIRE_RATE_AMOUNT)},
        {"Wpn Count+", "+ Weapon Proj. Count", BuffType::WEAPON_PROJ_COUNT, static_cast<float>(BUFF_PROJ_COUNT_AMOUNT)},
        {"Wpn Pierce+", "+ Weapon Pierce", BuffType::WEAPON_PIERCE, static_cast<float>(BUFF_PIERCE_AMOUNT)},
        {"Wpn Burst+", "+ Weapon Burst Count", BuffType::WEAPON_BURST_COUNT, static_cast<float>(BUFF_BURST_COUNT_AMOUNT)},
        //Player Buffs
        {"Heal", "Heal 10% Max HP", BuffType::PLAYER_HEAL, 10.0f}
    };

    int numPossible = allPossibleBuffs.size();
    if (numPossible == 0) return;

    // Simple random index selection (ensure uniqueness) - unchanged
    std::vector<int> chosenIndices;
    while(currentBuffOptions.size() < 4 && chosenIndices.size() < static_cast<size_t>(numPossible)) { // Use size_t for comparison
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
         if (chosenIndices.size() >= static_cast<size_t>(numPossible)) break; // Use size_t
    }

    // std::cout << "Generated " << currentBuffOptions.size() << " buff options." << std::endl;
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
    if (isInBuffSelection && index >= 0 && index < static_cast<int>(currentBuffOptions.size())) {
         if (playerEntity) {
             // Access the selected buff using the index
             const BuffInfo& selectedBuff = currentBuffOptions[index]; // Use const&

            //  std::cout << "Applying buff: " << selectedBuff.name << std::endl;

             // Apply buff based on type using a switch
             int intAmount = static_cast<int>(selectedBuff.amount);
             float floatAmount = selectedBuff.amount;

            SpellComponent* spellComp = playerEntity->hasComponent<SpellComponent>() ? &playerEntity->getComponent<SpellComponent>() : nullptr;
            WeaponComponent* weaponComp = playerEntity->hasComponent<WeaponComponent>() ? &playerEntity->getComponent<WeaponComponent>() : nullptr;
            HealthComponent* healthComp = playerEntity->hasComponent<HealthComponent>() ? &playerEntity->getComponent<HealthComponent>() : nullptr; // Get HealthComponent

             // Use selectedBuff.type (This should now work)
             switch (selectedBuff.type) {
                 // Spell Cases
                 case BuffType::SPELL_DAMAGE:       if(spellComp) spellComp->increaseDamage(intAmount); break;
                 case BuffType::SPELL_COOLDOWN:     if(spellComp) spellComp->decreaseCooldown(intAmount); break;
                 case BuffType::SPELL_PROJ_COUNT:   if(spellComp) spellComp->increaseProjectileCount(intAmount); break;
                 case BuffType::SPELL_PIERCE:       if(spellComp) spellComp->increasePierce(intAmount); break;
                 // Weapon Cases
                 case BuffType::WEAPON_DAMAGE:      if(weaponComp) weaponComp->increaseDamage(intAmount); break;
                 case BuffType::WEAPON_FIRE_RATE:   if(weaponComp) weaponComp->decreaseFireRate(intAmount); break;
                 case BuffType::WEAPON_PROJ_COUNT:  if(weaponComp) weaponComp->increaseProjectileCount(intAmount); break;
                 case BuffType::WEAPON_PIERCE:      if(weaponComp) weaponComp->increasePierce(intAmount); break;
                 case BuffType::WEAPON_BURST_COUNT: if(weaponComp) weaponComp->increaseBurstCount(intAmount); break;
                    // Player Buffs
                case BuffType::PLAYER_HEAL:
                     if (healthComp) {
                         int maxHP = healthComp->getMaxHealth();
                         // Calculate 10% of max HP (using floatAmount which is 10.0f)
                         int healAmount = static_cast<int>(maxHP * (floatAmount / 100.0f));
                         healthComp->heal(healAmount); // Call the heal method
                        //  std::cout << "Player healed for " << healAmount << " HP." << std::endl;
                     } else {
                          std::cerr << "Error: Player has no HealthComponent to heal!" << std::endl;
                     }
                     break;
                case BuffType::INVALID: // Fallthrough intended
                 default:
                     std::cerr << "Warning: Invalid or unknown BuffType selected!" << std::endl;
                     break;
             }

             exitBuffSelection(); // Exit selection state after applying
         }
    }
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