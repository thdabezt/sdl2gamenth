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
    
    // --- Add more logging to this section ---
    std::cout << "DEBUG: UIManager::init - Attempting to load icons..." << std::endl;
    if (Game::instance && Game::instance->assets) {
        weaponIconTex = Game::instance->assets->GetTexture("weapon_icon");
        std::cout << "       Loaded weapon_icon: Pointer = " << (weaponIconTex ? "Valid" : "NULL") << std::endl; // Log result immediately

        fireIconTex = Game::instance->assets->GetTexture("fire_icon");
        std::cout << "       Loaded fire_icon: Pointer = " << (fireIconTex ? "Valid" : "NULL") << std::endl;

        starIconTex = Game::instance->assets->GetTexture("star_icon");
        std::cout << "       Loaded star_icon: Pointer = " << (starIconTex ? "Valid" : "NULL") << std::endl;

        healthIconTex = Game::instance->assets->GetTexture("health_icon");
        std::cout << "       Loaded health_icon: Pointer = " << (healthIconTex ? "Valid" : "NULL") << std::endl;

        lifestealIconTex = Game::instance->assets->GetTexture("lifesteal_icon");
        std::cout << "       Loaded lifesteal_icon: Pointer = " << (lifestealIconTex ? "Valid" : "NULL") << std::endl;

        defaultBuffIconTex = Game::instance->assets->GetTexture("default_buff_icon");
        std::cout << "       Loaded default_buff_icon: Pointer = " << (defaultBuffIconTex ? "Valid" : "NULL") << std::endl;

    } else {
        std::cerr << "Error in UIManager::init: Cannot load buff icons, Game instance or AssetManager is null!" << std::endl;
    }
    // --- End Icon Loading ---
    

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

// Replace the entire UIManager::renderBuffSelectionUI function in UI.cpp with this:
// Replace the entire UIManager::renderBuffSelectionUI function in UI.cpp with this:
void UIManager::renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight) {
    // --- Debug Log Entry ---
    // std::cout << "DEBUG: renderBuffSelectionUI called." << std::endl;

    if (!hasFonts() || buffs.empty()) {
        // std::cout << "DEBUG: renderBuffSelectionUI returning early (no fonts or buffs)." << std::endl;
        return;
    }

    // --- Dynamic Layout Calculation ---
    const int numButtons = std::min((int)buffs.size(), 4);
    if (numButtons == 0) {
        // std::cout << "DEBUG: renderBuffSelectionUI returning early (numButtons is 0)." << std::endl;
        return;
    }

    // Define base sizes and padding relative to a reference resolution
    const int refWidth = 800;
    const int refHeight = 600;
    const int baseIconSize = 48; // Base size for the SQUARE AREA allocated for the icon
    const int baseBoxW = 180;
    const int baseBoxH = 100;
    const int baseGap = 25;
    const int baseTitleOffsetY = 40;
    const int baseIconOffsetY = -100; // Y position for the top of the icon area (relative to center)
    // Note: baseBoxOffsetY is calculated after iconAreaHeight below

    // Calculate scale factors
    float scaleX = static_cast<float>(windowWidth) / refWidth;
    float scaleY = static_cast<float>(windowHeight) / refHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    // Apply scaling (consider adding minimum sizes)
    // --- Calculate the SQUARE AREA allocated for the icon ---
    int iconAreaSize = std::max(32, static_cast<int>(baseIconSize * scaleFactor));
    // --- This defines the max width and height for the icon ---

    int boxW = std::max(100, static_cast<int>(baseBoxW * scaleFactor));
    int boxH = std::max(60, static_cast<int>(baseBoxH * scaleFactor));
    int gap = std::max(15, static_cast<int>(baseGap * scaleFactor));
    int titleOffsetY = static_cast<int>(baseTitleOffsetY * scaleY);
    // Adjust Y offsets based on the square icon AREA size
    int iconAreaY = windowHeight / 2 + static_cast<int>(baseIconOffsetY * scaleY); // Top of the area for the icon
    int boxOffsetY = iconAreaY + iconAreaSize + std::max(3, static_cast<int>(5 * scaleFactor)); // Position box below the icon area

    // Calculate total width needed and starting position for centering
    int totalWidth = numButtons * boxW + (numButtons - 1) * gap;
    int startX = (windowWidth - totalWidth) / 2;

    // --- Rendering ---

    // Semi-transparent background overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fullscreen = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Title
    SDL_Color titleColor = {255, 215, 0, 255};
    int approxTitleWidth = 300 * scaleX;
    drawTextWithOutline("Choose an Upgrade!", (windowWidth - approxTitleWidth) / 2, titleOffsetY, titleColor, {0, 0, 0, 255}, 2, largeFont);

    // Button rendering loop
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color outlineColor = {0, 0, 0, 255}; // Black outline color for box and text

    for (int i = 0; i < numButtons; ++i) {
        int currentX = startX + i * (boxW + gap);
        SDL_Rect boxRect = {currentX, boxOffsetY, boxW, boxH}; // Box position

        // Define the bounding box for the icon area
        int iconAreaX = currentX + (boxW - iconAreaSize) / 2; // Center the area horizontally above the box
        SDL_Rect iconBoundingBox = {iconAreaX, iconAreaY, iconAreaSize, iconAreaSize};

        // Determine Icon Texture AND Button Background Color
        SDL_Texture* iconToDraw = defaultBuffIconTex;
        SDL_Color buttonColor = defaultBuffColor; // Default color
        const BuffInfo& currentBuff = buffs[i];

        switch (currentBuff.type) {
            // Cases to select iconToDraw and buttonColor remain the same...
            case BuffType::PLAYER_HEAL:
                iconToDraw = healthIconTex; buttonColor = healthBuffColor; break;
            case BuffType::STAR_SPELL_DAMAGE: case BuffType::STAR_SPELL_COOLDOWN: // Fallthrough
            case BuffType::STAR_SPELL_PIERCE: case BuffType::STAR_SPELL_PROJ_COUNT:
                iconToDraw = starIconTex; buttonColor = starSpellBuffColor; break;
            case BuffType::FIRE_SPELL_DAMAGE: case BuffType::FIRE_SPELL_COOLDOWN: // Fallthrough
            case BuffType::FIRE_SPELL_PIERCE: case BuffType::FIRE_SPELL_DURATION:
                iconToDraw = fireIconTex; buttonColor = fireSpellBuffColor; break;
            case BuffType::WEAPON_DAMAGE: case BuffType::WEAPON_FIRE_RATE: // Fallthrough
            case BuffType::WEAPON_PROJ_COUNT: case BuffType::WEAPON_PIERCE: case BuffType::WEAPON_BURST_COUNT:
                iconToDraw = weaponIconTex; buttonColor = weaponBuffColor; break;
            default:
                iconToDraw = defaultBuffIconTex; buttonColor = defaultBuffColor; break;
        }

        // Draw Icon (with aspect ratio preservation)
        if (iconToDraw) {
            int texW, texH;
            SDL_QueryTexture(iconToDraw, NULL, NULL, &texW, &texH); // Get original texture size

            // Calculate scaling factor to fit texture within iconBoundingBox while preserving aspect ratio
            float wScale = (texW > 0) ? (float)iconBoundingBox.w / texW : 1.0f;
            float hScale = (texH > 0) ? (float)iconBoundingBox.h / texH : 1.0f;
            float iconSpecificScale = std::min(wScale, hScale); // Use smaller scale to fit

            // Calculate final destination dimensions
            int destW = static_cast<int>(texW * iconSpecificScale);
            int destH = static_cast<int>(texH * iconSpecificScale);

            // Calculate final destination position (centered within the bounding box)
            int destX = iconBoundingBox.x + (iconBoundingBox.w - destW) / 2;
            int destY = iconBoundingBox.y + (iconBoundingBox.h - destH) / 2;

            SDL_Rect finalIconRect = {destX, destY, destW, destH};

            // Render using the calculated final rect
            SDL_RenderCopy(renderer, iconToDraw, NULL, &finalIconRect);

        } else {
            // Optional: Draw a placeholder if icon is NULL
        }

        // Draw colored background fill for the box below
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer, &boxRect);

        // Draw black outline for the box
        SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(renderer, &boxRect);

        // --- Draw Text with Outline --- (Code remains the same)
        int textWidth, textHeight;
        int textYOffset = boxRect.h / 5;
        int textCenterX = boxRect.x + boxRect.w / 2;

        // Buff Name
        TTF_SizeText(font, buffs[i].name.c_str(), &textWidth, &textHeight);
        drawTextWithOutline(buffs[i].name, textCenterX - textWidth / 2, boxRect.y + textYOffset, textColor, outlineColor, 1, font);

        // Buff Description
        TTF_SizeText(font, buffs[i].description.c_str(), &textWidth, &textHeight);
        drawTextWithOutline(buffs[i].description, textCenterX - textWidth / 2, boxRect.y + textYOffset * 2, textColor, outlineColor, 1, font);

        // Buff Amount
        std::stringstream ss;
        if (buffs[i].amount == floor(buffs[i].amount)) { ss << "+" << static_cast<int>(buffs[i].amount); }
        else { ss << "+" << std::fixed << std::setprecision(1) << buffs[i].amount; }
        TTF_SizeText(font, ss.str().c_str(), &textWidth, &textHeight);
        drawTextWithOutline(ss.str(), textCenterX - textWidth / 2, boxRect.y + textYOffset * 3, textColor, outlineColor, 1, font);

    } // End For Loop
} // End Function

