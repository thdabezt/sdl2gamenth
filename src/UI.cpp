#include "UI.h"
#include "game.h"       
#include "Vector2D.h"
#include "ECS/Player.h" 
#include "ECS/Components.h" 
#include <iostream>
#include <iomanip>      
#include <sstream>      
#include <vector>
#include <string>
#include <algorithm>    
#include <cmath>        

UIManager::UIManager(SDL_Renderer* rend) : renderer(rend), font(nullptr), largeFont(nullptr), uiFont(nullptr), uiHeaderFont(nullptr), bossHealthFont(nullptr) {
    if (!TTF_WasInit() && TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;

    }
}

UIManager::~UIManager() {
    clearCache(); 

    if (font) { TTF_CloseFont(font); font = nullptr; }
    if (largeFont) { TTF_CloseFont(largeFont); largeFont = nullptr; }
    if (uiFont) { TTF_CloseFont(uiFont); uiFont = nullptr; }
    if (uiHeaderFont) { TTF_CloseFont(uiHeaderFont); uiHeaderFont = nullptr; }

    if (bossHealthFont && bossHealthFont != uiHeaderFont && bossHealthFont != largeFont && bossHealthFont != font && bossHealthFont != uiFont) {
        TTF_CloseFont(bossHealthFont);
    }
    bossHealthFont = nullptr;
}

void UIManager::init() {

    font = TTF_OpenFont("assets/font.ttf", 14);
    largeFont = TTF_OpenFont("assets/font.ttf", 20);
    uiFont = TTF_OpenFont("assets/font.ttf", 12);
    uiHeaderFont = TTF_OpenFont("assets/font.ttf", 14);
    bossHealthFont = TTF_OpenFont("assets/font.ttf", 24);

    if (!font) std::cerr << "Failed to load font (14pt)." << std::endl;
    if (!largeFont) std::cerr << "Failed to load largeFont (20pt)." << std::endl;
    if (!uiFont) {
        std::cerr << "Failed to load uiFont (12pt)! Using font as fallback." << std::endl;
        uiFont = font; 
    }
    if (!uiHeaderFont) {
        std::cerr << "Failed to load uiHeaderFont (14pt)! Using largeFont as fallback." << std::endl;
        uiHeaderFont = largeFont; 
    }
     if (!bossHealthFont) {
         std::cerr << "Failed to load boss health font (24pt)! Using uiHeaderFont/largeFont as fallback." << std::endl;
         bossHealthFont = uiHeaderFont ? uiHeaderFont : largeFont; 
     }

    if (!hasFonts()) {
         std::cerr << "UI Warning: One or more essential fonts failed to load, UI text may not display correctly." << std::endl;
    }

    if (Game::instance && Game::instance->assets) {
        weaponIconTex = Game::instance->assets->GetTexture("weapon_icon");
        fireIconTex = Game::instance->assets->GetTexture("fire_icon");
        starIconTex = Game::instance->assets->GetTexture("star_icon");
        healthIconTex = Game::instance->assets->GetTexture("health_icon");
        lifestealIconTex = Game::instance->assets->GetTexture("lifesteal_icon");
        maxHealthIconTex = healthIconTex; 
        defaultBuffIconTex = Game::instance->assets->GetTexture("default_buff_icon");

    } else {
        std::cerr << "Error in UIManager::init: Cannot load buff icons, Game instance or AssetManager is null!" << std::endl;
    }
}

bool UIManager::hasFonts() const {

    return uiFont != nullptr && uiHeaderFont != nullptr && largeFont != nullptr;
}

SDL_Texture* UIManager::renderTextToTexture(const std::string& text, SDL_Color color, TTF_Font* fontToUse, int& width, int& height) {
    width = 0; height = 0; 
    if (!fontToUse) fontToUse = uiFont; 
    if (!fontToUse || text.empty() || !renderer) {
        if (text.empty()) return nullptr; 
        if (!fontToUse) std::cerr << "renderTextToTexture Error: fontToUse is null!" << std::endl;
        if (!renderer) std::cerr << "renderTextToTexture Error: renderer is null!" << std::endl;
        return nullptr;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface ('" << text << "')! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!texture) {
        std::cerr << "Unable to create texture from surface ('" << text << "')! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        width = textSurface->w;
        height = textSurface->h;
    }

    SDL_FreeSurface(textSurface);
    return texture;
}

