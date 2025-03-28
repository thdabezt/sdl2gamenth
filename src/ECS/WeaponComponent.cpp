#include "Components.h"
#include "../AssetManager.h"

void WeaponComponent::init() {
    transform = &entity->getComponent<TransformComponent>();
    collider = &entity->getComponent<ColliderComponent>();
    lastShotTime = SDL_GetTicks(); // Initialize when component is created
}

void WeaponComponent::update() {
    // Check if the weapon can fire (cooldown elapsed)
    Uint32 currentTime = SDL_GetTicks();
    
    // Get current mouse position for aiming
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // Add camera offset to get world coordinates
    Game::mouseX = mouseX + Game::camera.x;
    Game::mouseY = mouseY + Game::camera.y;
    
    // Auto-fire when cooldown elapsed (no mouse click required)
    if (currentTime > lastShotTime + fireRate) {
        shoot();
        lastShotTime = currentTime;
    }
}

void WeaponComponent::shoot() {
    // Get projectile start position from collider
    Vector2D projectilePosition;
    projectilePosition.x = collider->collider.x;
    projectilePosition.y = collider->collider.y;
    
    // Calculate direction to mouse cursor
    Vector2D targetPosition(Game::mouseX, Game::mouseY);
    Vector2D direction = targetPosition - projectilePosition;
    
    // Normalize and set speed
    Vector2D baseVelocity = direction.Normalize() * projectileSpeed;
    
    // Debug output
    std::cout << "Auto-firing weapon toward: (" << Game::mouseX << ", " << Game::mouseY << ")" << std::endl;
    
    // Single projectile case
    if (projectilesPerShot == 1) {
        createProjectile(projectilePosition, baseVelocity);
        return;
    }
    
    // Multiple projectiles with spread
    float angleStep = 0;
    if (projectilesPerShot > 1) {
        angleStep = (spreadAngle * 2) / (projectilesPerShot - 1);
    }
    
    for (int i = 0; i < projectilesPerShot; i++) {
        float currentAngle = -spreadAngle + (i * angleStep);
        
        // Calculate rotated velocity
        Vector2D rotatedVelocity;
        rotatedVelocity.x = baseVelocity.x * cos(currentAngle) - baseVelocity.y * sin(currentAngle);
        rotatedVelocity.y = baseVelocity.x * sin(currentAngle) + baseVelocity.y * cos(currentAngle);
        rotatedVelocity = rotatedVelocity.Normalize() * projectileSpeed;
        
        // Create the projectile
        createProjectile(projectilePosition, rotatedVelocity);
    }
}

// Helper to create projectile - this is where we actually call AssetManager
void WeaponComponent::createProjectile(Vector2D position, Vector2D velocity) {
    // Access AssetManager from the cpp file to avoid circular dependency
    Game::assets->CreateProjectile(position, velocity, projectileRange, damage, projectileSize, projectileTexture);

}

void WeaponComponent::onEnemyDefeated() {
    // Increase damage by 1 each time an enemy is defeated
    damage += 1;
    
    // Decrease fire rate (make it faster) every 3 enemies
    static int enemiesDefeated = 0;
    enemiesDefeated++;
    
    if (enemiesDefeated % 3 == 0) {
        // Reduce fire rate by 5% (makes it shoot faster)
        fireRate = std::max(50, static_cast<int>(fireRate * 0.95f));
    }
    
    // Increase projectile count periodically
    if (enemiesDefeated % 10 == 0 && projectilesPerShot < 5) {
        projectilesPerShot += 1;
    }
    
    // Log the weapon upgrade
    std::cout << "Weapon upgraded! Damage: " << damage 
              << ", Fire Rate: " << fireRate 
              << ", Projectiles: " << projectilesPerShot << std::endl;
}