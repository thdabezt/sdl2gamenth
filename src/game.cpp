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
Manager manager;

SDL_Rect Game::camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

AssetManager *Game::assets = new AssetManager(&manager);
bool Game::isRunning = false;
bool godmode = false;
auto& player(manager.addEntity());

Game::Game() {
    instance = this;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Game::~Game(){}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen){
    int flags = 0;
    if(fullscreen){
        flags = SDL_WINDOW_FULLSCREEN;
    }
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0){
        std::cout<< "Starting" <<std::endl;

        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if(window){
            std::cout<<"Ran Game - Created window"<<std::endl;

        }
        renderer = SDL_CreateRenderer(window,-1,0);
        if(renderer){
            std::cout<< "Renderer running" <<std::endl;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
            isRunning = false;
            return;
        }
        isRunning = true;
    }
    else {
        isRunning = false;
    }
    manager.refresh();
    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");

    player.addComponent<TransformComponent>(400.0f, 320.0f, CHAR_W, CHAR_H, 2);
    player.addComponent<SpriteComponent>("player", true);
    player.addComponent<KeyboardController>();
    player.addComponent<ColliderComponent>("player", 32 , 37);
    player.addComponent<WeaponComponent>(25, 200, 5.0f, 500, 0.2f, 3, "projectile");
    player.addGroup(groupPlayers);


    
    
    map = new Map("terrain", 1, 32);
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10);

 
}

auto& tiles(manager.getGroup(Game::groupMap));
auto& players(manager.getGroup(Game::groupPlayers));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& projectiles(manager.getGroup(Game::groupProjectiles));
auto& enemies(manager.getGroup(Game::groupEnemies));
void Game::handleEvents(){
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        isRunning = false;
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
    std::cout << "Number of spawn points: " << spawnPoints.size() << std::endl;

    if (spawnPoints.empty()) {
        std::cerr << "No spawn points available!" << std::endl;
        return;
    }

    // Choose a random spawn point
    int randomIndex = std::rand() % spawnPoints.size();
    Vector2D spawnPosition = spawnPoints[randomIndex];

    // Print the spawn location
    std::cout << "Spawning enemy at: (" << spawnPosition.x << ", " << spawnPosition.y << ")" << std::endl;

    // Create the enemy entity
    auto& enemy = manager.addEntity();
    assets->AddTexture("enemy", enemy1Sprites);
    enemy.addComponent<TransformComponent>(spawnPosition.x, spawnPosition.y, E1_W, E1_H, 2);
    enemy.addComponent<SpriteComponent>("enemy", true);
    enemy.addComponent<ColliderComponent>("enemy", E1_W, E1_H);
    enemy.addComponent<HealthComponent>(50, 50);
    // Pass the player's TransformComponent and ColliderComponent to the EnemyAIComponent
    enemy.addComponent<EnemyAIComponent>(
        5000, // Detection range
        1.0f, // Movement speed
        &player.getComponent<ColliderComponent>().position// Player's position
    );

    enemy.addGroup(groupEnemies); // Add to the appropriate group
}