void UIManager::drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = uiFont;
    if (!fontToUse || !renderer) return;

    int textWidth = 0, textHeight = 0;
    SDL_Texture* textTexture = renderTextToTexture(text, color, fontToUse, textWidth, textHeight);

    if (textTexture) {
        SDL_Rect renderQuad = {x, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
        SDL_DestroyTexture(textTexture);
    }
}

void UIManager::drawTextWithOutline(const std::string& text, int x, int y, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth, TTF_Font* fontToUse) {
    if (!fontToUse) fontToUse = uiFont;
    if (!fontToUse || !renderer || outlineWidth < 1) return;

    int olWidth = 0, olHeight = 0;
    SDL_Texture* outlineTexture = renderTextToTexture(text, outlineColor, fontToUse, olWidth, olHeight);
    if (!outlineTexture) return;

    SDL_Rect dst = { 0, 0, olWidth, olHeight };
    for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
        for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
            if (dx == 0 && dy == 0) continue;
            dst.x = x + dx;
            dst.y = y + dy;
            SDL_RenderCopy(renderer, outlineTexture, NULL, &dst);
        }
    }
    SDL_DestroyTexture(outlineTexture);

    drawText(text, x, y, textColor, fontToUse);
}

void UIManager::renderPlayerHealthBar(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; 

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color labelColor = white;

    drawText("HEALTH", xPos, yPos, labelColor, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); 
    SDL_Rect bgRect = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2};
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); 
    SDL_RenderDrawRect(renderer, &bgRect);

    int maxHP = player->getMaxHealth();
    float healthPercent = (maxHP > 0) ? static_cast<float>(player->getHealth()) / maxHP : 0.0f;
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent);
    int r = static_cast<int>(255 * (1.0f - healthPercent)); int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255); 
    SDL_Rect healthRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthRect);

    std::stringstream ss; ss << player->getHealth() << "/" << maxHP;
    int textW = 0, textH = 0; TTF_SizeText(uiFont, ss.str().c_str(), &textW, &textH);
    drawText(ss.str(), xPos + (HEALTH_BAR_WIDTH - textW) / 2, yPos + (HEALTH_BAR_HEIGHT - textH) / 2, white, uiFont);

    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING;
}

void UIManager::renderExpBar(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; 

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color purple = {180, 100, 255, 255};

    std::stringstream ss_lvl; ss_lvl << "LEVEL " << player->getLevel();
    drawText(ss_lvl.str(), xPos, yPos, purple, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255); 
    SDL_Rect bgRect = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2};
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_SetRenderDrawColor(renderer, 140, 80, 200, 255); 
    SDL_RenderDrawRect(renderer, &bgRect);

    float expPercent = player->getExperiencePercentage();
    int currentBarWidth = static_cast<int>(HEALTH_BAR_WIDTH * expPercent);
    SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255); 
    SDL_Rect expRect = {xPos, yPos, currentBarWidth, HEALTH_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &expRect);

    std::stringstream ss_exp; ss_exp << player->getExperience() << "/" << player->getExperienceToNextLevel() << " EXP";
    int textW = 0, textH = 0; TTF_SizeText(uiFont, ss_exp.str().c_str(), &textW, &textH);
    drawText(ss_exp.str(), xPos + (HEALTH_BAR_WIDTH - textW) / 2, yPos + (HEALTH_BAR_HEIGHT - textH) / 2, white, uiFont);

    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING;
}

