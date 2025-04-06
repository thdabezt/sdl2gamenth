#include "SaveLoadManager.h"
#include "game.h"          
#include "ECS/Components.h"   
#include "ECS/Player.h"       
#include <fstream>
#include <sstream>
#include <filesystem>         
#include <chrono>             
#include <iomanip>            
#include <ctime>              
#include <vector>
#include <algorithm>          
#include <SDL_mixer.h>        
#include <iostream>           

SaveLoadManager::SaveLoadManager(Game* game) : gameInstance(game) {
    if (!gameInstance) {

        throw std::runtime_error("SaveLoadManager requires a valid Game instance pointer.");
    }
}

std::string SaveLoadManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c); 

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%d%m%Y-%H%M%S");
    return oss.str();
}

void SaveLoadManager::saveGameState(const std::string& filename) {

    if (!gameInstance || !gameInstance->playerManager || !gameInstance->playerEntity) {
         std::cerr << "Error saving: Game instance or player not initialized!" << std::endl;
         return;
    }

    std::string saveFilename = filename;
    std::string saveDir = "saves";

    try {
        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directory(saveDir);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating save directory '" << saveDir << "': " << e.what() << std::endl;
        return;
    }

    if (saveFilename.empty()) {
        saveFilename = saveDir + "/" + getCurrentTimestamp() + ".state";
    } else if (filename == "default.state") {
         saveFilename = "default.state"; 
    } else if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        saveFilename = saveDir + "/" + filename; 
    }

    std::ofstream saveFile(saveFilename);
    if (!saveFile.is_open()) {
        std::cerr << "Error: Could not open save file: " << saveFilename << std::endl;
        return;
    }

    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

    saveFile << "PlayerName:" << gameInstance->getPlayerName() << "\n";
    saveFile << "PlayerLevel:" << playerManager->getLevel() << "\n";
    saveFile << "PlayerExperience:" << playerManager->getExperience() << "\n";
    saveFile << "PlayerExpToNext:" << playerManager->getExperienceToNextLevel() << "\n";
    saveFile << "PlayerEnemiesDefeated:" << playerManager->getEnemiesDefeated() << "\n";

    if (playerEntity->hasComponent<HealthComponent>()) {
        auto& health = playerEntity->getComponent<HealthComponent>();
        saveFile << "PlayerHealth:" << health.getHealth() << "\n";
        saveFile << "PlayerMaxHealth:" << health.getMaxHealth() << "\n";
    } else { std::cerr << "Warning: Player missing HealthComponent during save!" << std::endl; }

    if (playerEntity->hasComponent<TransformComponent>()) {
        auto& transform = playerEntity->getComponent<TransformComponent>();
        saveFile << "PlayerPosX:" << transform.position.x << "\n";
        saveFile << "PlayerPosY:" << transform.position.y << "\n";
    } else { std::cerr << "Warning: Player missing TransformComponent during save!" << std::endl; }

    if (playerManager) {
         saveFile << "PlayerLifesteal:" << playerManager->getLifestealPercentage() << "\n";
    } else { std::cerr << "Warning: PlayerManager is null during save, cannot save Lifesteal!" << std::endl; }

    if (playerEntity->hasComponent<WeaponComponent>()) {
         auto& weapon = playerEntity->getComponent<WeaponComponent>();
         saveFile << "WeaponTag:" << weapon.tag << "\n";
         saveFile << "WeaponLevel:" << weapon.getLevel() << "\n";
         saveFile << "WeaponDamage:" << weapon.damage << "\n";
         saveFile << "WeaponFireRate:" << weapon.fireRate << "\n";
         saveFile << "WeaponProjSpeed:" << weapon.projectileSpeed << "\n";
         saveFile << "WeaponSpread:" << weapon.spreadAngle << "\n";
         saveFile << "WeaponProjCount:" << weapon.projectilesPerShot << "\n";
         saveFile << "WeaponProjSize:" << weapon.projectileSize << "\n";
         saveFile << "WeaponProjTexture:" << weapon.projectileTexture << "\n";
         saveFile << "WeaponPierce:" << weapon.projectilePierce << "\n";
         saveFile << "WeaponBurstCount:" << weapon.shotsPerBurst << "\n";
         saveFile << "WeaponBurstDelay:" << weapon.burstDelay << "\n";
    } else { std::cerr << "Warning: Player missing WeaponComponent during save!" << std::endl; }

    int spellIndex = 0;

    if (playerEntity) { 

        for (const auto& compPtr : playerEntity->getAllComponents()) {

            if (compPtr) { 
                 if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {

                     saveFile << "SpellIndex:" << spellIndex << "\n";
                     saveFile << "SpellTag:" << spellComp->tag << "\n"; 
                     saveFile << "SpellLevel:" << spellComp->getLevel() << "\n";
                     saveFile << "SpellDamage:" << spellComp->damage << "\n";
                     saveFile << "SpellCooldown:" << spellComp->cooldown << "\n";
                     saveFile << "SpellProjSpeed:" << spellComp->projectileSpeed << "\n";
                     saveFile << "SpellProjCount:" << spellComp->projectilesPerCast << "\n";
                     saveFile << "SpellProjSize:" << spellComp->projectileSize << "\n";
                     saveFile << "SpellProjTexture:" << spellComp->projectileTexture << "\n";

                     saveFile << "SpellTrajectory:" << static_cast<int>(spellComp->trajectoryMode) << "\n";
                     saveFile << "SpellSpiralGrowth:" << spellComp->spiralGrowthRate << "\n";
                     saveFile << "SpellPierce:" << spellComp->projectilePierce << "\n";

                     spellIndex++; 
                 } else {

                 }
            } else {

            }
        }
    } else {

    }

    saveFile << "TotalSpells:" << spellIndex << "\n"; 

    saveFile.close();

}

