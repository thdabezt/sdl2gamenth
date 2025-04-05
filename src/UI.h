#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

class Entity;
class Vector2D;
class Player;
struct BuffInfo;

class UIManager {
private:

    SDL_Renderer* renderer;
    Entity* currentBossEntity = nullptr; 

    TTF_Font* font = nullptr;       
    TTF_Font* largeFont = nullptr;    
    TTF_Font* uiFont = nullptr;       
    TTF_Font* uiHeaderFont = nullptr; 
    TTF_Font* bossHealthFont = nullptr; 

    SDL_Texture* weaponIconTex = nullptr;
    SDL_Texture* fireIconTex = nullptr;
    SDL_Texture* starIconTex = nullptr;
    SDL_Texture* healthIconTex = nullptr;
    SDL_Texture* lifestealIconTex = nullptr;
    SDL_Texture* defaultBuffIconTex = nullptr;
    SDL_Texture* maxHealthIconTex = nullptr; 

    struct TextCache {
        std::string text;
        SDL_Texture* texture;
        int width;
        int height;
        TextCache() : texture(nullptr), width(0), height(0) {}
        ~TextCache() { if (texture) SDL_DestroyTexture(texture); }
    };
    std::vector<TextCache> textCache;

    const int HEALTH_BAR_WIDTH = 180;
    const int HEALTH_BAR_HEIGHT = 20;
    const int UI_PADDING = 15;
    const int STAT_LINE_HEIGHT = 18;
    const int SECTION_PADDING = 10;

    const SDL_Color healthBuffColor = {0, 255, 0, 255};       
    const SDL_Color starSpellBuffColor = {255, 255, 0, 255}; 
    const SDL_Color fireSpellBuffColor = {255, 0, 0, 255};   
    const SDL_Color weaponBuffColor = {0, 0, 255, 255};      
    const SDL_Color lifeStealBuffColor = {128, 128, 128, 255}; 
    const SDL_Color defaultBuffColor = {50, 50, 100, 255};   
    const SDL_Color maxHealthBuffColor = {0, 180, 0, 255};   

    void renderBossHealthBar();
    void renderSimpleUI(Player* player); 
    void renderSpellStats(Player* player, int& yPos); 

public:

    UIManager(SDL_Renderer* renderer);
    ~UIManager();

    void init(); 

    bool hasFonts() const;
    TTF_Font* getFont() { return uiFont; } 
    TTF_Font* getLargeFont() { return largeFont; }
    TTF_Font* getHeaderFont() { return uiHeaderFont; }

    void renderUI(Player* player); 
    void renderPlayerHealthBar(Player* player, int& yPos);
    void renderExpBar(Player* player, int& yPos);
    void renderWeaponStats(Player* player, int& yPos);
    void renderPlayerStats(Player* player, int& yPos);

    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse = nullptr);
    void drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth = 1, TTF_Font* fontToUse = nullptr);

    void renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight);

    bool isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect);
    void clearCache(); 
    void setBossEntity(Entity* boss); 

}; 