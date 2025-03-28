#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

// Forward declarations
class Entity;
class Vector2D;
class Player;  // Add Player forward declaration

class UIManager {
private:
    SDL_Renderer* renderer;
    TTF_Font* font = nullptr;
    TTF_Font* largeFont = nullptr;
    
    // UI positions and dimensions
    const int HEALTH_BAR_WIDTH = 200;
    const int HEALTH_BAR_HEIGHT = 25;
    const int UI_PADDING = 20;
    const int STAT_LINE_HEIGHT = 24;
    
    // Cache for text textures
    struct TextCache {
        std::string text;
        SDL_Texture* texture;
        int width;
        int height;
        
        TextCache() : texture(nullptr), width(0), height(0) {}
        ~TextCache() {
            if (texture) SDL_DestroyTexture(texture);
        }
    };
    
    std::vector<TextCache> textCache;
    
    // Helper methods
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* font, int& width, int& height);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr);
    void renderSimpleUI(Player* player); // Updated to use Player
    
public:
    UIManager(SDL_Renderer* renderer);
    ~UIManager();
    
    void init();
    bool hasFonts() const { return font != nullptr && largeFont != nullptr; }
    
    // Main render methods - updated to use Player
    void renderPlayerHealthBar(Player* player);
    void renderWeaponStats(Player* player);
    void renderPlayerStats(Player* player);
    void renderExpBar(Player* player);

    void renderUI(Player* player);  // Updated to use Player directly
    
    // Clear cached text
    void clearCache();
};