void UIManager::renderWeaponStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; 
    Entity* playerEntity = nullptr;
    try { playerEntity = &player->getEntity(); } catch (...) { return; }
    if (!playerEntity || !playerEntity->hasComponent<WeaponComponent>()) return;

    const WeaponComponent& weapon = playerEntity->getComponent<WeaponComponent>();
    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color black = {0, 0, 0, 255}; 

    std::string weaponName = "Main Weapon"; 
    std::stringstream ss_header;
    ss_header << weaponName << " - Lv." << weapon.getLevel();

    drawTextWithOutline(ss_header.str(), xPos, yPos, gold, black, 1, uiHeaderFont); 

    yPos += STAT_LINE_HEIGHT + 5;

    std::stringstream ss;
    ss << "Damage: " << weapon.getDamage(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
    ss.str("");
    float fireRatePerSec = (weapon.getFireRate() > 0) ? 1000.0f / weapon.getFireRate() : 0.0f;
    ss << "Fire Rate: " << std::fixed << std::setprecision(2) << fireRatePerSec << "/sec"; drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Projectiles: " << weapon.getProjectileCount(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Burst Count: " << weapon.getBurstCount(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Pierce: " << weapon.getPierce(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;

    yPos += SECTION_PADDING;
}

void UIManager::renderSpellStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; 
    Entity* playerEntity = nullptr;
    try { playerEntity = &player->getEntity(); } catch (...) { return; }
    if (!playerEntity) return;

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};

    SDL_Color fireColor = fireSpellBuffColor; 
    SDL_Color starColor = starSpellBuffColor; 
    SDL_Color defaultSpellColor = {100, 150, 255, 255}; 
    SDL_Color getSpellColor = {180, 180, 180, 255};

    int spellCount = 0;
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            spellCount++;
            if(spellCount > 1) yPos += SECTION_PADDING / 2;

            std::string spellName = "";
            SDL_Color headerColor = defaultSpellColor;
            std::string currentTag = spellComp->getTag();

            if (currentTag == "spell") { 
                spellName = "Fire Vortex";
                headerColor = fireColor;
            } else if (currentTag == "star") { 
                spellName = "Starfall";
                headerColor = starColor;
            } else { 
                spellName = spellComp->getTag(); if (!spellName.empty()) spellName[0] = std::toupper(spellName[0]);
            }

            if (spellComp->getLevel() == 0) {
                std::stringstream ss_get; ss_get << "Get " << spellName;
                drawText(ss_get.str(), xPos, yPos, getSpellColor, uiHeaderFont);
                yPos += STAT_LINE_HEIGHT + 5;
            } else {
                std::stringstream ss_header; ss_header << spellName << " - Lv." << spellComp->getLevel();

                drawText(ss_header.str(), xPos, yPos, headerColor, uiHeaderFont);
                yPos += STAT_LINE_HEIGHT + 5;

                std::stringstream ss;
                ss << "Damage: " << spellComp->getDamage(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
                ss.str(""); float cooldownSec = (spellComp->getCooldown() > 0) ? spellComp->getCooldown() / 1000.0f : 0.0f;
                ss << "Cooldown: " << std::fixed << std::setprecision(1) << cooldownSec << "s"; drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
                ss.str(""); ss << "Pierce: " << spellComp->getPierce(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;

                 if (currentTag == "star") { 
                     ss.str(""); ss << "Projectiles: " << spellComp->getProjectileCount(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
                 }

            }
             yPos += SECTION_PADDING;
        }
    }
    if (spellCount == 0) yPos += SECTION_PADDING / 2; 
}

void UIManager::renderPlayerStats(Player* player, int& yPos) {
    if (!player || !renderer || !hasFonts()) return; 

    int xPos = UI_PADDING;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color headerColor = {100, 200, 255, 255};

    drawText("PLAYER STATS", xPos, yPos, headerColor, uiHeaderFont);
    yPos += STAT_LINE_HEIGHT + 5;

    std::stringstream ss;
    ss << "Enemies Defeated: " << player->getEnemiesDefeated(); drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;
    ss.str(""); ss << "Lifesteal: " << std::fixed << std::setprecision(1) << player->getLifestealPercentage() << "%"; drawText(ss.str(), xPos, yPos, white, uiFont); yPos += STAT_LINE_HEIGHT;

    yPos += SECTION_PADDING;
}

