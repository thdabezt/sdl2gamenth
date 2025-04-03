#include "UI.h"
#include "game.h"       // For Game::instance, AssetManager access
#include "Vector2D.h"
#include "ECS/Player.h" // For Player class
#include "ECS/Components.h" // For WeaponComponent, SpellComponent, etc.
#include <iostream>
#include <iomanip>      // For std::fixed, std::setprecision
#include <sstream>      // For std::stringstream
#include <vector>       // For iterating components
#include <string>       // For string manipulation (toupper)
#include <cmath>        // For std::floor

UIManager::UIManager(SDL_Renderer* rend) : renderer(rend), font(nullptr), largeFont(nullptr), uiFont(nullptr), uiHeaderFont(nullptr) {
    // Initialize SDL_ttf if not already done
    if (!TTF_WasInit() && TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        // Constructor should probably indicate failure here, maybe throw?
    }
}

UIManager::~UIManager() {
    clearCache();
    // Free all loaded fonts
    if (font) { TTF_CloseFont(font); font = nullptr; }
    if (largeFont) { TTF_CloseFont(largeFont); largeFont = nullptr; }
    if (uiFont) { TTF_CloseFont(uiFont); uiFont = nullptr; }
    if (uiHeaderFont) { TTF_CloseFont(uiHeaderFont); uiHeaderFont = nullptr; }
    // Icon textures are managed by AssetManager, no need to destroy here
}

void UIManager::init() {
    // Load original fonts (potentially for Buff UI or other elements)
    font = TTF_OpenFont("assets/font.ttf", 14);
    largeFont = TTF_OpenFont("assets/font.ttf", 20); // Keep for Buff UI Title?

    // --- Load New Smaller Fonts for In-Game UI ---
    uiFont = TTF_OpenFont("assets/font.ttf", 12);       // Smaller general text (e.g., 12pt)
    uiHeaderFont = TTF_OpenFont("assets/font.ttf", 14); // Slightly larger for headers (e.g., 14pt)

    if (!font || !largeFont || !uiFont || !uiHeaderFont) {
        std::cerr << "Failed to load one or more fonts from 'assets/font.ttf'. UI may not display text correctly." << std::endl;
        // Implement font fallback logic here if desired
    } else {
        std::cout << "UI Fonts loaded successfully!" << std::endl;
    }

    // Load Icons using AssetManager from Game instance
    if (Game::instance && Game::instance->assets) {
        weaponIconTex = Game::instance->assets->GetTexture("weapon_icon");
        fireIconTex = Game::instance->assets->GetTexture("fire_icon");
        starIconTex = Game::instance->assets->GetTexture("star_icon");
        healthIconTex = Game::instance->assets->GetTexture("health_icon"); // Used for HEAL and MAX_HEALTH buffs
        lifestealIconTex = Game::instance->assets->GetTexture("lifesteal_icon");
        maxHealthIconTex = healthIconTex; // Reuse health icon texture for Max Health buff display
        defaultBuffIconTex = Game::instance->assets->GetTexture("default_buff_icon");
        // Add checks here (if (icon == nullptr) cerr...) to ensure icons loaded
    } else {
        std::cerr << "Error in UIManager::init: Cannot load buff icons, Game instance or AssetManager is null!" << std::endl;
    }
}

// Renders text to a texture, used internally by drawText and drawTextWithOutline
SDL_Texture* UIManager::renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height) {
    if (!fontToUse || text.empty()) { // Use default if null, handle empty string
         fontToUse = uiFont; // Default to the small UI font
         if (!fontToUse || text.empty()) { // Still null or empty? Cannot render.
             width = 0; height = 0; return nullptr;
         }
    }
    if (!renderer) return nullptr;

    SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface ('" << text << "')! SDL_ttf Error: " << TTF_GetError() << std::endl;
        width = 0; height = 0; return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!texture) {
        std::cerr << "Unable to create texture from surface ('" << text << "')! SDL Error: " << SDL_GetError() << std::endl;
        width = 0; height = 0;
    } else {
        width = textSurface->w;
        height = textSurface->h;
    }

    SDL_FreeSurface(textSurface);
    return texture;
}