bool SaveLoadManager::loadGameState(const std::string& filename) {

    if (!gameInstance) {
         std::cerr << "Error loading: Game instance not available!" << std::endl;
         return false;
     }

    std::string loadFilename = filename;
    std::string saveDir = "saves";

    if (filename != "default.state" && filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        loadFilename = saveDir + "/" + filename;
    }

    std::ifstream loadFile(loadFilename);
    if (!loadFile.is_open()) {
        std::cerr << "Error: Could not open load file: " << loadFilename << std::endl;
        return false;
    }

    gameInstance->manager.refresh(); 
    for(auto& e : gameInstance->manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->destroy();
    for(auto& p : gameInstance->manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->destroy();
    for(auto& o : gameInstance->manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->destroy();
    gameInstance->manager.refresh(); 

    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

     if (!playerManager || !playerEntity) {
        std::cerr << "Error loading: Player or PlayerManager not initialized before load!" << std::endl;
        loadFile.close();
        return false;
     }

    std::string line, key, value;
    int spellLoadIndex = -1;
    std::vector<SpellComponent*> playerSpells;

    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            playerSpells.push_back(spellComp);
        }
    }

    while (std::getline(loadFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) {

            continue;
        }

        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);

        try {

            if (key == "PlayerName") {
                gameInstance->setPlayerName(value);
            }
            else if (key == "PlayerLevel") {
                if (playerManager) playerManager->setLevel(std::stoi(value));
                else { std::cerr << "Warning: Cannot load PlayerLevel, PlayerManager is null." << std::endl; }
            }
            else if (key == "PlayerExperience") {
                 if (playerManager) playerManager->setExperience(std::stoi(value));
                 else { std::cerr << "Warning: Cannot load PlayerExperience, PlayerManager is null." << std::endl; }
            }
            else if (key == "PlayerExpToNext") {
                 if (playerManager) playerManager->setExperienceToNextLevel(std::stoi(value));
                 else { std::cerr << "Warning: Cannot load PlayerExpToNext, PlayerManager is null." << std::endl; }
            }
            else if (key == "PlayerEnemiesDefeated") {
                 if (playerManager) playerManager->setEnemiesDefeated(std::stoi(value));
                 else { std::cerr << "Warning: Cannot load PlayerEnemiesDefeated, PlayerManager is null." << std::endl; }
            }
            else if (key == "PlayerHealth") {
                if (playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setHealth(std::stoi(value));
                else { std::cerr << "Warning: Cannot load PlayerHealth, HealthComponent missing." << std::endl; }
            }
            else if (key == "PlayerMaxHealth") {
                if (playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setMaxHealth(std::stoi(value));
                 else { std::cerr << "Warning: Cannot load PlayerMaxHealth, HealthComponent missing." << std::endl; }
            }
            else if (key == "PlayerPosX") {
                 if (playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.x = std::stof(value);
                 else { std::cerr << "Warning: Cannot load PlayerPosX, TransformComponent missing." << std::endl; }
            }
            else if (key == "PlayerPosY") {
                 if (playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.y = std::stof(value);
                 else { std::cerr << "Warning: Cannot load PlayerPosY, TransformComponent missing." << std::endl; }
            }

            else if (key == "PlayerLifesteal") {
                if (playerManager) playerManager->setLifestealPercentage(std::stof(value));
                else { std::cerr << "Warning: Cannot load PlayerLifesteal, PlayerManager is null." << std::endl; }
            }

            else if (key == "WeaponTag") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().tag = value;
                 else { std::cerr << "Warning: Cannot load WeaponTag, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponLevel") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().setLevel(std::stoi(value));
                 else { std::cerr << "Warning: Cannot load WeaponLevel, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponDamage") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().damage = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponDamage, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponFireRate") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().fireRate = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponFireRate, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponProjSpeed") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileSpeed = std::stof(value);
                 else { std::cerr << "Warning: Cannot load WeaponProjSpeed, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponSpread") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().spreadAngle = std::stof(value);
                 else { std::cerr << "Warning: Cannot load WeaponSpread, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponProjCount") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectilesPerShot = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponProjCount, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponProjSize") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileSize = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponProjSize, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponProjTexture") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileTexture = value;
                 else { std::cerr << "Warning: Cannot load WeaponProjTexture, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponPierce") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectilePierce = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponPierce, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponBurstCount") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().shotsPerBurst = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponBurstCount, WeaponComponent missing." << std::endl; }
            }
            else if (key == "WeaponBurstDelay") {
                 if (playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().burstDelay = std::stoi(value);
                 else { std::cerr << "Warning: Cannot load WeaponBurstDelay, WeaponComponent missing." << std::endl; }
            }

            else if (key == "SpellIndex") {
                 spellLoadIndex = std::stoi(value);

            }

            else if (spellLoadIndex >= 0 && static_cast<size_t>(spellLoadIndex) < playerSpells.size()) {
                SpellComponent* currentSpell = playerSpells[spellLoadIndex];
                if (!currentSpell) {
                     std::cerr << "Warning: Pre-fetched spell component at index " << spellLoadIndex << " is unexpectedly null. Skipping load for key '" << key << "'." << std::endl;
                     continue; 
                }

                if (key == "SpellTag") {

                    currentSpell->tag = value;
                }
                else if (key == "SpellLevel") {
                     currentSpell->setLevel(std::stoi(value));

                }
                else if (key == "SpellDamage") {
                     currentSpell->damage = std::stoi(value);
                }
                else if (key == "SpellCooldown") {
                     currentSpell->cooldown = std::stoi(value);
                }
                else if (key == "SpellProjSpeed") {
                     currentSpell->projectileSpeed = std::stof(value);
                }
                else if (key == "SpellProjCount") {
                     currentSpell->projectilesPerCast = std::stoi(value);

                }
                else if (key == "SpellProjSize") {
                     currentSpell->projectileSize = std::stoi(value);
                }
                else if (key == "SpellProjTexture") {
                     currentSpell->projectileTexture = value;
                }

                else if (key == "SpellTrajectory") {
                     currentSpell->trajectoryMode = static_cast<SpellTrajectory>(std::stoi(value));
                }
                else if (key == "SpellSpiralGrowth") {
                     currentSpell->spiralGrowthRate = std::stof(value);
                }
                else if (key == "SpellPierce") {
                     currentSpell->projectilePierce = std::stoi(value);
                }

            }
            else if (key == "TotalSpells") {

            }

        } catch (const std::invalid_argument& ia) {
            std::cerr << "[DEBUG Load] Load Error (Invalid Argument): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << ia.what() << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "[DEBUG Load] Load Error (Out of Range): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << oor.what() << std::endl;
        } catch (const std::exception& e) { 
            std::cerr << "[DEBUG Load] Load Error (General Exception): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << e.what() << std::endl;
        }
    } 

    loadFile.close();

    if (playerEntity->hasComponent<TransformComponent>()) {

        int currentWindowWidth = WINDOW_WIDTH, currentWindowHeight = WINDOW_HEIGHT; 
        if(Game::renderer) { SDL_GetRendererOutputSize(Game::renderer, &currentWindowWidth, &currentWindowHeight); }
        Game::camera.w = currentWindowWidth;
        Game::camera.h = currentWindowHeight;

        Game::camera.x = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.x - (Game::camera.w / 2.0f));
        Game::camera.y = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.y - (Game::camera.h / 2.0f));

        int mapPixelWidth = MAP_WIDTH * TILE_SIZE; 
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE; 
        Game::camera.x = std::max(0, std::min(Game::camera.x, mapPixelWidth - Game::camera.w));
        Game::camera.y = std::max(0, std::min(Game::camera.y, mapPixelHeight - Game::camera.h));

     }

     if (playerEntity->hasComponent<HealthComponent>()) {
          auto& health = playerEntity->getComponent<HealthComponent>();
          if (health.getHealth() > health.getMaxHealth()) {
               health.setHealth(health.getMaxHealth());

          }
     }

     gameInstance->updateSpawnPoolAndWeights();

    return true; 
}