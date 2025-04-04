#pragma once

#include "Components.h"
#include <SDL.h>
#include "../TextureManager.h"
#include "Animation.h"
#include <map>
#include "../AssetManager.h"
#include "../game.h" // Include game.h for Game::instance
#include <iostream> // For error logging
#include "ProjectileComponent.h"


class SpriteComponent : public Component {
private:
    TransformComponent *transform = nullptr; // Initialize to nullptr
    SDL_Texture *texture = nullptr;          // Initialize to nullptr
    SDL_Rect srcRect, destRect;

    bool animated = false;
    int frames = 0;
    

    bool initialized = false; // <<< ADD Initialization Flag

public:
    int speed = 100; //ms

    bool isHit = false;
    Uint32 hitTime = 0;
    Uint32 hitDuration = 150; // Flash duration in milliseconds
    SDL_Color tint = {255, 255, 255, 255}; // Add this line to declare the tint variable
    // Add this getter method if it doesn't exist yet
    SDL_Texture* getTexture() {
        return texture;
    }
    double angle = 0.0; // Angle in degrees for rotation
    float rotationSpeed = 180.0f; // Degrees per second (adjust as needed)

    int animIndex = 0;

    std::map<const char*, Animation> animations;

    SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;


    SpriteComponent() = default;
    SpriteComponent(std::string id){
        setTex(id);
    }
    SpriteComponent(std::string id, bool isAnimated){
        animated = isAnimated;
        Animation idle = Animation(0, 3, 200);
        Animation walk = Animation(1, 6, 200);

        animations.emplace("Idle", idle);
        animations.emplace("Walk", walk);

        Play("Idle");

        setTex(id);
    }

    ~SpriteComponent() {
        // Texture is managed by AssetManager, no need to destroy here generally
    }
    void setTex(std::string id){ {
        // Use Game::instance to access the non-static assets pointer
        // Add null check for gameInstance and assets
        if (Game::instance && Game::instance->assets) {
             texture = Game::instance->assets->GetTexture(id);
        } else {
            std::cerr << "Error in SpriteComponent::setTex: Game::instance or Game::instance->assets is null!" << std::endl;
            texture = nullptr; // Ensure texture is null if assets couldn't be accessed
        }
    }}

    void init() override {
        initialized = false; // Reset flag at start of init
        // Ensure the entity exists before proceeding
        if (!entity) {
            std::cerr << "Error in SpriteComponent::init: Entity is null!" << std::endl;
            return;
        }

        // Check if the entity has the required TransformComponent
        if (!entity->hasComponent<TransformComponent>()) {
             std::cerr << "Error in SpriteComponent::init: Entity missing TransformComponent!" << std::endl;
             return; // Cannot initialize without TransformComponent
        }

        // Get the TransformComponent
        transform = &entity->getComponent<TransformComponent>();

        // --- ADD NULL CHECK for transform pointer ---
        if (!transform) {
            std::cerr << "Error in SpriteComponent::init: Failed to get TransformComponent pointer (is null)!" << std::endl;
             return;
        }
        // --- End NULL CHECK ---


        // Now it's safe to access transform members
        srcRect.x = srcRect.y = 0;
        srcRect.w = transform->width;
        srcRect.h = transform->height;

        // If all checks passed and setup complete:
        initialized = true; // <<< SET Flag to true only on success
    }

