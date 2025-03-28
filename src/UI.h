#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

// Forward declarations - prevents circular dependencies
class Entity;
class Vector2D;

class UIManager {
private:
    SDL_Renderer* renderer;
    TTF_Font* font = nullptr;
    TTF_Font* largeFont = nullptr;
    int enemiesDefeated = 0;
    
    // Level system
    int playerLevel = 1;
    int playerExp = 0;
    int expToNextLevel = 10; // Initial value: 1 * 10
    
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
    void renderSimpleUI(Entity& player); // Fallback when fonts aren't available
    
public:
    UIManager(SDL_Renderer* renderer);
    ~UIManager();
    
    void init();
    void incrementEnemiesDefeated() { 
        enemiesDefeated++; 
        
        // Add experience when defeating an enemy
        addExperience(playerLevel * 10);
    }
    int getEnemiesDefeated() const { return enemiesDefeated; }
    bool hasFonts() const { return font != nullptr && largeFont != nullptr; }
    
    // Level system methods
    void addExperience(int exp);
    int getPlayerLevel() const { return playerLevel; }
    int getPlayerExp() const { return playerExp; }
    int getExpToNextLevel() const { return expToNextLevel; }
    float getExpPercentage() const { return static_cast<float>(playerExp) / expToNextLevel; }
    
    // Main render methods
    void renderPlayerHealthBar(Entity& player);
    void renderWeaponStats(Entity& player);
    void renderPlayerStats();
    void renderExpBar();
    void renderUI(Entity& player);
    
    // Clear cached text
    void clearCache();
};