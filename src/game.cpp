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

Map *map;
SDL_Event Game::event;

SDL_Renderer *Game::renderer = nullptr;

Manager manager;

SDL_Rect Game::camera = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

AssetManager *Game::assets = new AssetManager(&manager);
bool Game::isRunning = false;

auto& player(manager.addEntity());


Game::Game(){}

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
    assets->AddTexture("terrain", MAP);
    assets->AddTexture("player", playerSprites);
    assets->AddTexture("projectile", "sprites/projectile/gunshot.png");

    player.addComponent<TransformComponent>(400.0f, 320.0f, CHAR_W, CHAR_H, 2);
    player.addComponent<SpriteComponent>("player", true);
    player.addComponent<KeyboardController>();
    player.addComponent<ColliderComponent>("player");
    player.addGroup(groupPlayers);

    
    map = new Map("terrain", 1, 32);
    map->LoadMap("assets/map.map", MAP_WIDTH, MAP_HEIGHT, 10);
    
    assets->CreateProjectile(Vector2D(600, 600), Vector2D(2,0), 200, 2, "projectile");
 
}

auto& tiles(manager.getGroup(Game::groupMap));
auto& players(manager.getGroup(Game::groupPlayers));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& projectiles(manager.getGroup(Game::groupProjectiles));

void Game::handleEvents(){
    
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        isRunning = false;
        break;
    
    default:
        break;
    }
}

void Game::update(){

    SDL_Rect playerCol = player.getComponent<ColliderComponent>().collider;
    Vector2D playerPos = player.getComponent<TransformComponent>().position;

    manager.refresh();
    manager.update();

    for(auto& c : colliders){
        SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
        if(Collision::AABB(playerCol, cCol)){
            player.getComponent<TransformComponent>().position = playerPos;
        }
    }
    for(auto& p : projectiles){
        if(Collision::AABB(playerCol, p->getComponent<ColliderComponent>().collider)){
            // p->destroy();
            std::cout << "Hit player" << std::endl;
        }
    }
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastShotTime + 1000) { // 1000 ms = 1 second
    Vector2D projectilePosition = player.getComponent<TransformComponent>().position;
    Vector2D projectileVelocity = Vector2D(-5, 0); // Move left at a reasonable speed
    assets->CreateProjectile(projectilePosition, projectileVelocity, 500, 5, "projectile"); // Range = 500, Speed = 5
    lastShotTime = currentTime;
    std::cout << "Projectile shot from player!" << std::endl;
}

    camera.x = player.getComponent<TransformComponent>().position.x - (WINDOW_WIDTH / 2);
    camera.y = player.getComponent<TransformComponent>().position.y - (WINDOW_HEIGHT / 2);

    if(camera.x < 0){
        camera.x = 0;
    }
    if(camera.y < 0){
        camera.y = 0;
    }
    if(camera.x > camera.w){
        camera.x = camera.w;
    }
    if(camera.y > camera.h){
        camera.y = camera.h;
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
    

    SDL_RenderPresent(renderer);
}

void Game::clean(){
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
    SDL_Quit();
    std::cout<< "Game quitted"<< std::endl;

}