void UIManager::renderBossHealthBar() {
    if (!currentBossEntity || !currentBossEntity->isActive() || !currentBossEntity->hasComponent<HealthComponent>()) {
        return;
    }
    TTF_Font* fontToUse = bossHealthFont ? bossHealthFont : uiHeaderFont; 
    if (!renderer || !fontToUse) return;

    const HealthComponent& bossHealth = currentBossEntity->getComponent<HealthComponent>();
    int currentHP = bossHealth.getHealth();
    int maxHP = bossHealth.getMaxHealth();

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    const int barWidth = windowWidth * 3 / 5;
    const int barHeight = 30;
    const int barX = (windowWidth - barWidth) / 2;
    const int barY = 20;

    SDL_Color labelColor = {230, 230, 230, 255};

    SDL_SetRenderDrawColor(renderer, 50, 10, 10, 200); 
    SDL_Rect bgRect = {barX - 2, barY - 2, barWidth + 4, barHeight + 4};
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_SetRenderDrawColor(renderer, 150, 40, 40, 255); 
    SDL_RenderDrawRect(renderer, &bgRect);

    float healthPercent = (maxHP > 0) ? static_cast<float>(currentHP) / maxHP : 0.0f;
    int currentBarWidth = static_cast<int>(barWidth * healthPercent);
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255); 
    SDL_Rect healthRect = {barX, barY, currentBarWidth, barHeight}; 
    SDL_RenderFillRect(renderer, &healthRect);

    std::stringstream ss; ss << "BOSS: " << currentHP << " / " << maxHP;
    int textW = 0, textH = 0; TTF_SizeText(fontToUse, ss.str().c_str(), &textW, &textH);
    drawTextWithOutline(ss.str(), barX + (barWidth - textW) / 2, barY + (barHeight - textH) / 2, labelColor, {0, 0, 0, 255}, 1, fontToUse);
}

void UIManager::renderUI(Player* player) {
    if (!renderer) return;

    renderBossHealthBar(); 

    if (!player) return; 

    if (!hasFonts()) { 
        renderSimpleUI(player); 
        return;
    }

    int currentY = UI_PADDING; 
    renderPlayerHealthBar(player, currentY);
    renderExpBar(player, currentY);
    renderWeaponStats(player, currentY);
    renderSpellStats(player, currentY);
    renderPlayerStats(player, currentY);
}

void UIManager::renderSimpleUI(Player* player) {

    if (!player || !renderer) return;
    int xPos = UI_PADDING; int yPos = UI_PADDING;

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); SDL_Rect bgRectH = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2}; SDL_RenderFillRect(renderer, &bgRectH);
    float healthPercent = (player->getMaxHealth() > 0) ? static_cast<float>(player->getHealth()) / player->getMaxHealth() : 0.0f;
    int currentBarWidthH = static_cast<int>(HEALTH_BAR_WIDTH * healthPercent); int r = static_cast<int>(255 * (1.0f - healthPercent)); int g = static_cast<int>(255 * healthPercent); SDL_SetRenderDrawColor(renderer, r, g, 0, 255); SDL_Rect healthRect = {xPos, yPos, currentBarWidthH, HEALTH_BAR_HEIGHT}; SDL_RenderFillRect(renderer, &healthRect);
    yPos += HEALTH_BAR_HEIGHT + SECTION_PADDING;

    SDL_SetRenderDrawColor(renderer, 30, 10, 40, 255); SDL_Rect bgRectE = {xPos - 1, yPos - 1, HEALTH_BAR_WIDTH + 2, HEALTH_BAR_HEIGHT + 2}; SDL_RenderFillRect(renderer, &bgRectE);
    float expPercent = player->getExperiencePercentage(); int currentBarWidthE = static_cast<int>(HEALTH_BAR_WIDTH * expPercent); SDL_SetRenderDrawColor(renderer, 130, 30, 240, 255); SDL_Rect expRect = {xPos, yPos, currentBarWidthE, HEALTH_BAR_HEIGHT}; SDL_RenderFillRect(renderer, &expRect);
}

