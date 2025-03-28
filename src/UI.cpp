#include "UI.h"
#include "game.h"
#include "Vector2D.h"

#include "ECS/Player.h"
// #include <sstream>
#include <iostream>
#include <iomanip> // For std::setprecision

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

void UIManager::renderSimpleUI(Player* player) {
    if (!player) return;
    
    Entity& playerEntity = player->getEntity();
    if (!playerEntity.hasComponent<HealthComponent>()) {
        return;
    }
    
    // Position in top left corner
    int xPos = UI_PADDING;
    int yPos = UI_PADDING;
    
    // Health bar (at the top)
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect bgRect = {xPos - 2, yPos - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &bgRect);
    
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    float healthPercent = static_cast<float>(player->getHealth()) / player->getMaxHealth();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent);
    
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthRect);
    
    // XP Bar (below health)
    int expBarY = yPos + HEALTH_BAR_HEIGHT + 10;
    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255);
    SDL_Rect expBgRect = {xPos - 2, expBarY - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &expBgRect);
    
    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255);
    SDL_RenderDrawRect(renderer, &expBgRect);
    
    float expPercent = player->getExperiencePercentage();
    int currentExpWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255);
    SDL_Rect expRect = {xPos, expBarY, currentExpWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);
    
    // Weapon stats area
    yPos = expBarY + HEALTH_BAR_HEIGHT + 20;
    SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255);
    SDL_Rect weaponRect = {xPos, yPos, HEALTH_BAR_WIDTH, 100};
    SDL_RenderFillRect(renderer, &weaponRect);
    SDL_SetRenderDrawColor(renderer, 120, 120, 200, 255);
    SDL_RenderDrawRect(renderer, &weaponRect);
    
    // Level indicator box
    yPos += 120;
    SDL_SetRenderDrawColor(renderer, 60, 10, 80, 255);
    SDL_Rect levelRect = {xPos, yPos, HEALTH_BAR_WIDTH, 40};
    SDL_RenderFillRect(renderer, &levelRect);
    SDL_SetRenderDrawColor(renderer, 180, 100, 255, 255);
    SDL_RenderDrawRect(renderer, &levelRect);
    
    // Draw level indicator blocks
    int blockWidth = 20;
    int blockPadding = 5;
    int blockStart = xPos + 10;
    int level = player->getLevel();
    
    for (int i = 0; i < std::min(level, 8); i++) {
        SDL_Rect levelBlock = {blockStart + (i * (blockWidth + blockPadding)), yPos + 10, blockWidth, 20};
        SDL_SetRenderDrawColor(renderer, 180, 100, 255, 255);
        SDL_RenderFillRect(renderer, &levelBlock);
    }
}

void UIManager::renderPlayerHealthBar(Player* player) {
    if (!player) return;
    
    // Position in top left corner
    int xPos = UI_PADDING;
    int yPos = UI_PADDING; // At the very top
    
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
    float healthPercent = static_cast<float>(player->getHealth()) / player->getMaxHealth();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent);
    
    // Draw health bar with gradient from green to red
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthRect);
    
    // Draw health text
    std::stringstream ss;
    ss << player->getHealth() << "/" << player->getMaxHealth();
    drawText(ss.str(), xPos + HEALTH_BAR_WIDTH/2 - 20, yPos + 4, white);
}

