#pragma once

// --- Includes ---
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

// --- Forward Declarations ---
class Entity;
class Vector2D;
class Player;
struct BuffInfo;

// --- Class Definition ---

class UIManager {
private:
    // --- Private Members ---

    // Core Pointers & State
    SDL_Renderer* renderer;
    Entity* currentBossEntity = nullptr; // Pointer to the current boss

    // Fonts
    TTF_Font* font = nullptr;       // General purpose font (14pt)
    TTF_Font* largeFont = nullptr;    // Larger font (e.g., titles) (20pt)
    TTF_Font* uiFont = nullptr;       // Smaller font for UI text (12pt)
    TTF_Font* uiHeaderFont = nullptr; // Font for UI headers (14pt)
    TTF_Font* bossHealthFont = nullptr; // Specific font for boss health bar (24pt)

    // Buff Icon Textures
    SDL_Texture* weaponIconTex = nullptr;
    SDL_Texture* fireIconTex = nullptr;
    SDL_Texture* starIconTex = nullptr;
    SDL_Texture* healthIconTex = nullptr;
    SDL_Texture* lifestealIconTex = nullptr;
    SDL_Texture* defaultBuffIconTex = nullptr;
    SDL_Texture* maxHealthIconTex = nullptr; // Reuses healthIconTex

    // Text Cache (Not currently used but kept for potential future implementation)
    struct TextCache {
        std::string text;
        SDL_Texture* texture;
        int width;
        int height;
        TextCache() : texture(nullptr), width(0), height(0) {}
        ~TextCache() { if (texture) SDL_DestroyTexture(texture); }
    };
    std::vector<TextCache> textCache;

    // UI Layout Constants
    const int HEALTH_BAR_WIDTH = 180;
    const int HEALTH_BAR_HEIGHT = 20;
    const int UI_PADDING = 15;
    const int STAT_LINE_HEIGHT = 18;
    const int SECTION_PADDING = 10;

    // Buff UI Colors
    const SDL_Color healthBuffColor = {0, 255, 0, 255};       // Green
    const SDL_Color starSpellBuffColor = {255, 255, 0, 255}; // Yellow
    const SDL_Color fireSpellBuffColor = {255, 0, 0, 255};   // Red
    const SDL_Color weaponBuffColor = {0, 0, 255, 255};      // Blue
    const SDL_Color lifeStealBuffColor = {128, 128, 128, 255}; // Gray
    const SDL_Color defaultBuffColor = {50, 50, 100, 255};   // Default blueish gray
    const SDL_Color maxHealthBuffColor = {0, 180, 0, 255};   // Dark Green

    // --- Private Methods ---
    void renderBossHealthBar();
    void renderSimpleUI(Player* player); // Fallback UI
    void renderSpellStats(Player* player, int& yPos); // Renders stats for all spells

public:
    // --- Constructor & Destructor ---
    UIManager(SDL_Renderer* renderer);
    ~UIManager();

    // --- Public Methods ---
    void init(); // Loads fonts and icons

    // Font Accessors & Checks
    bool hasFonts() const;
    TTF_Font* getFont() { return uiFont; } // Return general UI font
    TTF_Font* getLargeFont() { return largeFont; }
    TTF_Font* getHeaderFont() { return uiHeaderFont; }

    // Core Rendering Methods
    void renderUI(Player* player); // Main UI render call
    void renderPlayerHealthBar(Player* player, int& yPos);
    void renderExpBar(Player* player, int& yPos);
    void renderWeaponStats(Player* player, int& yPos);
    void renderPlayerStats(Player* player, int& yPos);

    // Text Rendering Helpers
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr);
    void drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth = 1, TTF_Font* fontToUse = nullptr);

    // Buff Selection UI
    void renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight);

    // Utility Methods
    bool isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect);
    void clearCache(); // Clears the text texture cache (if used)
    void setBossEntity(Entity* boss); // Sets the boss entity for health bar display

}; // End UIManager class