void Game::update(){
     // Store position BEFORE any movement this frame
     Vector2D playerPosStart = player.getComponent<TransformComponent>().position;
     // Get player collider component once
     ColliderComponent& playerCollider = player.getComponent<ColliderComponent>();
     Vector2D realposition;
     realposition.x = player.getComponent<ColliderComponent>().collider.x;
     realposition.y = player.getComponent<ColliderComponent>().collider.y;
     
 
     Uint32 currentTime = SDL_GetTicks();
     manager.refresh();
     manager.update(); 

     SDL_Rect playerCol = playerCollider.collider;
     
     TransformComponent& playerTransform = player.getComponent<TransformComponent>();
 
     for (auto& c : colliders) {
         ColliderComponent& obstacleCollider = c->getComponent<ColliderComponent>();
         SDL_Rect cCol = obstacleCollider.collider;
         if (obstacleCollider.tag != "enemy" && Collision::AABB(playerCol, cCol)) {
             float overlapX = std::min(playerCol.x + playerCol.w, cCol.x + cCol.w) - std::max(playerCol.x, cCol.x);
             float overlapY = std::min(playerCol.y + playerCol.h, cCol.y + cCol.h) - std::max(playerCol.y, cCol.y);
             SDL_Rect prevPlayerCol = playerCol; 
             prevPlayerCol.x += static_cast<int>(playerPosStart.x - playerTransform.position.x);
             prevPlayerCol.y += static_cast<int>(playerPosStart.y - playerTransform.position.y);
             bool wasClearX = (prevPlayerCol.x + prevPlayerCol.w <= cCol.x) || (prevPlayerCol.x >= cCol.x + cCol.w);
             bool wasClearY = (prevPlayerCol.y + prevPlayerCol.h <= cCol.y) || (prevPlayerCol.y >= cCol.y + cCol.h);
             if (overlapX < overlapY && wasClearX) { 
                  if (playerTransform.position.x > playerPosStart.x) { 
                     playerTransform.position.x -= overlapX; 
                  } else if (playerTransform.position.x < playerPosStart.x) { 
                     playerTransform.position.x += overlapX; 
                  } else { 
                      if ((playerCol.x + playerCol.w / 2.0f) < (cCol.x + cCol.w / 2.0f))
                         playerTransform.position.x -= overlapX;
                      else
                         playerTransform.position.x += overlapX;
                  }
             } else if (overlapY <= overlapX && wasClearY) { 
                  if (playerTransform.position.y > playerPosStart.y) { 
                     playerTransform.position.y -= overlapY; 
                  } else if (playerTransform.position.y < playerPosStart.y) { 
                     playerTransform.position.y += overlapY; 
                  } else { 
                      if ((playerCol.y + playerCol.h / 2.0f) < (cCol.y + cCol.h / 2.0f))
                         playerTransform.position.y -= overlapY;
                      else
                         playerTransform.position.y += overlapY;
                  }
             } else {
                  if(overlapX < overlapY) playerTransform.position.x += (playerTransform.position.x > playerPosStart.x ? -overlapX : overlapX);
                  else playerTransform.position.y += (playerTransform.position.y > playerPosStart.y ? -overlapY : overlapY);
             }
             playerCollider.update();
             playerCol = playerCollider.collider;
         } 
     } 
    // Handle projectile collisions and enemy knockback
    for (auto& p : projectiles) {
        SDL_Rect projectileCollider = p->getComponent<ColliderComponent>().collider;
        bool projectileHit = false;
    
        // Check if there are any enemies before iterating
        if (!enemies.empty()) {
            // Iterate through enemies to check for collisions
            for (auto& e : enemies) {
                // Make sure the enemy is active and has the required components
                if (e->isActive() && e->hasComponent<ColliderComponent>()) {
                    SDL_Rect enemyCollider = e->getComponent<ColliderComponent>().collider;
    
                    if (Collision::AABB(enemyCollider, projectileCollider)) {
                        // Apply damage to the enemy
                        std::cout << "Projectile hit enemy!" << std::endl;
                        
                        if (e->hasComponent<TransformComponent>()) {
                            // Get projectile direction vector for knockback
                            Vector2D knockbackDir = p->getComponent<TransformComponent>().velocity;
                            knockbackDir = knockbackDir.Normalize(); // Get just the direction
                            
                            // Store original position to check for collisions after knockback
                            Vector2D originalPos = e->getComponent<TransformComponent>().position;
                            
                            // Apply knockback based on projectile direction
                            float knockbackForce = 15.0f; // Adjust for desired strength
                            e->getComponent<TransformComponent>().position.x += knockbackDir.x * knockbackForce;
                            e->getComponent<TransformComponent>().position.y += knockbackDir.y * knockbackForce;
                            
                            // Make sure enemy collider is updated after position change
                            if (e->hasComponent<ColliderComponent>()) {
                                e->getComponent<ColliderComponent>().update();
                                
                                // Check if knockback pushed enemy into a wall or terrain
                                bool collision = false;
                                for (auto& c : colliders) {
                                    if (Collision::AABB(e->getComponent<ColliderComponent>().collider, 
                                                      c->getComponent<ColliderComponent>().collider)) {
                                        collision = true;
                                        break;
                                    }
                                }
                                
                                // If knockback caused collision, revert to original position
                                if (collision) {
                                    e->getComponent<TransformComponent>().position = originalPos;
                                    e->getComponent<ColliderComponent>().update();
                                }
                            }
                            
                            // Optional: Add visual feedback for enemy hit
                            // In the projectile hit logic:
                            if (e->hasComponent<SpriteComponent>()) {
                                // Don't modify the texture directly, just set the hit flag
                                e->getComponent<SpriteComponent>().isHit = true;
                                e->getComponent<SpriteComponent>().hitTime = currentTime;
                            }
                        }
                        
                        // Inside the projectile collision detection code:
                        if (e->hasComponent<HealthComponent>()) {
                            // Use the projectile's damage value instead of a fixed value
                            int damage = p->getComponent<ProjectileComponent>().getDamage();
                            e->getComponent<HealthComponent>().takeDamage(damage);
                            projectileHit = true;
                            p->destroy();
                            break;
                        }
                    }
                }
            }
        }
    }
    
    


    // enemyspawn
    if (currentTime > lastEnemySpawnTime + 1000) { // 5000 ms = 5 seconds
        spawnEnemy();
        lastEnemySpawnTime = currentTime;
    }


    //camera

    camera.x = player.getComponent<TransformComponent>().position.x - (WINDOW_WIDTH / 2);
    camera.y = player.getComponent<TransformComponent>().position.y - (WINDOW_HEIGHT / 2);

    // Clamp the camera's position within the map boundaries
    if (camera.x < 0) {
        camera.x = 0;
    }
    if (camera.y < 0) {
        camera.y = 0;
    }
    if (camera.x > (MAP_WIDTH * 32) - camera.w) { // Assuming each tile is 32x32
        camera.x = (MAP_WIDTH * 32) - camera.w;
    }
    if (camera.y > (MAP_HEIGHT * 32) - camera.h) { // Assuming each tile is 32x32
        camera.y = (MAP_HEIGHT * 32) - camera.h;
    }

}