// Draws text directly to the screen, defaults to the smaller uiFont
void UIManager::drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = uiFont; // Default to smaller uiFont if none specified
    if (!fontToUse || !renderer) return; // Need font and renderer

    int textWidth = 0, textHeight = 0;
    SDL_Texture* textTexture = renderTextToTexture(text, color, fontToUse, textWidth, textHeight);

    if (textTexture) {
        SDL_Rect renderQuad = {x, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
        SDL_DestroyTexture(textTexture); // Clean up temporary texture immediately
    }
}

// Draws text with a simple outline
void UIManager::drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = uiFont; // Default to small font
    if (!fontToUse || !renderer || outlineWidth < 1) return;

    // Render outline texture
    int olWidth = 0, olHeight = 0;
    SDL_Texture* outlineTexture = renderTextToTexture(text, outlineColor, fontToUse, olWidth, olHeight);
    if (!outlineTexture) return; // Bail if outline failed

    // Render outline offsets
    SDL_Rect dst = { 0, 0, olWidth, olHeight };
    for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
        for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
            if (dx == 0 && dy == 0) continue; // Skip center
            dst.x = x + dx;
            dst.y = y + dy;
            SDL_RenderCopy(renderer, outlineTexture, NULL, &dst);
        }
    }
    SDL_DestroyTexture(outlineTexture); // Clean up outline texture

    // Render main text texture on top
    drawText(text, x, y, textColor, fontToUse); // Reuse drawText for the foreground
}


// Renders Player Health Bar
void UIManager::renderPlayerHealthBar(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; // Need player, renderer, and fonts

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color labelColor = white; // Or another color for label

    // Draw label using header font
    drawText("HEALTH", xPos, yPos, labelColor, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT; // Move down for the bar itself

    // Draw Bar Background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Dark grey background
    SDL_Rect bgRect = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2}; // Small border included
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw Bar Border
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // Light grey border
    SDL_RenderDrawRect(renderer, &bgRect);

    // Draw Health Bar Fill (Gradient)
    int maxHP = player->getMaxHealth();
    float healthPercent = (maxHP > 0) ? static_cast<float>(player->getHealth()) / maxHP : 0.0f;
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent);
    int r = static_cast<int>(255 * (1.0f - healthPercent)); int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255); // Green to Red gradient
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthRect);

    // Draw Health Text (Value/Max) - Centered on the bar
    std::stringstream ss; ss << player->getHealth() << "/" << maxHP;
    int textW = 0, textH = 0; TTF_SizeText(uiFont, ss.str().c_str(), &textW, &textH); // Use smaller font
    drawText(ss.str(), xPos + (HEALTH_BAR_WIDTH - textW) / 2, yPos + (HEALTH_BAR_HEIGHT - textH) / 2, white, uiFont);

    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING; // Update yPos for the next UI element
}

// Renders Player Experience Bar
void UIManager::renderExpBar(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return;

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color purple = {180, 100, 255, 255}; // Label color

    // Draw Level Text using header font
    std::stringstream ss_lvl; ss_lvl << "LEVEL " << player->getLevel();
    drawText(ss_lvl.str(), xPos, yPos, purple, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT; // Move down for the bar

    // Draw Bar Background
    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255); // Dark purple background
    SDL_Rect bgRect = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2};
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw Bar Border
    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255); // Lighter purple border
    SDL_RenderDrawRect(renderer, &bgRect);

    // Draw Exp Bar Fill
    float expPercent = player->getExperiencePercentage();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255); // Bright purple fill
    SDL_Rect expRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);

    // Draw Exp Text (Value/Max) - Centered on the bar
    std::stringstream ss_exp; ss_exp << player->getExperience() << "/" << player->getExperienceToNextLevel() << " EXP";
    int textW = 0, textH = 0; TTF_SizeText(uiFont, ss_exp.str().c_str(), &textW, &textH); // Use smaller font
    drawText(ss_exp.str(), xPos + (HEALTH_BAR_WIDTH - textW) / 2, yPos + (HEALTH_BAR_HEIGHT - textH) / 2, white, uiFont);

    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING; // Update yPos for the next UI element
}