void UIManager::renderWeaponStats(Player* player) {
    if (!player) return;
    
    Entity& playerEntity = player->getEntity();
    if (!playerEntity.hasComponent<WeaponComponent>()) {
        return;
    }
    
    const WeaponComponent& weapon = playerEntity.getComponent<WeaponComponent>();
    
    // Position below EXP bar
    int xPos = UI_PADDING;
    int yPos = UI_PADDING + HEALTH_BAR_HEIGHT*2 + 35;
    
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

void UIManager::renderPlayerStats(Player* player) {
    if (!player) return;
    
    // Position below weapon stats
    int xPos = UI_PADDING;
    int yPos = UI_PADDING + HEALTH_BAR_HEIGHT*2 + 35 + (5 * STAT_LINE_HEIGHT);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color lightBlue = {100, 200, 255, 255};
    
    // Stats header
    drawText("STATS", xPos, yPos, lightBlue, largeFont);
    
    // Show enemies defeated
    yPos += 30;
    std::stringstream ss;
    ss << "Enemies Defeated: " << player->getEnemiesDefeated();
    drawText(ss.str(), xPos, yPos, white);
    
    // Show player level
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Player Level: " << player->getLevel();
    drawText(ss.str(), xPos, yPos, white);
    
    // Show total experience gained
    yPos += STAT_LINE_HEIGHT;
    ss.str("");
    ss << "Experience: " << player->getExperience() << "/" << player->getExperienceToNextLevel();
    drawText(ss.str(), xPos, yPos, white);
}

void UIManager::renderExpBar(Player* player) {
    if (!player) return;
    
    // Position at below health bar
    int xPos = UI_PADDING;
    int yPos = UI_PADDING + HEALTH_BAR_HEIGHT + 10;  // Below health bar
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color purple = {180, 100, 255, 255};
    
    // Draw level text
    std::stringstream ss;
    ss << "LEVEL " << player->getLevel();
    drawText(ss.str(), xPos, yPos - 24, purple, largeFont);
    
    // Draw background (dark purple)
    SDL_SetRenderDrawColor(renderer, 40, 10, 60, 255);
    SDL_Rect bgRect = {xPos - 2, yPos - 2, HEALTH_BAR_WIDTH + 4, HEALTH_BAR_HEIGHT + 4};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Calculate exp percentage
    float expPercent = player->getExperiencePercentage();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    
    // Draw exp bar (purple gradient)
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255);
    SDL_Rect expRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);
    
    // Draw exp text
    ss.str("");
    ss << player->getExperience() << "/" << player->getExperienceToNextLevel() << " EXP";
    drawText(ss.str(), xPos + HEALTH_BAR_WIDTH/2 - 30, yPos + 4, white);
}

void UIManager::renderUI(Player* player) {
    if (!player) return;
    
    if (!hasFonts()) {
        // Use the fallback rendering if fonts aren't available
        renderSimpleUI(player);
        return;
    }
    
    // Normal text-based UI
    renderPlayerHealthBar(player);
    renderExpBar(player);
    renderWeaponStats(player);
    renderPlayerStats(player);
}

void UIManager::clearCache() {
    for (TextCache& cache : textCache) {
        if (cache.texture) {
            SDL_DestroyTexture(cache.texture);
            cache.texture = nullptr;
        }
    }
    textCache.clear();
}
bool UIManager::isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}


void UIManager::renderBuffSelectionUI(const std::vector<BuffInfo>& buffs) {
    if (!hasFonts() || buffs.empty()) return;

    // Semi-transparent background overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Darker overlay
    SDL_Rect fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Title
    SDL_Color titleColor = {255, 215, 0, 255}; // Gold
    drawText("Choose an Upgrade!", WINDOW_WIDTH / 2 - 150, 50, titleColor, largeFont);

    // Button layout (example: 4 horizontal buttons)
    int totalButtonWidth = 4 * 180 + 3 * 20; // 4 buttons * width + 3 gaps
    int startX = (WINDOW_WIDTH - totalButtonWidth) / 2;
    int buttonY = WINDOW_HEIGHT / 2 - 50;
    int buttonW = 180;
    int buttonH = 100;
    int gap = 20;

    SDL_Color buttonColor = {50, 50, 100, 255};
    SDL_Color borderColor = {150, 150, 200, 255};
    SDL_Color textColor = {255, 255, 255, 255};

    for (size_t i = 0; i < buffs.size() && i < 4; ++i) {
        SDL_Rect buttonRect = {startX + static_cast<int>(i) * (buttonW + gap), buttonY, buttonW, buttonH};

        // Draw button background and border
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer, &buttonRect);
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &buttonRect);

        // Draw buff name and description (centered)
        // You might need to adjust text positioning based on font size and text length
        int textWidth, textHeight;
        renderTextToTexture(buffs[i].name, textColor, font, textWidth, textHeight); // Use helper to get size
        drawText(buffs[i].name, buttonRect.x + (buttonRect.w - textWidth) / 2, buttonRect.y + 15, textColor, font);

        renderTextToTexture(buffs[i].description, textColor, font, textWidth, textHeight);
        drawText(buffs[i].description, buttonRect.x + (buttonRect.w - textWidth) / 2, buttonRect.y + 45, textColor, font);

         // Draw amount
         std::stringstream ss;
         // Check if amount is whole number for cleaner display
         if (buffs[i].amount == floor(buffs[i].amount)) {
             ss << "+" << static_cast<int>(buffs[i].amount);
         } else {
              ss << "+" << std::fixed << std::setprecision(1) << buffs[i].amount;
         }
         renderTextToTexture(ss.str(), textColor, font, textWidth, textHeight);
         drawText(ss.str(), buttonRect.x + (buttonRect.w - textWidth) / 2, buttonRect.y + 75, textColor, font);
    }
}