    void update() override {
        if (!initialized) return; // <<< CHECK Flag at start of update
        // Keep internal null checks too
        if (!transform) return;

        if(animated){
            srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
        }
        // Ensure animIndex is valid and transform height is non-zero before division/multiplication
        if (transform->height > 0) {
            srcRect.y = animIndex * transform->height;
        } else {
            srcRect.y = 0; // Default if height is invalid
        }

        destRect.x = static_cast<int>(transform->position.x) - Game::camera.x;
        destRect.y = static_cast<int>(transform->position.y) - Game::camera.y;
        destRect.w = transform->width * transform->scale;
        destRect.h = transform->height * transform->scale;

// --- Projectile Spinning Logic ---
        // Determine if this projectile should spin
        bool shouldSpin = false;
        if (entity && entity->hasComponent<ProjectileComponent>()) {
            shouldSpin = entity->getComponent<ProjectileComponent>().isSpinning;
            // --- DEBUG LOG ---
            // bool isBossProj = false;
            // if (Game::instance && Game::instance->assets && texture == Game::instance->assets->GetTexture("boss_projectile")) { // Added null checks
            //     isBossProj = true;
            //     std::cout << "[DEBUG] SpriteComponent::update: Boss Projectile found. isSpinning flag = " << (shouldSpin ? "true" : "false") << std::endl;
            // }
            // --- END DEBUG LOG ---
        }

        // *** ADDED the missing 'if' condition here ***
        if (shouldSpin) {
            // Calculate the angle update only if it should spin
            const float timeStep = 1.0f / 60.0f; // Assuming 60 FPS target
            angle += rotationSpeed * timeStep;
            if (angle >= 360.0) angle -= 360.0;

            // --- DEBUG LOG ---
            // if (isBossProj) { // isBossProj needs to be declared in the debug block above if you uncomment this
            //     std::cout << "[DEBUG] SpriteComponent::update: Boss Projectile spinning. New Angle: " << angle << std::endl;
            // }
            // --- END DEBUG LOG ---

        } else {
            // Reset angle to 0 if it shouldn't be spinning
            // (Only assign if it's not already 0 to avoid redundant work)
            if (angle != 0.0) {
                angle = 0.0;
                // --- DEBUG LOG ---
                // if (isBossProj) { // isBossProj needs to be declared in the debug block above if you uncomment this
                //    std::cout << "[DEBUG] SpriteComponent::update: Boss Projectile stopped spinning. Angle reset." << std::endl;
                // }
                // --- END DEBUG LOG ---
            }
        }
        // --- End Projectile Spinning Logic ---
    }

    void draw() override {
         if (!initialized) return; // <<< CHECK Flag at start of draw (optional but safe)

         if (!texture) return;
         if (!transform) return;

        // Calculate the current tint color
        SDL_Color currentTint = tint;

        // If hit, apply red tint for this specific entity
        if (isHit) {
            if (SDL_GetTicks() > hitTime + hitDuration) {
                isHit = false;
                currentTint = {255, 255, 255, 255}; // Reset to normal color
            } else {
                currentTint = {255, 100, 100, 255}; // Red tint
            }
        }

        // --- DEBUG LOG ---
    bool isBossProj = false;
    if (entity && entity->hasComponent<ProjectileComponent>() && texture == Game::instance->assets->GetTexture("boss_projectile")){
        isBossProj = true;
        // Log only for the boss projectile, and maybe less frequently (e.g., every 60 frames)
        // static int frameCount = 0;
        // if (frameCount % 60 == 0) {
        //     std::cout << "[DEBUG] SpriteComponent::draw: Drawing Boss Projectile. Angle passed: " << angle << std::endl;
        // }
        // frameCount++;
    }
    // --- END DEBUG LOG ---


        SDL_SetTextureColorMod(texture, currentTint.r, currentTint.g, currentTint.b);
        TextureManager::Draw(texture, srcRect, destRect, angle, spriteFlip); // Pass the angle

        SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset
        
    }

    void Play(const char* animName){
        // Check if the animation exists before trying to access it
        if (animations.count(animName)) {
            const Animation& anim = animations[animName]; // Get reference
            // Check if the requested animation is already playing with the same parameters
            // This prevents resetting the animation cycle unnecessarily every frame
            if (this->animIndex != anim.index || this->frames != anim.frames || this->speed != anim.speed) {
                 this->frames = anim.frames;
                 this->animIndex = anim.index; // Sets the ROW
                 this->speed = anim.speed;
                 // std::cout << "Playing animation: " << animName << " (Index: " << animIndex << ", Frames: " << frames << ", Speed: " << speed << ")" << std::endl; // Debug log
            }
        } else {
            std::cerr << "Warning: Animation '" << animName << "' not found in SpriteComponent!" << std::endl;
            // Optionally default to a known animation like "Idle" or "Walk" if available
            // if (animations.count("Walk")) {
            //     const Animation& defaultAnim = animations["Walk"];
            //     this->frames = defaultAnim.frames;
            //     this->animIndex = defaultAnim.index;
            //     this->speed = defaultAnim.speed;
            // } else {
                 this->frames = 1; // Fallback to single frame
                 this->animIndex = 0;
                 this->speed = 10000; // Very slow speed
            // }
        }
    }

};