// Renders Weapon Stats (including Level)
void UIManager::renderWeaponStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return;
    Entity* playerEntity = nullptr;
    try { playerEntity = &player->getEntity(); } catch (...) { return; }
    if (!playerEntity || !playerEntity->hasComponent<WeaponComponent>()) return; // Need weapon component

    const WeaponComponent& weapon = playerEntity->getComponent<WeaponComponent>();
    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255}; // Header color

    // Weapon Header (including Level) using header font
    std::string weaponName = weapon.getTag(); if (!weaponName.empty()) weaponName[0] = std::toupper(weaponName[0]);
    std::stringstream ss_header; ss_header << weaponName << " - Lv." << weapon.getLevel();
    drawText(ss_header.str(), xPos, yPos, gold, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT + 5; // Add a bit more padding after header

    // Weapon Stats using smaller uiFont
    std::stringstream ss;
    ss << "Damage: " << weapon.getDamage(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    ss.str(""); // Clear stringstream
    float fireRatePerSec = (weapon.getFireRate() > 0) ? 1000.0f / weapon.getFireRate() : 0.0f;
    ss << "Fire Rate: " << std::fixed << std::setprecision(2) << fireRatePerSec << "/sec"; drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Projectiles: " << weapon.getProjectileCount(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Burst Count: " << weapon.getBurstCount(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Pierce: " << weapon.getPierce(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    // Add other stats like speed, range, spread if desired

    yPos += SECTION_PADDING; // Padding before next section
}

// Renders Stats for All Spells (including Level)
// --- Implementation for renderSpellStats (Revised Level 0 display) ---
void UIManager::renderSpellStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return;
    Entity* playerEntity = nullptr;
    try { playerEntity = &player->getEntity(); } catch (...) { return; }
    if (!playerEntity) return;

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color spellColor = {100, 150, 255, 255}; // Light blue for spell headers
    SDL_Color getSpellColor = {180, 180, 180, 255}; // Grey for "Get" text

    int spellCount = 0;
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            spellCount++;
            if(spellCount > 1) yPos += SECTION_PADDING / 2; // Padding between spells

            std::string spellName = spellComp->getTag(); if (!spellName.empty()) spellName[0] = std::toupper(spellName[0]);

            // --- Check Spell Level ---
            if (spellComp->getLevel() == 0) {
                std::stringstream ss_get; ss_get << "Get " << spellName;
                drawText(ss_get.str(), xPos, yPos, getSpellColor, uiHeaderFont); // Use header font size maybe
                yPos += STAT_LINE_HEIGHT + 5; // Add padding like a header
            } else {
                // --- Render normal stats if level > 0 ---
                // Spell Header (including Level) using header font
                std::stringstream ss_header; ss_header << spellName << " - Lv." << spellComp->getLevel();
                drawText(ss_header.str(), xPos, yPos, spellColor, uiHeaderFont);
                yPos += STAT_LINE_HEIGHT + 5; // Padding after header

                // Spell Stats using smaller uiFont
                std::stringstream ss;
                ss << "Damage: " << spellComp->getDamage(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
                ss.str(""); float cooldownSec = (spellComp->getCooldown() > 0) ? spellComp->getCooldown() / 1000.0f : 0.0f;
                ss << "Cooldown: " << std::fixed << std::setprecision(1) << cooldownSec << "s"; drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
                ss.str(""); ss << "Pierce: " << spellComp->getPierce(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;

                // Display type-specific stats
                 if (spellComp->getTag() == "star") {
                     ss.str(""); ss << "Projectiles: " << spellComp->getProjectileCount(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
                 } else if (spellComp->getTag() == "spell") { // Fire
                     ss.str(""); float durationSec = (spellComp->getDuration() > 0) ? spellComp->getDuration() / 1000.0f : 0.0f;
                     ss << "Duration: " << std::fixed << std::setprecision(1) << durationSec << "s"; drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
                 }
                // --- End normal stats ---
            }
             yPos += SECTION_PADDING; // Padding before next spell or section
        }
    }
    // Add padding even if no spells were found, before the next section starts
    if (spellCount == 0) yPos += SECTION_PADDING / 2;
}

// Renders General Player Stats
void UIManager::renderPlayerStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return;

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color headerColor = {100, 200, 255, 255}; // Light blue header

    // Stats Header using header font
    drawText("PLAYER STATS", xPos, yPos, headerColor, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT + 5;

    // Stats using smaller uiFont
    std::stringstream ss;
    ss << "Enemies Defeated: " << player->getEnemiesDefeated(); drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Lifesteal: " << std::fixed << std::setprecision(1) << player->getLifestealPercentage() << "%"; drawText(ss.str(), xPos, yPos, white); yPos += STAT_LINE_HEIGHT;
    // Add other player stats here (e.g., movement speed if it changes)

    yPos += SECTION_PADDING; // Padding at the end
}

// Main UI Rendering Function - Calls the specific renderers
void UIManager::renderUI(Player* player) {
    if (!player || !renderer) return; // Need player and renderer

    if (!hasFonts()) {
        renderSimpleUI(player); // Use fallback if fonts aren't loaded
        return;
    }

    // Manage vertical layout using yPos
    int currentY = UI_PADDING; // Start near the top

    renderPlayerHealthBar(player, currentY);
    renderExpBar(player, currentY);
    renderWeaponStats(player, currentY);
    renderSpellStats(player, currentY); // Call the new spell stats renderer
    renderPlayerStats(player, currentY);

    // Add calls to render any other UI sections here, passing currentY
}

// Fallback Simple UI (if fonts fail)
void UIManager::renderSimpleUI(Player* player) {
    if (!player || !renderer) return;
    // Minimal graphical representation without text
    int xPos = UI_PADDING; int yPos = UI_PADDING;
    // Health bar
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); SDL_Rect bgRectH = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2}; SDL_RenderFillRect(renderer, &bgRectH);
    float healthPercent = (player->getMaxHealth() > 0) ? static_cast<float>(player->getHealth()) / player->getMaxHealth() : 0.0f;
    int currentBarWidthH = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent); int r = static_cast<int>(255 * (1.0f - healthPercent)); int g = static_cast<int>(255 * healthPercent); SDL_SetRenderDrawColor(renderer, r, g, 0, 255); SDL_Rect healthRect = {xPos, yPos, currentBarWidthH, HEALTH_BAR_HEIGHT}; SDL_RenderFillRect(renderer, &healthRect);
    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING;
    // Exp bar
    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255); SDL_Rect bgRectE = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2}; SDL_RenderFillRect(renderer, &bgRectE);
    float expPercent = player->getExperiencePercentage(); int currentBarWidthE = static_cast<int>(HEALTH_BAR_WIDTH * expPercent); SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255); SDL_Rect expRect = {xPos, yPos, currentBarWidthE, HEALTH_BAR_HEIGHT}; SDL_RenderFillRect(renderer, &expRect);
}

