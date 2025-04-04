#pragma once

// --- Includes ---
#include "Components.h" // Includes ECS.h, TransformComponent.h indirectly
#include <SDL.h>
#include <map>
#include <string>
#include <iostream> // For std::cerr
#include "../TextureManager.h"
#include "../AssetManager.h"
#include "Animation.h"
#include "../game.h" // For Game::instance, Game::camera
#include "ProjectileComponent.h" // For checking spin flag

// --- Forward Declarations ---
class TransformComponent; // Already included via Components.h, but explicit is fine

// --- Class Definition ---

class SpriteComponent : public Component {
private:
    // --- Private Members ---
    TransformComponent *transform = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_Rect srcRect = {0, 0, 0, 0};
    SDL_Rect destRect = {0, 0, 0, 0};

    bool animated = false;
    int frames = 0;
    bool initialized = false;

public:
    // --- Public Members ---

    // Animation & Appearance
    int speed = 100; // Animation speed (ms per frame)
    int animIndex = 0; // Current animation row index
    std::map<const char*, Animation> animations;
    SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;
    SDL_Color tint = {255, 255, 255, 255}; // Color modulation

    // Rotation
    double angle = 0.0;         // Rotation angle in degrees
    float rotationSpeed = 180.0f; // Degrees per second

    // Hit Effect
    bool isHit = false;
    Uint32 hitTime = 0;
    Uint32 hitDuration = 150; // ms

    // --- Constructors ---
    SpriteComponent() = default;

    SpriteComponent(std::string id) {
        setTex(id); // Call setTex from constructor
    }

    SpriteComponent(std::string id, bool isAnimated) {
        animated = isAnimated;
        // Define default animations if needed
        Animation idle = Animation(0, 3, 200); // Example: row 0, 3 frames, 200ms/frame
        Animation walk = Animation(1, 6, 200); // Example: row 1, 6 frames, 200ms/frame
        animations.emplace("Idle", idle);
        animations.emplace("Walk", walk);

        Play("Idle"); // Start with Idle animation
        setTex(id);   // Load the texture
    }

    // --- Destructor ---
    ~SpriteComponent() override {
        // Texture pointers ('texture') are managed by AssetManager, so no SDL_DestroyTexture here.
    }

    // --- Public Methods ---

    // Component Lifecycle Overrides
    void init() override {
        initialized = false; // Reset flag
        if (!entity) {
            std::cerr << "Error in SpriteComponent::init: Entity is null!" << std::endl;
            return;
        }
        if (!entity->hasComponent<TransformComponent>()) {
             std::cerr << "Error in SpriteComponent::init: Entity missing TransformComponent!" << std::endl;
             return;
        }
        transform = &entity->getComponent<TransformComponent>();
        if (!transform) {
            std::cerr << "Error in SpriteComponent::init: Failed to get TransformComponent pointer!" << std::endl;
             return;
        }

        // Initialize source rectangle based on transform's initial size
        srcRect.x = srcRect.y = 0;
        srcRect.w = transform->width;
        srcRect.h = transform->height;

        initialized = true; // Set flag on successful initialization
    }

    void update() override {
        if (!initialized || !transform) return; // Check initialization and required pointers

        // Update animation frame if animated
        if (animated && frames > 0 && speed > 0) {
            srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
        } else if (!animated) {
             srcRect.x = 0; // Ensure static sprites use the first frame
        }

        // Set animation row (vertical position in spritesheet)
        // Ensure height is positive to avoid division by zero or negative index
        srcRect.y = (transform->height > 0) ? (animIndex * transform->height) : 0;

        // Update destination rectangle for rendering
        destRect.x = static_cast<int>(transform->position.x - Game::camera.x);
        destRect.y = static_cast<int>(transform->position.y - Game::camera.y);
        destRect.w = transform->width * transform->scale;
        destRect.h = transform->height * transform->scale;

        // Handle projectile spinning logic
        bool shouldSpin = false;
        if (entity->hasComponent<ProjectileComponent>()) { // Null check for entity done by initialized check
            shouldSpin = entity->getComponent<ProjectileComponent>().isSpinning;
        }

        if (shouldSpin) {
            const float timeStep = 1.0f / 60.0f; // Assuming ~60 FPS update rate
            angle += rotationSpeed * timeStep;
            if (angle >= 360.0) angle -= 360.0;
            if (angle < 0.0) angle += 360.0; // Keep angle positive
        } else {
            angle = 0.0; // Ensure non-spinning sprites have angle 0
        }
    }

    void draw() override {
        if (!initialized || !texture || !transform) return; // Check required members

        // Determine tint (normal or hit effect)
        SDL_Color currentTint = tint; // Start with default tint
        if (isHit) {
            if (SDL_GetTicks() > hitTime + hitDuration) {
                isHit = false; // Hit duration expired
                // currentTint remains default (or could be explicitly set to white)
            } else {
                currentTint = {255, 100, 100, 255}; // Apply red tint
            }
        }

        // Apply tint and draw
        SDL_SetTextureColorMod(texture, currentTint.r, currentTint.g, currentTint.b);
        TextureManager::Draw(texture, srcRect, destRect, angle, spriteFlip); // Use the Draw function handling rotation
        SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset tint for other draws
    }

    // Sets the texture from AssetManager using ID
    void setTex(std::string id) {
        if (Game::instance && Game::instance->assets) {
             texture = Game::instance->assets->GetTexture(id);
             if (!texture) {
                 std::cerr << "Warning in SpriteComponent::setTex: Texture ID '" << id << "' not found in AssetManager!" << std::endl;
             }
        } else {
            std::cerr << "Error in SpriteComponent::setTex: Game::instance or Game::instance->assets is null!" << std::endl;
            texture = nullptr;
        }
    }

    // Gets the raw SDL_Texture pointer
    SDL_Texture* getTexture() {
        return texture;
    }

    // Sets the current animation to play
    void Play(const char* animName) {
        auto it = animations.find(animName);
        if (it != animations.end()) {
            const Animation& anim = it->second; // Use found iterator
            // Only update if different to prevent resetting animation cycle
            if (this->animIndex != anim.index || this->frames != anim.frames || this->speed != anim.speed) {
                 this->frames = anim.frames;
                 this->animIndex = anim.index;
                 this->speed = anim.speed > 0 ? anim.speed : 100; // Ensure speed is positive
            }
        } else {
            std::cerr << "Warning: Animation '" << animName << "' not found in SpriteComponent!" << std::endl;
            // Fallback to a non-animated state or a default animation if desired
            this->frames = 1;
            this->animIndex = 0;
            this->speed = 100000; // Effectively static
        }
    }

}; // End SpriteComponent class