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
#include <ctime> // For std::time

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

    // Refresh manager before adding entities for this game instance
    manager.refresh(); // Clean up entities from previous game if manager persists

    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");

    // Create and setup player entity as member
    playerEntity = &manager.addEntity(); // Create the entity for this Game instance
    playerEntity->addComponent<TransformComponent>(400.0f, 320.0f, CHAR_W, CHAR_H, 2);
    playerEntity->addComponent<SpriteComponent>("player", true);
    playerEntity->addComponent<KeyboardController>();
    playerEntity->addComponent<ColliderComponent>("player", 32 , 37);
    playerEntity->addComponent<HealthComponent>(99999999, 99999999); // High health for testing?
    // Create a pistol (small bullets)

    playerEntity->addComponent<WeaponComponent>(
        "pistol",     // tag
        25,           // damage
        500,          // fireRate (time between bursts)
        5.0f,         // projectileSpeed
        500,          // projectileRange
        0.1f,         // spreadAngle (small spread for burst accuracy?)
        1,            // projectilesPerShot (1 projectile per shot in the burst)
        16,           // projectileSize
        "projectile", // projectileTexture
        1,            // projectilePierce
        3,            // shotsPerBurst (Fires 3 shots per trigger pull)
        75            // burstDelay (75ms between shots in the burst)
    );
    playerEntity->addGroup(groupPlayers);

    playerEntity->addComponent<SpellComponent>(
        "spiral_bolts",       // tag
        5,                    // damage
        100,                  // cooldown (ms) - Maybe faster cooldown for visible spiral?
        3.0f,                 // projectileSpeed
        0,                    // projectilesPerCast
        12,                   // projectileSize
        "projectile",         // projectileTexture
        3000,                 // duration (ms)
        SpellTrajectory::SPIRAL, // trajectoryMode
        8.0f,
        2                  // spiralGrowthRate (pixels per radian, adjust as needed)
    );
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
    // Process the current event polled in main.cpp
  
    switch (event.type) {
    case SDL_QUIT:
        isRunning = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_e) { // Toggle pause

            togglePause();
            
        }
        break;
    case SDL_MOUSEMOTION:
        mouseX = event.motion.x + camera.x; // Add camera offset
        mouseY = event.motion.y + camera.y; // Add camera offset
        break;
    default:
        break;
    }
}

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

    std::cout << "Spawning " << selectedEnemy.tag << " at: (" 
              << spawnPosition.x << ", " << spawnPosition.y << ")" << std::endl;

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

                        std::cout << "Projectile hit NEW enemy!" << std::endl;
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
                                     std::cout << "Enemy defeated! Total: " << playerManager->getEnemiesDefeated()
                                               << ", Level: " << playerManager->getLevel()
                                               << ", Exp: " << playerManager->getExperience()
                                               << "/" << playerManager->getExperienceToNextLevel() << std::endl;
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

void Game::render(){
    if (!renderer) return; // Safety check

    SDL_RenderClear(renderer);

    // Render active entities by group
    for(auto& t : tiles) if(t->isActive()) t->draw();
    for(auto& c : colliders) if(c->isActive()) c->draw(); // Draw terrain colliders if needed for debug
    for(auto& p : projectiles) if(p->isActive()) p->draw();
    for(auto& e : enemies) if(e->isActive()) e->draw();
    // Render player last (or based on desired layering)
    if(playerEntity && playerEntity->isActive()) playerEntity->draw();


    // Render Health Bars
    if (playerEntity && playerEntity->isActive()) {
        Vector2D playerPos = playerEntity->getComponent<ColliderComponent>().position; // Use collider position
        renderHealthBar(*playerEntity, playerPos);
    }
    for(auto& e : enemies) {
        if (e->isActive() && e->hasComponent<ColliderComponent>() && e->hasComponent<HealthComponent>()) {
            Vector2D enemyPos = e->getComponent<ColliderComponent>().position;
            renderHealthBar(*e, enemyPos);
        }
    }

    // Render UI (Top Layer)
    if (ui && playerManager) {
        ui->renderUI(playerManager);
    }

    // Render Pause Overlay if paused
    if (isPaused) {

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Semi-transparent black
        SDL_Rect fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // Add "PAUSED" text (ensure font loading/handling is robust)
        TTF_Font* pauseFont = TTF_OpenFont("assets/font.ttf", 28); // Consider loading fonts once
        if (pauseFont) {
             SDL_Color white = {255, 255, 255, 255};
             SDL_Surface* surf = TTF_RenderText_Blended(pauseFont, "PAUSED", white);
             if (surf) {
                 SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                 if (tex) {
                     SDL_Rect dstRect = { WINDOW_WIDTH / 2 - surf->w / 2, WINDOW_HEIGHT / 2 - surf->h / 2, surf->w, surf->h };
                     SDL_RenderCopy(renderer, tex, NULL, &dstRect);
                     SDL_DestroyTexture(tex);
                 }
                 SDL_FreeSurface(surf);
             }
             TTF_CloseFont(pauseFont); // Close font after use
        }
    }

    SDL_RenderPresent(renderer);
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


// Static method - potentially unnecessary if main handles renderer lifetime
void Game::cleanupRenderer() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
        std::cout << "Static renderer cleanup called." << std::endl;
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