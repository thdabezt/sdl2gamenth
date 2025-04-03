#pragma once

#include <SDL.h>
#include <SDL_ttf.h> // Make sure SDL_ttf is correctly linked
#include <string>
#include <vector>

// Forward declarations
class Entity;
class Vector2D;
class Player;
struct BuffInfo; // Forward declare BuffInfo struct (defined in game.h)

class UIManager {
private: // Keep member variables private
    SDL_Renderer* renderer;
    TTF_Font* font = nullptr;
    TTF_Font* largeFont = nullptr;

    // UI layout constants (example values)
    const int HEALTH_BAR_WIDTH = 200;
    const int HEALTH_BAR_HEIGHT = 25;
    const int UI_PADDING = 20;
    const int STAT_LINE_HEIGHT = 24;


    // --- Buff Colors ---
    const SDL_Color healthBuffColor = {0, 255, 0, 255};       // Green
    const SDL_Color starSpellBuffColor = {255, 255, 0, 255}; // Yellow
    const SDL_Color fireSpellBuffColor = {255, 0, 0, 255};   // Red
    const SDL_Color weaponBuffColor = {0, 0, 255, 255};      // Blue
    const SDL_Color lifeStealBuffColor = {128, 128, 128, 255}; // Gray
    const SDL_Color defaultBuffColor = {50, 50, 100, 255};   // Default blueish gray
    // Cache for text textures (Keep private)
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

    // Simple UI fallback (Keep private if only called by public renderUI)
    void renderSimpleUI(Player* player);

    SDL_Texture* weaponIconTex = nullptr;
    SDL_Texture* fireIconTex = nullptr;
    SDL_Texture* starIconTex = nullptr;
    SDL_Texture* healthIconTex = nullptr;
    SDL_Texture* lifestealIconTex = nullptr;
    SDL_Texture* defaultBuffIconTex = nullptr;

public: // Public interface
    UIManager(SDL_Renderer* renderer);
    ~UIManager();

    void init();
    bool hasFonts() const { return font != nullptr && largeFont != nullptr; }

    // --- Font Accessors ---
    TTF_Font* getFont() { return font; }
    TTF_Font* getLargeFont() { return largeFont; }

    // --- Core Rendering Methods ---
    void renderPlayerHealthBar(Player* player);
    void renderWeaponStats(Player* player);
    void renderPlayerStats(Player* player);
    void renderExpBar(Player* player);
    void renderUI(Player* player); // Combined UI rendering

    // --- Text Helpers (Make Public) ---
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr);


    // --- Buff Selection UI ---
    void renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight);
    bool isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect);
    void drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth = 1, TTF_Font* fontToUse = nullptr);
    // --- Cache Management ---
    void clearCache();

}; // End of UIManager class definition