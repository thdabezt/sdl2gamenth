#include "AssetManager.h"
#include "ECS/Components.h"


AssetManager::AssetManager(Manager* Man) : manager(Man){

}

AssetManager::~AssetManager(){

}
void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int range, int damage, int size, std::string id, int pierce) { // Add pierce parameter
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, size, size, 1);
    projectile.addComponent<SpriteComponent>(id);
    projectile.addComponent<ProjectileComponent>(range, damage, vel, pierce); // Pass pierce here
    projectile.addComponent<ColliderComponent>("projectile"); // Consider collider size relative to 'size'
    projectile.addGroup(Game::groupProjectiles);
}

void AssetManager::AddTexture(std::string id, const char* path){
    textures.emplace(id, TextureManager::LoadTexture(path));
}

SDL_Texture* AssetManager::GetTexture(std::string id){
    return textures[id];
}