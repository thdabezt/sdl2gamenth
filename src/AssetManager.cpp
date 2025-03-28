#include "AssetManager.h"
#include "ECS/Components.h"


AssetManager::AssetManager(Manager* Man) : manager(Man){

}

AssetManager::~AssetManager(){

}
void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int range, int damage, std::string id){
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, 32, 32, 1);
    projectile.addComponent<SpriteComponent>(id);
    projectile.addComponent<ProjectileComponent>(range, damage, vel);
    projectile.addComponent<ColliderComponent>("projectile");
    projectile.addGroup(Game::groupProjectiles);
}

void AssetManager::AddTexture(std::string id, const char* path){
    textures.emplace(id, TextureManager::LoadTexture(path));
}

SDL_Texture* AssetManager::GetTexture(std::string id){
    return textures[id];
}