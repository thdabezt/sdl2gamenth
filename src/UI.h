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
    void renderBuffSelectionUI(const std::vector<BuffInfo>& buffs);
    bool isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect);

    // --- Cache Management ---
    void clearCache();

}; // End of UIManager class definition