// --- UIManager::renderBuffSelectionUI (Revised Box Color Logic) ---
void UIManager::renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight) {
    // Use 'font' (smaller) and 'largeFont' for this UI
    // Use default uiFont if 'font' failed to load
    TTF_Font* fontToUse = font ? font : uiFont;
    if (!fontToUse || !largeFont || buffs.empty() || !renderer) {
         if(buffs.empty()) std::cerr << "Error: renderBuffSelectionUI called with empty buffs vector." << std::endl;
         if(!renderer) std::cerr << "Error: renderBuffSelectionUI called with null renderer." << std::endl;
         if(!fontToUse) std::cerr << "Error: renderBuffSelectionUI - No valid font found (font or uiFont)." << std::endl;
         if(!largeFont) std::cerr << "Error: renderBuffSelectionUI - largeFont is null." << std::endl;
        return;
    }

    // Dynamic Layout Calculation
    const int numButtons = std::min((int)buffs.size(), 4);
    if (numButtons == 0) return; // Should be caught by buffs.empty() check, but good practice
    const int refWidth = 800, refHeight = 600;
    const int baseIconSize = 48, baseBoxW = 180, baseBoxH = 110; // Slightly increased base BoxH for 3 lines
    const int baseGap = 25;
    const int baseTitleOffsetY = 40, baseIconOffsetY = -100; // Y position for icons (relative to center)

    // Calculate scale factors
    float scaleX = static_cast<float>(windowWidth) / refWidth;
    float scaleY = static_cast<float>(windowHeight) / refHeight;
    float scaleFactor = std::min(scaleX, scaleY);

    // Calculate scaled sizes
    int iconAreaSize = std::max(32, static_cast<int>(baseIconSize * scaleFactor));
    int boxW = std::max(100, static_cast<int>(baseBoxW * scaleFactor));
    int boxH = std::max(75, static_cast<int>(baseBoxH * scaleFactor)); // Adjusted min BoxH
    int gap = std::max(15, static_cast<int>(baseGap * scaleFactor));
    int titleOffsetY = static_cast<int>(baseTitleOffsetY * scaleY);
    int iconAreaY = windowHeight / 2 + static_cast<int>(baseIconOffsetY * scaleY);
    // Calculate boxOffsetY using the square iconAreaSize
    int boxOffsetY = iconAreaY + iconAreaSize + std::max(3, static_cast<int>(5 * scaleFactor));

    // Calculate total width needed and starting X position for centering
    int totalWidth = numButtons * boxW + (numButtons - 1) * gap;
    int startX = (windowWidth - totalWidth) / 2;

    // Render Overlay & Title (using largeFont)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Semi-transparent black overlay
    SDL_Rect fullscreen = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Reset blend mode

    SDL_Color titleColor = {255, 215, 0, 255}; // Gold title
    int approxTitleWidth = 300 * scaleFactor; // Approximate for centering
    // Use largeFont for the title
    drawTextWithOutline("Choose an Upgrade!", (windowWidth - approxTitleWidth) / 2, titleOffsetY, titleColor, {0, 0, 0, 255}, 2, largeFont);

    // Button Rendering Loop
    SDL_Color textColor = {255, 255, 255, 255}; // White text for buffs
    SDL_Color outlineColor = {0, 0, 0, 255};   // Black outline for text

    for (int i = 0; i < numButtons; ++i) {
        int currentX = startX + i * (boxW + gap);
        SDL_Rect boxRect = {currentX, boxOffsetY, boxW, boxH};
        int iconAreaX = currentX + (boxW - iconAreaSize) / 2;
        SDL_Rect iconBoundingBox = {iconAreaX, iconAreaY, iconAreaSize, iconAreaSize};

        SDL_Texture* iconToDraw = defaultBuffIconTex; // Default icon
        SDL_Color buttonColor = defaultBuffColor;     // Default box color
        const BuffInfo& currentBuff = buffs[i];       // Get current buff info

        // --- Determine Icon & Box Color based on BuffType ---
        BuffType type = currentBuff.type;
        // Check Categories using the UPDATED BuffType enum values
        if (type >= BuffType::FIRE_SPELL_DAMAGE && type <= BuffType::FIRE_SPELL_PROJ_PLUS_1) {
            buttonColor = fireSpellBuffColor; iconToDraw = fireIconTex;
        } else if (type >= BuffType::STAR_SPELL_DAMAGE && type <= BuffType::STAR_SPELL_PROJ_PLUS_1) { // Use updated range
            buttonColor = starSpellBuffColor; iconToDraw = starIconTex;
        } else if (type == BuffType::WEAPON_DAMAGE_FLAT || type == BuffType::WEAPON_DAMAGE_RAND_PERC || (type >= BuffType::WEAPON_FIRE_RATE && type <= BuffType::WEAPON_BURST_COUNT) || type == BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30) {
             buttonColor = weaponBuffColor; iconToDraw = weaponIconTex;
        } else if (type == BuffType::PLAYER_HEAL_FLAT || type == BuffType::PLAYER_HEAL_PERC_MAX || type == BuffType::PLAYER_HEAL_PERC_LOST || type == BuffType::PLAYER_MAX_HEALTH_FLAT || type == BuffType::PLAYER_MAX_HEALTH_PERC_MAX || type == BuffType::PLAYER_MAX_HEALTH_PERC_CUR) {
            buttonColor = healthBuffColor; iconToDraw = healthIconTex;
        } else if (type == BuffType::PLAYER_LIFESTEAL) {
            buttonColor = lifeStealBuffColor; iconToDraw = lifestealIconTex;
        }
        // --- End Icon & Color Determination ---


        // --- Draw Icon (with NULL check and logging) ---
        // Optional: Add logging to debug icon selection
        // std::cout << "  Buff [" << i << "] Type: " << static_cast<int>(currentBuff.type) << ", Attempting to draw icon..." << std::endl;
        if (!iconToDraw) {
             // std::cout << "  Buff [" << i << "] -> iconToDraw is NULL (Texture likely failed to load or default is null)" << std::endl;
             iconToDraw = defaultBuffIconTex; // Attempt to use default again if specific one was null
             if (!iconToDraw) {
                  // std::cout << "  Buff [" << i << "] -> defaultBuffIconTex is also NULL." << std::endl;
             }
        }

        if (iconToDraw) {
            int texW, texH;
            SDL_QueryTexture(iconToDraw, NULL, NULL, &texW, &texH);
            float wScale = (texW > 0) ? (float)iconBoundingBox.w / texW : 1.0f;
            float hScale = (texH > 0) ? (float)iconBoundingBox.h / texH : 1.0f;
            float iconSpecificScale = std::min(wScale, hScale); // Preserve aspect ratio
            int destW = static_cast<int>(texW * iconSpecificScale);
            int destH = static_cast<int>(texH * iconSpecificScale);
            int destX = iconBoundingBox.x + (iconBoundingBox.w - destW) / 2; // Center horizontally
            int destY = iconBoundingBox.y + (iconBoundingBox.h - destH) / 2; // Center vertically
            SDL_Rect finalIconRect = {destX, destY, destW, destH};
            // Optional: Add logging for icon rect
            // std::cout << "  Buff [" << i << "] Icon Rect: x=" << finalIconRect.x << " y=" << finalIconRect.y << " w=" << finalIconRect.w << " h=" << finalIconRect.h << std::endl;
            SDL_RenderCopy(renderer, iconToDraw, NULL, &finalIconRect);
         }
         // --- End Icon Drawing ---


        // --- Draw Box (with logging) ---
        // Optional: Logging to debug box rendering
        // std::cout << "  Buff [" << i << "] Box Rect: x=" << boxRect.x << " y=" << boxRect.y << " w=" << boxRect.w << " h=" << boxRect.h << std::endl;
        // std::cout << "  Buff [" << i << "] Box Color: r=" << (int)buttonColor.r << " g=" << (int)buttonColor.g << " b=" << (int)buttonColor.b << " a=" << (int)buttonColor.a << std::endl;

        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a); // Set fill color
        SDL_RenderFillRect(renderer, &boxRect); // Fill the box
        SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a); // Set outline color
        SDL_RenderDrawRect(renderer, &boxRect); // Draw outline
        // --- End Box Drawing ---


        // --- Text Drawing (3 Lines) ---
        int textWidth, textHeight;
        // Use the smaller 'fontToUse' for the 3 lines of text
        int textBaseY = boxRect.y + boxRect.h / 8; // Fine-tune starting Y position inside box
        int lineSpacing = static_cast<int>(3 * scaleFactor); // Dynamic line spacing
        lineSpacing = std::max(1, lineSpacing); // Ensure at least 1px spacing
        int currentTextY = textBaseY;
        int textCenterX = boxRect.x + boxRect.w / 2; // Center text horizontally
        SDL_Color line2Color = {210, 210, 210, 255}; // Lighter grey for verb
        SDL_Color line3Color = {255, 255, 255, 255}; // White for effect

        // --- Line 1: Buff Name ---
        TTF_SizeText(fontToUse, currentBuff.name.c_str(), &textWidth, &textHeight);
        // Adjust X to center based on actual text width
        drawTextWithOutline(currentBuff.name, textCenterX - textWidth / 2, currentTextY, textColor, outlineColor, 1, fontToUse);
        currentTextY += textHeight + lineSpacing;

        // --- Determine Line 2 (Verb) & Line 3 (Effect/Value) ---
        std::string line2Text = "";
        std::string line3Text = "";
        std::stringstream ssEffect;

        switch (currentBuff.type) {
             // Player Heals
            case BuffType::PLAYER_HEAL_FLAT:
                line2Text = "Restore"; ssEffect << (int)currentBuff.amount << " HP"; break;
            case BuffType::PLAYER_HEAL_PERC_MAX:
                line2Text = "Restore"; ssEffect << (int)currentBuff.amount << "% Max HP"; break;
            case BuffType::PLAYER_HEAL_PERC_LOST:
                line2Text = "Restore"; ssEffect << (int)currentBuff.amount << "% Lost HP"; break;

            // Player Max HP
            case BuffType::PLAYER_MAX_HEALTH_FLAT:
                line2Text = "Increase"; ssEffect << "+" << (int)currentBuff.amount << " Max HP"; break;
            case BuffType::PLAYER_MAX_HEALTH_PERC_MAX:
                line2Text = "Increase"; ssEffect << "+" << (int)currentBuff.amount << "% Max HP"; break;
            case BuffType::PLAYER_MAX_HEALTH_PERC_CUR:
                line2Text = "Increase Max HP by"; ssEffect << (int)currentBuff.amount << "% Current HP"; break;

            // Lifesteal
            case BuffType::PLAYER_LIFESTEAL:
                line2Text = "Increase"; ssEffect << "+" << std::fixed << std::setprecision(1) << currentBuff.amount << "% Lifesteal"; break;

            // Weapon Damage
            case BuffType::WEAPON_DAMAGE_FLAT:
                 line2Text = "Increase Damage"; ssEffect << "+" << (int)currentBuff.amount << "%"; break; // Show the percentage
            case BuffType::WEAPON_DAMAGE_RAND_PERC:
                line2Text = "Increase Damage";
                if (Game::instance && Game::instance->playerManager && Game::instance->playerManager->getEntity().hasComponent<WeaponComponent>()) {
                     int currentDmg = Game::instance->playerManager->getEntity().getComponent<WeaponComponent>().getDamage();
                     if(currentDmg > 0) {
                         int minInc = std::max(1, static_cast<int>(currentDmg * 0.01f)); int maxInc = std::max(1, static_cast<int>(currentDmg * 0.20f));
                         ssEffect << "+" << minInc << "-" << maxInc << " (1-20%)";
                     } else { ssEffect << "+(1-20)%"; }
                } else { ssEffect << "+(1-20)%"; }
                break;

            // Weapon Fire Rate / Pierce / Burst / Special Proj
            case BuffType::WEAPON_FIRE_RATE:
                line2Text = "Decrease"; ssEffect << (int)currentBuff.amount << "% Fire Delay"; break;
            case BuffType::WEAPON_PIERCE:
                line2Text = "Increase"; ssEffect << "+1 Pierce"; break;
            case BuffType::WEAPON_BURST_COUNT:
                line2Text = "Increase"; ssEffect << "+1 Burst Count"; break;
            case BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30:
                line2Text = "Increase Spread"; ssEffect << "+1 Proj (DMG -30%)"; break;

            // Fire Spell Damage / CDR / Proj
            case BuffType::FIRE_SPELL_DAMAGE:
                 line2Text = "Increase Damage";
                 if (Game::instance && Game::instance->playerManager) { int increase = std::max(1, 1 * Game::instance->playerManager->getLevel()); ssEffect << "+" << increase;}
                 else { ssEffect << "+? (Scales)";}
                 break;
            case BuffType::FIRE_SPELL_COOLDOWN:
                 line2Text = "Decrease"; ssEffect << (int)currentBuff.amount << "% Cooldown"; break;
            case BuffType::FIRE_SPELL_PROJ_PLUS_1:
                 if(currentBuff.name.find("Get") != std::string::npos) { line2Text = "Grants"; ssEffect << "Fire Vortex"; }
                 else { line2Text = "Increase"; ssEffect << "+1 Fire Burst"; }
                 break;

             // Star Spell Damage / CDR / Proj
            case BuffType::STAR_SPELL_DAMAGE:
                 line2Text = "Increase Damage";
                  if (Game::instance && Game::instance->playerManager) { int increase = std::max(1, 3 * Game::instance->playerManager->getLevel()); ssEffect << "+" << increase; }
                  else { ssEffect << "+? (Scales)";}
                 break;
            case BuffType::STAR_SPELL_COOLDOWN:
                 line2Text = "Decrease"; ssEffect << (int)currentBuff.amount << "% Cooldown"; break;
            case BuffType::STAR_SPELL_PROJ_PLUS_1:
                 if(currentBuff.name.find("Get") != std::string::npos) { line2Text = "Grants"; ssEffect << "Starfall"; }
                 else { line2Text = "Increase"; ssEffect << "+1 Star"; }
                 break;

            default: line2Text = "Effect"; ssEffect << "Unknown Buff"; break;
        }
        line3Text = ssEffect.str();

        // --- Line 2: Draw Verb ---
        if (!line2Text.empty()) {
            TTF_SizeText(fontToUse, line2Text.c_str(), &textWidth, &textHeight);
            drawTextWithOutline(line2Text, textCenterX - textWidth / 2, currentTextY, line2Color, outlineColor, 1, fontToUse);
            currentTextY += textHeight + lineSpacing;
        }

        // --- Line 3: Draw Effect/Value ---
        if (!line3Text.empty()) {
             TTF_SizeText(fontToUse, line3Text.c_str(), &textWidth, &textHeight);
             // Adjust Y if line 2 was empty? Maybe not necessary if lineSpacing is sufficient.
             drawTextWithOutline(line3Text, textCenterX - textWidth / 2, currentTextY, line3Color, outlineColor, 1, fontToUse);
        }
        // --- End Text Drawing ---

    } // End of the for loop
}

// Checks if mouse coordinates are inside a given rectangle
bool UIManager::isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}

// Clears the text texture cache
void UIManager::clearCache() {
    for (auto& cache : textCache) { // Use range-based for loop if C++11 or later
        if (cache.texture) {
            SDL_DestroyTexture(cache.texture);
            cache.texture = nullptr; // Important to nullify after destroying
        }
    }
    textCache.clear(); // Clear the vector itself
}