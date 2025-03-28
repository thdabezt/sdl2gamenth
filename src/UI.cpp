#include "UI.h"
#include "game.h"
#include "Vector2D.h"
#include "ECS/Components.h"
#include <sstream>
#include <iostream>

UIManager::UIManager(SDL_Renderer* renderer) : renderer(renderer) {
    // Initialize SDL_ttf if not already initialized
    if (!TTF_WasInit() && TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
}

UIManager::~UIManager() {
    clearCache();
    
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    if (largeFont) {
        TTF_CloseFont(largeFont);
        largeFont = nullptr;
    }
}

void UIManager::init() {
    // Try to load font from the correct path
    font = TTF_OpenFont("assets/font.ttf", 16);
    largeFont = TTF_OpenFont("assets/font.ttf", 24);
    
    if (!font || !largeFont) {
        std::cerr << "Failed to load font at assets/font.ttf! SDL_ttf Error: " << TTF_GetError() << std::endl;
        std::cerr << "Trying alternative paths..." << std::endl;
        
        // Try alternative paths
        const char* altPaths[] = {
            "./font.ttf",
            "./assets/font.ttf",
            "../assets/font.ttf",
            "assets/fonts/arial.ttf",
            "C:/Windows/Fonts/arial.ttf"  // Windows system font as last resort
        };
        
        for (const char* path : altPaths) {
            std::cout << "Trying font path: " << path << std::endl;
            font = TTF_OpenFont(path, 16);
            largeFont = TTF_OpenFont(path, 24);
            
            if (font && largeFont) {
                std::cout << "Successfully loaded font from: " << path << std::endl;
                break;
            }
            
            // Clean up if one loaded but the other didn't
            if (font) {
                TTF_CloseFont(font);
                font = nullptr;
            }
            if (largeFont) {
                TTF_CloseFont(largeFont);
                largeFont = nullptr;
            }
        }
    }
    
    if (font && largeFont) {
        std::cout << "Font loaded successfully!" << std::endl;
    } else {
        std::cerr << "Could not load any fonts. UI will use simple graphics only." << std::endl;
    }
}

SDL_Texture* UIManager::renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height) {
    if (!fontToUse) fontToUse = font;
    
    // Safety check - don't try to render with NULL font
    if (!fontToUse) {
        return nullptr;
    }
    
    SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    width = textSurface->w;
    height = textSurface->h;
    
    SDL_FreeSurface(textSurface);
    return texture;
}

void UIManager::drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = font;
    
    // Safety check - don't try to render with NULL font
    if (!fontToUse) return;
    
    int textWidth, textHeight;
    SDL_Texture* textTexture = renderTextToTexture(text, color, fontToUse, textWidth, textHeight);
    
    if (textTexture) {
        SDL_Rect renderQuad = {x, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
        SDL_DestroyTexture(textTexture);
    }
}

// Update this method:

void UIManager::renderSimpleUI(Entity& player) {
    if (!player.hasComponent<HealthComponent>()) {
        return;
    }
    
    const HealthComponent& health = player.getComponent<HealthComponent>();
    
    // Position in top right corner
    int xPos = Game::camera.w - HEALTH_BAR_WIDTH - UI_PADDING;
    int yPos = UI_PADDING;
    
    // XP Bar (above health)
    int expBarY = yPos - 30;
    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255);
    SDL_Rect expBgRect = {xPos - 2, expBarY - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &expBgRect);
    
    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255);
    SDL_RenderDrawRect(renderer, &expBgRect);
    
    float expPercent = getExpPercentage();
    int currentExpWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255);
    SDL_Rect expRect = {xPos, expBarY, currentExpWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);
    
    // Health bar (existing code)
    // (rest of the method remains the same)
    
    // Add level indicator box
    yPos += HEALTH_BAR_HEIGHT + 120 + 50;
    SDL_SetRenderDrawColor(renderer, 60, 10, 80, 255);
    SDL_Rect levelRect = {xPos, yPos, HEALTH_BAR_WIDTH, 40};
    SDL_RenderFillRect(renderer, &levelRect);
    SDL_SetRenderDrawColor(renderer, 180, 100, 255, 255);
    SDL_RenderDrawRect(renderer, &levelRect);
    
    // Draw level indicator block
    int blockWidth = 20;
    int blockPadding = 5;
    int blockStart = xPos + 10;
    
    for (int i = 0; i < std::min(playerLevel, 8); i++) {
        SDL_Rect levelBlock = {blockStart + (i * (blockWidth + blockPadding)), yPos + 10, blockWidth, 20};
        SDL_SetRenderDrawColor(renderer, 180, 100, 255, 255);
        SDL_RenderFillRect(renderer, &levelBlock);
    }
}