void UIManager::renderBuffSelectionUI(const std::vector<BuffInfo>& buffs, int windowWidth, int windowHeight) {
    TTF_Font* fontToUse = font ? font : uiFont; 
    if (!fontToUse || !largeFont || buffs.empty() || !renderer) {
         if(buffs.empty()) std::cerr << "Error: renderBuffSelectionUI called with empty buffs vector." << std::endl;

        return;
    }

    const int numButtons = std::min((int)buffs.size(), 4);
    if (numButtons == 0) return;
    const int refWidth = 800, refHeight = 600;
    const int baseIconSize = 48, baseBoxW = 180, baseBoxH = 110, baseGap = 25;
    const int baseTitleOffsetY = 40, baseIconOffsetY = -100;

    float scaleX = static_cast<float>(windowWidth) / refWidth; float scaleY = static_cast<float>(windowHeight) / refHeight; float scaleFactor = std::min(scaleX, scaleY);
    int iconAreaSize = std::max(32, static_cast<int>(baseIconSize * scaleFactor));
    int boxW = std::max(100, static_cast<int>(baseBoxW * scaleFactor)); int boxH = std::max(75, static_cast<int>(baseBoxH * scaleFactor));
    int gap = std::max(15, static_cast<int>(baseGap * scaleFactor));
    int titleOffsetY = static_cast<int>(baseTitleOffsetY * scaleY);
    int iconAreaY = windowHeight / 2 + static_cast<int>(baseIconOffsetY * scaleY);
    int boxOffsetY = iconAreaY + iconAreaSize + std::max(3, static_cast<int>(5 * scaleFactor));
    int totalWidth = numButtons * boxW + (numButtons - 1) * gap; int startX = (windowWidth - totalWidth) / 2;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fullscreen = {0, 0, windowWidth, windowHeight}; SDL_RenderFillRect(renderer, &fullscreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_Color titleColor = {255, 215, 0, 255};
    int approxTitleWidth = static_cast<int>(300 * scaleFactor);
    drawTextWithOutline("Choose an Upgrade!", (windowWidth - approxTitleWidth) / 2, titleOffsetY, titleColor, {0, 0, 0, 255}, 2, largeFont);

    SDL_Color outlineColor = {0, 0, 0, 255};
    for (int i = 0; i < numButtons; ++i) {
        int currentX = startX + i * (boxW + gap);
        SDL_Rect boxRect = {currentX, boxOffsetY, boxW, boxH};
        int iconAreaX = currentX + (boxW - iconAreaSize) / 2;
        SDL_Rect iconBoundingBox = {iconAreaX, iconAreaY, iconAreaSize, iconAreaSize};

        SDL_Texture* iconToDraw = defaultBuffIconTex;
        SDL_Color buttonColor = defaultBuffColor;
        const BuffInfo& currentBuff = buffs[i];
        BuffType type = currentBuff.type;

        if (type >= BuffType::FIRE_SPELL_DAMAGE && type <= BuffType::FIRE_SPELL_PROJ_PLUS_1) { buttonColor = fireSpellBuffColor; iconToDraw = fireIconTex; }
        else if (type >= BuffType::STAR_SPELL_DAMAGE && type <= BuffType::STAR_SPELL_PROJ_PLUS_1) { buttonColor = starSpellBuffColor; iconToDraw = starIconTex; }
        else if (type == BuffType::WEAPON_DAMAGE_FLAT || type == BuffType::WEAPON_DAMAGE_RAND_PERC || (type >= BuffType::WEAPON_FIRE_RATE && type <= BuffType::WEAPON_BURST_COUNT) || type == BuffType::WEAPON_PROJ_PLUS_1_DMG_MINUS_30) { buttonColor = weaponBuffColor; iconToDraw = weaponIconTex; }
        else if (type >= BuffType::PLAYER_HEAL_FLAT && type <= BuffType::PLAYER_MAX_HEALTH_PERC_CUR) { buttonColor = healthBuffColor; iconToDraw = healthIconTex; }
        else if (type == BuffType::PLAYER_LIFESTEAL) { buttonColor = lifeStealBuffColor; iconToDraw = lifestealIconTex; }

        if (!iconToDraw) iconToDraw = defaultBuffIconTex; 
        if (iconToDraw) {
            int texW, texH; SDL_QueryTexture(iconToDraw, NULL, NULL, &texW, &texH);
            float wScale = (texW > 0) ? (float)iconBoundingBox.w / texW : 1.0f; float hScale = (texH > 0) ? (float)iconBoundingBox.h / texH : 1.0f;
            float iconSpecificScale = std::min(wScale, hScale);
            int destW = static_cast<int>(texW * iconSpecificScale); int destH = static_cast<int>(texH * iconSpecificScale);
            int destX = iconBoundingBox.x + (iconBoundingBox.w - destW) / 2; int destY = iconBoundingBox.y + (iconBoundingBox.h - destH) / 2;
            SDL_Rect finalIconRect = {destX, destY, destW, destH};
            SDL_RenderCopy(renderer, iconToDraw, NULL, &finalIconRect);
         }

        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer, &boxRect);
        SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(renderer, &boxRect);

        int textPadding = std::max(2, static_cast<int>(5 * scaleFactor));
        int availableTextWidth = boxRect.w - 2 * textPadding;
        int textBaseY = boxRect.y + textPadding; 
        int currentTextY = textBaseY;
        SDL_Color titleColor = {255, 255, 255, 255};
        SDL_Color descColor = {210, 210, 210, 255};

        if (!currentBuff.name.empty()) {

             int textWidth, textHeight;
             TTF_SizeText(fontToUse, currentBuff.name.c_str(), &textWidth, &textHeight);
             int titleX = boxRect.x + (boxRect.w - textWidth) / 2; 
             drawTextWithOutline(currentBuff.name, titleX, currentTextY, titleColor, outlineColor, 1, fontToUse);
             currentTextY += textHeight + std::max(1, static_cast<int>(3 * scaleFactor)); 
        }

    if (!currentBuff.description.empty() && availableTextWidth > 0) {
        SDL_Color outlineColor = {0, 0, 0, 255}; 
        int outlineWidth = 1;                    

        SDL_Surface* outlineSurface = TTF_RenderText_Blended_Wrapped(fontToUse, currentBuff.description.c_str(), outlineColor, availableTextWidth);
        SDL_Texture* outlineTexture = nullptr;
        if (outlineSurface) {
            outlineTexture = SDL_CreateTextureFromSurface(renderer, outlineSurface);
            if (!outlineTexture) {
                std::cerr << "Failed to create texture for outline wrapped description! SDL Error: " << SDL_GetError() << std::endl; 
            }

            SDL_FreeSurface(outlineSurface);
        } else {
             std::cerr << "Failed to render outline wrapped text surface! TTF_Error: " << TTF_GetError() << std::endl; 
        }

        SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(fontToUse, currentBuff.description.c_str(), descColor, availableTextWidth);
        SDL_Texture* textTexture = nullptr;
        SDL_Rect descriptionRect = {0, 0, 0, 0}; 

        if (textSurface) {
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
             if (textTexture) {
                descriptionRect.w = textSurface->w;
                descriptionRect.h = textSurface->h;

                descriptionRect.x = boxRect.x + (boxRect.w - descriptionRect.w) / 2;

                descriptionRect.y = currentTextY;
             } else {
                 std::cerr << "Failed to create texture for main wrapped description! SDL Error: " << SDL_GetError() << std::endl; 
             }
             SDL_FreeSurface(textSurface);
        } else {
             std::cerr << "Failed to render main wrapped text surface! TTF_Error: " << TTF_GetError() << std::endl; 
        }

        if (outlineTexture && descriptionRect.w > 0 && descriptionRect.h > 0) {

             bool fitsVertically = (currentTextY + descriptionRect.h <= boxRect.y + boxRect.h - textPadding);
             int renderHeight = fitsVertically ? descriptionRect.h : (boxRect.y + boxRect.h - textPadding - currentTextY);
             if (renderHeight > 0) {
                 SDL_Rect clippedSrcRect = {0, 0, descriptionRect.w, renderHeight};
                 SDL_Rect baseDestRect = descriptionRect; 
                 baseDestRect.h = renderHeight;

                 for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
                     for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
                         if (dx == 0 && dy == 0) continue; 
                         SDL_Rect outlineDestRect = baseDestRect;
                         outlineDestRect.x += dx;
                         outlineDestRect.y += dy;
                         SDL_RenderCopy(renderer, outlineTexture, &clippedSrcRect, &outlineDestRect);
                     }
                 }
             }
        }
         if(outlineTexture) {
              SDL_DestroyTexture(outlineTexture); 
         }

        if (textTexture && descriptionRect.w > 0 && descriptionRect.h > 0) {

             bool fitsVertically = (currentTextY + descriptionRect.h <= boxRect.y + boxRect.h - textPadding);
             int renderHeight = fitsVertically ? descriptionRect.h : (boxRect.y + boxRect.h - textPadding - currentTextY);
             if (renderHeight > 0) {
                 SDL_Rect clippedSrcRect = {0, 0, descriptionRect.w, renderHeight};
                 SDL_Rect finalDestRect = descriptionRect; 
                 finalDestRect.h = renderHeight;
                 SDL_RenderCopy(renderer, textTexture, &clippedSrcRect, &finalDestRect);
             }
             SDL_DestroyTexture(textTexture); 
            }
        }
    }
}    

bool UIManager::isMouseInside(int mouseX, int mouseY, const SDL_Rect& rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
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

void UIManager::setBossEntity(Entity* boss) {
    currentBossEntity = boss;
}