void UIManager::drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = font;
    if (!fontToUse || !renderer) return; // Need font and renderer

    // Create textures for outline and text (inefficient, but simple)
    // A more optimized version would cache surfaces/textures.
    SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), textColor);
    SDL_Surface* outlineSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), outlineColor);

    if (!textSurface || !outlineSurface) {
        std::cerr << "Unable to render text surface for outline! SDL_ttf Error: " << TTF_GetError() << std::endl;
        if (textSurface) SDL_FreeSurface(textSurface);
        if (outlineSurface) SDL_FreeSurface(outlineSurface);
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Texture* outlineTexture = SDL_CreateTextureFromSurface(renderer, outlineSurface);

    if (!textTexture || !outlineTexture) {
        std::cerr << "Unable to create text texture for outline! SDL Error: " << SDL_GetError() << std::endl;
        if (textTexture) SDL_DestroyTexture(textTexture);
        if (outlineTexture) SDL_DestroyTexture(outlineTexture);
        // Free surfaces here as well
        SDL_FreeSurface(textSurface);
        SDL_FreeSurface(outlineSurface);
        return;
    }

    SDL_Rect dstRect = { x, y, textSurface->w, textSurface->h };

    // Render outlines by offsetting
    for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
        for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
            if (dx == 0 && dy == 0) continue; // Skip center position for outline
            // Simple way: render outline texture at offset positions
            // More complex: only render if dx*dx + dy*dy <= outlineWidth*outlineWidth for rounded look
            SDL_Rect outlineDst = dstRect;
            outlineDst.x += dx;
            outlineDst.y += dy;
            SDL_RenderCopy(renderer, outlineTexture, NULL, &outlineDst);
        }
    }


    // Render the main text on top
    SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(outlineTexture);
    SDL_FreeSurface(textSurface);
    SDL_FreeSurface(outlineSurface);
}