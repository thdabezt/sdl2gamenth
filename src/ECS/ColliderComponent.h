#pragma once

// --- Includes ---
#include <string>
#include <SDL.h>
#include <iostream>     // For std::cerr
#include "Components.h" // Includes ECS.h, TransformComponent.h indirectly
#include "../game.h"    // For Game::camera, Game::renderer, Game::instance->assets
#include "../TextureManager.h" // For TextureManager::Draw (in debug draw)
#include "../Vector2D.h" // For position member

// --- Forward Declarations ---


// --- Class Definition ---

// Represents an axis-aligned bounding box (AABB) collider for collision detection.
class ColliderComponent : public Component {
private:
    // --- Private Members ---
    bool initialized = false; // Initialization flag

public:
    // --- Public Members ---
    SDL_Rect collider;        // The SDL rectangle representing the collision area
    std::string tag;          // Tag for identifying collider type (e.g., "player", "terrain", "enemy")

    // For Debug Drawing (Optional)
    SDL_Texture* tex = nullptr; // Debug texture
    SDL_Rect srcR = {0, 0, 0, 0}; // Source rect for debug texture
    SDL_Rect destR = {0, 0, 0, 0}; // Destination rect for debug drawing

    // State & Pointers
    TransformComponent* transform = nullptr; // Pointer to entity's transform (if applicable)
    Vector2D position;                       // Stores the collider's top-left corner (world coordinates)
    int colliderWidth = 0;                   // Explicit width, overrides transform if set
    int colliderHeight = 0;                  // Explicit height, overrides transform if set

    // --- Constructors ---

    // Basic constructor with tag only (size determined by transform in init).
    ColliderComponent(std::string t) : tag(std::move(t)) {}

    // Constructor with tag and explicit dimensions (overrides transform size).
    ColliderComponent(std::string t, int cWidth, int cHeight)
     : tag(std::move(t)), colliderWidth(cWidth), colliderHeight(cHeight) {}

    // Constructor for static colliders (like terrain) with explicit position and size.
    // Sets position directly and doesn't rely on a TransformComponent.
    ColliderComponent(std::string t, int xpos, int ypos, int size)
        : tag(std::move(t)),
          position(static_cast<float>(xpos), static_cast<float>(ypos)),
          colliderWidth(size),
          colliderHeight(size)
    {
        collider.x = xpos;
        collider.y = ypos;
        collider.w = size;
        collider.h = size;
        // transform will remain nullptr for this constructor type
    }

    // --- Public Methods ---

    // Component Lifecycle Overrides
    void init() override {
        initialized = false; // Reset flag
        if (!entity) {
            std::cerr << "Error in ColliderComponent::init: Entity is null for tag '" << tag << "'!" << std::endl;
            return;
        }

        // Dynamic colliders (non-terrain) require a TransformComponent
        if (tag != "terrain") {
            if (!entity->hasComponent<TransformComponent>()) {
                 std::cerr << "Error in ColliderComponent::init: Entity with tag '" << tag << "' missing TransformComponent!" << std::endl;
                 return; // Cannot initialize without transform
            }
            transform = &entity->getComponent<TransformComponent>();
            if (!transform) {
                  std::cerr << "Error in ColliderComponent::init: Failed to get TransformComponent for tag '" << tag << "'!" << std::endl;
                  return;
            }

            // Use explicit width/height if provided, otherwise use transform's size
            if (colliderWidth == 0) { colliderWidth = static_cast<int>(transform->width * transform->scale); }
            if (colliderHeight == 0) { colliderHeight = static_cast<int>(transform->height * transform->scale); }

        } else {
             // Static terrain collider setup
             transform = nullptr; // Ensure transform is null
             // Load debug texture (optional)
             if (Game::instance && Game::instance->assets) {
                // tex = Game::instance->assets->GetTexture("border"); // Example debug texture
             }
             srcR = {0, 0, 32, 32}; // Example source rect for debug texture
        }

        // Set final collider dimensions
        collider.w = colliderWidth;
        collider.h = colliderHeight;

        initialized = true; // Set flag on successful initialization
    }

    void update() override {
        if (!initialized) return; // Check initialization status

        // Update collider position based on transform or stored position
        if (transform) {
             // Dynamic collider: Position based on TransformComponent, with potential offsets by tag
             float basePosX = transform->position.x;
             float basePosY = transform->position.y;

             // Apply specific offsets based on tag - consider a data-driven approach or separate components later
             if (tag == "player") {
                 collider.x = static_cast<int>(basePosX + 50); // Example offset
                 collider.y = static_cast<int>(basePosY + 70); // Example offset
             }
             // Grouping similar enemy types for offset application
             else if (tag == "zombie" || tag == "aligator1" || tag == "aligator2" ||
                      tag == "bear1" || tag == "bear2" || tag == "eliteskeleton_shield" ||
                      tag == "ina1" || tag == "ina2" || tag == "ina3" ||
                      tag == "kfc1" || tag == "kfc2" ||
                      tag == "skeleton1" || tag == "skeleton2" || tag == "skeleton3" ||
                      tag == "skeleton4" || tag == "skeleton5" || tag == "skeleton_shield" ||
                      tag == "boss") { // Added boss tag for potential offset needs
                 collider.x = static_cast<int>(basePosX + 32); // Example enemy offset
                 collider.y = static_cast<int>(basePosY + 60); // Example enemy offset
             }
             else if (tag == "projectile" || tag == "exp_orb") {
                 // Projectiles/orbs usually align directly with transform
                 collider.x = static_cast<int>(basePosX);
                 collider.y = static_cast<int>(basePosY);
             }
             else { // Default for unknown dynamic tags
                 collider.x = static_cast<int>(basePosX);
                 collider.y = static_cast<int>(basePosY);
             }
        } else if (tag == "terrain") {
            // Static terrain collider: Use position set in constructor
            collider.x = static_cast<int>(position.x);
            collider.y = static_cast<int>(position.y);
        } else {
             // Should not happen if init succeeded and it's not terrain
             return;
        }

        // Update internal Vector2D position to match the final collider rect top-left
        position.x = static_cast<float>(collider.x);
        position.y = static_cast<float>(collider.y);

        // Update destR for debug drawing (accounts for camera)
        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
        destR.w = collider.w; // Use final calculated width
        destR.h = collider.h; // Use final calculated height
    }

    void draw() override {
        // Optional debug drawing of the collider bounds
        bool debug_draw = false; // Set to true to enable debug drawing
        if (!debug_draw || !initialized || !Game::renderer) return;

        // Set color (e.g., red for debug)
        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 150); // Semi-transparent red
        // Use destR which already has camera offset applied
        SDL_RenderDrawRect(Game::renderer, &destR);
        // Reset draw color (optional, depends on subsequent drawing calls)
        // SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    }

}; // End ColliderComponent class