void UIManager::renderPlayerHealthBar(Entity& player) {
    if (!player.hasComponent<HealthComponent>()) {
        return;
    }
    
    const HealthComponent& health = player.getComponent<HealthComponent>();
    
    // Position in top right corner
    int xPos = Game::camera.w - HEALTH_BAR_WIDTH - UI_PADDING;
    int yPos = UI_PADDING;
    
    // Draw bar label
    SDL_Color white = {255, 255, 255, 255};
    drawText("HEALTH", xPos, yPos - 24, white, largeFont);
    
    // Draw background (black)
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect bgRect = {xPos - 2, yPos - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border (white)
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Calculate health percentage
    float healthPercent = static_cast<float>(health.getHealth()) / health.getMaxHealth();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent);
    
    // Draw health bar with gradient from green to red
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthRect);
    
    // Draw health text
    std::stringstream ss;
    ss << health.getHealth() << "/" << health.getMaxHealth();
    drawText(ss.str(), xPos + HEALTH_BAR_WIDTH/2 - 20, yPos + 4, white);
}

void UIManager::renderWeaponStats(Entity& player) {
    if (!player.hasComponent<WeaponComponent>()) {
        return;
    }
    
    const WeaponComponent& weapon = player.getComponent<WeaponComponent>();
    
    // Position below health bar
    int xPos = Game::camera.w - HEALTH_BAR_WIDTH - UI_PADDING;
    int yPos = UI_PADDING + HEALTH_BAR_HEIGHT + 20;
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};
    
    // Weapon header
    std::string weaponName = weapon.tag;
    // Capitalize first letter
    if (!weaponName.empty()) {
        weaponName[0] = std::toupper(weaponName[0]);
    }
    drawText(weaponName, xPos, yPos, gold, largeFont);
    
    // Weapon stats
    yPos += 30;
    
    std::stringstream ss;
    ss << "Damage: " << weapon.damage;
    drawText(ss.str(), xPos, yPos, white);
    
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Fire Rate: " << (1000.0f / weapon.fireRate) << "/sec";
    drawText(ss.str(), xPos, yPos, white);
    
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Projectiles: " << weapon.projectilesPerShot;
    drawText(ss.str(), xPos, yPos, white);
    
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Speed: " << weapon.projectileSpeed;
    drawText(ss.str(), xPos, yPos, white);
}

// Update these existing methods:

void UIManager::renderPlayerStats() {
    // Position below weapon stats
    int xPos = Game::camera.w - HEALTH_BAR_WIDTH - UI_PADDING;
    int yPos = UI_PADDING + HEALTH_BAR_HEIGHT + 20 + (5 * STAT_LINE_HEIGHT);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color lightBlue = {100, 200, 255, 255};
    
    // Stats header
    drawText("STATS", xPos, yPos, lightBlue, largeFont);
    
    // Show enemies defeated
    yPos += 30;
    std::stringstream ss;
    ss << "Enemies Defeated: " << enemiesDefeated;
    drawText(ss.str(), xPos, yPos, white);
    
    // Show player level
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Player Level: " << playerLevel;
    drawText(ss.str(), xPos, yPos, white);
    
    // Show total experience gained
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Experience: " << playerExp << "/" << expToNextLevel;
    drawText(ss.str(), xPos, yPos, white);
}

void UIManager::renderUI(Entity& player) {
    if (!hasFonts()) {
        // Use the fallback rendering if fonts aren't available
        renderSimpleUI(player);
        return;
    }
    
    // Normal text-based UI
    renderExpBar();  // Add XP bar above health
    renderPlayerHealthBar(player);
    renderWeaponStats(player);
    renderPlayerStats();
}

void UIManager::clearCache() {
    for (auto& cache : textCache) {
        if (cache.texture) {
            SDL_DestroyTexture(cache.texture);
            cache.texture = nullptr;
        }
    }
    textCache.clear();
}
// Add this method implementation to your file:

void UIManager::addExperience(int exp) {
    playerExp += exp;
    
    // Check if player has enough experience to level up
    while (playerExp >= expToNextLevel) {
        // Level up!
        playerExp -= expToNextLevel;
        playerLevel++;
        
        // Calculate new experience required for next level
        expToNextLevel = playerLevel * 10;
        
        // Log level up message
        std::cout << "LEVEL UP! Now level " << playerLevel << std::endl;
    }
}

// Add this method to render the experience bar
void UIManager::renderExpBar() {
    // Position above the health bar
    int xPos = Game::camera.w - HEALTH_BAR_WIDTH - UI_PADDING;
    int yPos = UI_PADDING - 30;  // Above health bar
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color purple = {180, 100, 255, 255};
    
    // Draw level text
    std::stringstream ss;
    ss << "LEVEL " << playerLevel;
    drawText(ss.str(), xPos, yPos - 24, purple, largeFont);
    
    // Draw background (dark purple)
    SDL_SetRenderDrawColor(renderer, 40, 10, 60, 255);
    SDL_Rect bgRect = {xPos - 2, yPos - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Calculate exp percentage
    float expPercent = getExpPercentage();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    
    // Draw exp bar (purple gradient)
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255);
    SDL_Rect expRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);
    
    // Draw exp text
    ss.str("");
    ss << playerExp << "/" << expToNextLevel << " EXP";
    drawText(ss.str(), xPos + HEALTH_BAR_WIDTH/2 - 30, yPos + 4, white);
}