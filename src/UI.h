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
    // --- Smaller Font Size Adjustment ---
    TTF_Font* uiFont = nullptr;       // New font for general UI text
    TTF_Font* uiHeaderFont = nullptr; // New font for headers like "HEALTH"

    // UI layout constants
    const int HEALTH_BAR_WIDTH = 180; // Slightly smaller bar?
    const int HEALTH_BAR_HEIGHT = 20; // Slightly smaller bar?
    const int UI_PADDING = 15;        // Reduced padding
    const int STAT_LINE_HEIGHT = 18;  // Reduced line height
    const int SECTION_PADDING = 10;   // Padding between UI sections
// --- End Adjustment ---


    // --- Buff Colors ---
    const SDL_Color healthBuffColor = {0, 255, 0, 255};       // Green
    const SDL_Color starSpellBuffColor = {255, 255, 0, 255}; // Yellow
    const SDL_Color fireSpellBuffColor = {255, 0, 0, 255};   // Red
    const SDL_Color weaponBuffColor = {0, 0, 255, 255};      // Blue
    const SDL_Color lifeStealBuffColor = {128, 128, 128, 255}; // Gray
    const SDL_Color defaultBuffColor = {50, 50, 100, 255};   // Default blueish gray
    const SDL_Color maxHealthBuffColor = {0, 180, 0, 255}; // --- Dark Green ---

    Entity* currentBossEntity = nullptr; // Pointer to the current boss
    TTF_Font* bossHealthFont = nullptr; // Optional: Specific font for boss health

    // Helper to render the boss health bar (Keep private)
    void renderBossHealthBar();

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
    SDL_Texture* maxHealthIconTex = nullptr; // --- Use health_icon for now ---
      // --- Declare new spell rendering function ---
      void renderSpellStats(Player* player, int& yPos); // Pass yPos by reference

    public:
    UIManager(SDL_Renderer* renderer);
    ~UIManager();

    void init();
    // --- Update hasFonts check ---
    bool hasFonts() const { return uiFont != nullptr && uiHeaderFont != nullptr && largeFont != nullptr; }

    // --- Update Font Accessors ---
    TTF_Font* getFont() { return uiFont; } // Return general UI font
    TTF_Font* getLargeFont() { return largeFont; } // Keep large font for maybe titles/buffs?
    TTF_Font* getHeaderFont() { return uiHeaderFont; } // Return header font


    // Core Rendering Methods (remain declared)
    void renderPlayerHealthBar(Player* player, int& yPos); // Pass yPos
    void renderExpBar(Player* player, int& yPos);        // Pass yPos
    void renderWeaponStats(Player* player, int& yPos);   // Pass yPos
    void renderPlayerStats(Player* player, int& yPos);   // Pass yPos
    void renderUI(Player* player); // Main entry point

    // Text Helpers (remain declared)
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr);
    void drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth = 1, TTF_Font* fontToUse = nullptr);

    // Buff Selection UI (remain declared)
    void renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight);
    bool isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect);

    // Cache Management (remain declared)
    void clearCache();

    void setBossEntity(Entity* boss);
};