void Game::render(){
    SDL_RenderClear(renderer);
    for(auto& t : tiles){
        t->draw();
    }
    for(auto& c : colliders){
        c->draw();
    }
    for(auto& p : players){
        p->draw();
    }
    for(auto& p : projectiles){
        p->draw();
    }
    for(auto& e : manager.getGroup(groupEnemies)){
        e->draw();
    }
    
    // Get player position for health bar
    Vector2D realPosition;
    realPosition.x = player.getComponent<ColliderComponent>().collider.x;
    realPosition.y = player.getComponent<ColliderComponent>().collider.y;
    
    // Render player health bar
    renderHealthBar(player, realPosition);
    
    // Render enemy health bars
    for(auto& e : enemies) {
        if (e->isActive() && e->hasComponent<ColliderComponent>()) {
            Vector2D enemyPos;
            enemyPos.x = e->getComponent<ColliderComponent>().collider.x;
            enemyPos.y = e->getComponent<ColliderComponent>().collider.y;
            renderHealthBar(*e, enemyPos);
        }
    }
    SDL_RenderPresent(renderer);
}

void Game::clean(){
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
    SDL_Quit();
    std::cout<< "Game quitted"<< std::endl;

}


void Game::rezero(){
    player.getComponent<TransformComponent>().position.x = 400;
    player.getComponent<TransformComponent>().position.y = 320;
    godmode = true;
}


void Game::cleanupRenderer() {
    // Only clean the renderer when everything is done
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
}

void Game::cleanExceptRenderer() {
    // Clean everything except the renderer
    std::cout << "Cleaning game resources (keeping renderer)..." << std::endl;
    
    // Reset camera
    camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    
    // Stop managers first before destroying resources
    if (assets) {
        delete assets;
        assets = nullptr;
    }
    
    // Reset static variables
    assets = new AssetManager(&manager);
    
    // Clean up map
    if (map) {
        delete map;
        map = nullptr;
    }
    
    // Safe: Just close window, DON'T touch renderer
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    // Set running to false
    isRunning = false;
    
    std::cout << "Game resources cleaned up (renderer preserved)" << std::endl;
}
Entity& Game::getPlayer() {
    return player;
}
void Game::renderHealthBar(Entity& entity, Vector2D position) {
    if (!entity.hasComponent<HealthComponent>()) {
        std::cout << "Entity missing HealthComponent!" << std::endl;
        return;
    }
    
    const HealthComponent& health = entity.getComponent<HealthComponent>();
    
    // Calculate health bar dimensions and position
    int barWidth = 40;
    int barHeight = 5;
    int xPos = static_cast<int>(position.x) - camera.x; // Adjust for camera
    int yPos = static_cast<int>(position.y) - camera.y - 15; // Position above the entity
    
    // Make sure the bar is visible on screen
    if (xPos < -barWidth || xPos > WINDOW_WIDTH || 
        yPos < -barHeight || yPos > WINDOW_HEIGHT) {
        return; // Don't render if offscreen
    }
    
    // Draw background (black)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect bgRect = {xPos - 1, yPos - 1, barWidth + 2, barHeight + 2};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Calculate health percentage
    float healthPercent = static_cast<float>(health.getHealth()) / health.getMaxHealth();
    int currentBarWidth = static_cast<int>(barWidth * healthPercent);
    
    // Draw health bar (green to red based on health percentage)
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, barHeight};
    SDL_RenderFillRect(renderer, &healthRect);
}