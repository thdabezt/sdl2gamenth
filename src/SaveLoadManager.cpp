#include "SaveLoadManager.h"
#include "game.h"          // Include game.h for Game members & components
#include "ECS/Components.h"   // Include for component access (Health, Transform, etc.)
#include "ECS/Player.h"       // Include for Player class
#include <fstream>
#include <sstream>
#include <filesystem>         // Required for creating directories (C++17)
#include <chrono>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>          // For std::max in loading potentially
#include <SDL_mixer.h>        // For loading volume settings

// Constructor Implementation
SaveLoadManager::SaveLoadManager(Game* game) : gameInstance(game) {
    if (!gameInstance) {
        // Handle error: Game instance cannot be null
        throw std::runtime_error("SaveLoadManager requires a valid Game instance pointer.");
    }
}

// --- getCurrentTimestamp Implementation ---
std::string SaveLoadManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c); // Use localtime for local time zone

    std::ostringstream oss;
    // Format: DDMMYYYY-HHMMSS (e.g., 30032025-233722)
    oss << std::put_time(&now_tm, "%d%m%Y-%H%M%S");
    return oss.str();
}

// --- saveGameState Implementation ---
void SaveLoadManager::saveGameState(const std::string& filename) {
    if (!gameInstance || !gameInstance->playerManager || !gameInstance->playerEntity) {
         std::cerr << "Error saving: Game instance or player not initialized!" << std::endl;
         return;
    }

    std::string saveFilename = filename;
    std::string saveDir = "saves"; // Define save directory

    // Ensure the saves directory exists
    if (!std::filesystem::exists(saveDir)) {
        try {
            std::filesystem::create_directory(saveDir);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating save directory '" << saveDir << "': " << e.what() << std::endl;
            return; // Cannot proceed without save directory
        }
    }

    if (saveFilename.empty()) {
        saveFilename = saveDir + "/" + getCurrentTimestamp() + ".state";
    } else if (filename == "default.state") {
         saveFilename = "default.state"; // Keep default.state in the root directory
    } else if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        // If a specific filename without path is given, save it in the saves directory
        saveFilename = saveDir + "/" + filename;
    }
    // else: assume filename includes path if provided by user (less common)

    std::ofstream saveFile(saveFilename);
    if (!saveFile.is_open()) {
        std::cerr << "Error: Could not open save file: " << saveFilename << std::endl;
        return;
    }

    std::cout << "Saving game state to: " << saveFilename << std::endl;

    // Access game data through gameInstance pointer
    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

    // --- Save Player Stats ---
    saveFile << "PlayerName:" << gameInstance->getPlayerName() << std::endl;
    saveFile << "PlayerLevel:" << playerManager->getLevel() << std::endl;
    saveFile << "PlayerExperience:" << playerManager->getExperience() << std::endl;
    saveFile << "PlayerExpToNext:" << playerManager->getExperienceToNextLevel() << std::endl;
    saveFile << "PlayerEnemiesDefeated:" << playerManager->getEnemiesDefeated() << std::endl;

    if (playerEntity->hasComponent<HealthComponent>()) {
        auto& health = playerEntity->getComponent<HealthComponent>();
        saveFile << "PlayerHealth:" << health.getHealth() << std::endl;
        saveFile << "PlayerMaxHealth:" << health.getMaxHealth() << std::endl;
    }

    if (playerEntity->hasComponent<TransformComponent>()) {
        auto& transform = playerEntity->getComponent<TransformComponent>();
        saveFile << "PlayerPosX:" << transform.position.x << std::endl;
        saveFile << "PlayerPosY:" << transform.position.y << std::endl;
    }

    // --- Save Weapon Stats ---
    if (playerEntity->hasComponent<WeaponComponent>()) {
         auto& weapon = playerEntity->getComponent<WeaponComponent>();
         saveFile << "WeaponTag:" << weapon.tag << std::endl;
         saveFile << "WeaponLevel:" << weapon.getLevel() << std::endl; // --- SAVE LEVEL ---
         saveFile << "WeaponDamage:" << weapon.damage << std::endl;
         saveFile << "WeaponFireRate:" << weapon.fireRate << std::endl;
         saveFile << "WeaponProjSpeed:" << weapon.projectileSpeed << std::endl;
         saveFile << "WeaponSpread:" << weapon.spreadAngle << std::endl;
         saveFile << "WeaponProjCount:" << weapon.projectilesPerShot << std::endl;
         saveFile << "WeaponProjSize:" << weapon.projectileSize << std::endl;
         saveFile << "WeaponProjTexture:" << weapon.projectileTexture << std::endl;
         saveFile << "WeaponPierce:" << weapon.projectilePierce << std::endl;
         saveFile << "WeaponBurstCount:" << weapon.shotsPerBurst << std::endl;
         saveFile << "WeaponBurstDelay:" << weapon.burstDelay << std::endl;
    }

    // --- Save Spell Stats ---
    int spellIndex = 0;
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            saveFile << "SpellIndex:" << spellIndex << std::endl;
            saveFile << "SpellTag:" << spellComp->tag << std::endl;
            saveFile << "SpellLevel:" << spellComp->getLevel() << std::endl; // --- SAVE LEVEL ---
            saveFile << "SpellDamage:" << spellComp->damage << std::endl;
            saveFile << "SpellProjSpeed:" << spellComp->projectileSpeed << std::endl;
            saveFile << "SpellProjCount:" << spellComp->projectilesPerCast << std::endl;
            saveFile << "SpellProjSize:" << spellComp->projectileSize << std::endl;
            saveFile << "SpellProjTexture:" << spellComp->projectileTexture << std::endl;
            saveFile << "SpellDuration:" << spellComp->duration << std::endl;
            saveFile << "SpellTrajectory:" << static_cast<int>(spellComp->trajectoryMode) << std::endl;
            saveFile << "SpellSpiralGrowth:" << spellComp->spiralGrowthRate << std::endl;
            saveFile << "SpellPierce:" << spellComp->projectilePierce << std::endl;
            spellIndex++;
        }
    }
    saveFile << "TotalSpells:" << spellIndex << std::endl;

    // --- Save Game Settings ---
     saveFile << "MusicVolume:" << gameInstance->musicVolume << std::endl;
     saveFile << "SfxVolume:" << gameInstance->sfxVolume << std::endl;

    saveFile.close();
    std::cout << "Game state saved successfully." << std::endl;
}

// --- loadGameState Implementation ---
bool SaveLoadManager::loadGameState(const std::string& filename) {
     if (!gameInstance) {
         std::cerr << "Error loading: Game instance not available!" << std::endl;
         return false;
     }

    std::string loadFilename = filename;
    std::string saveDir = "saves";

    // Adjust path for loading if needed
    if (filename != "default.state" && filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        loadFilename = saveDir + "/" + filename;
    }
     // else: assume default.state is in root or filename includes path

    std::ifstream loadFile(loadFilename);
    if (!loadFile.is_open()) {
        std::cerr << "Error: Could not open load file: " << loadFilename << std::endl;
        return false; // Indicate failure
    }

    std::cout << "Loading game state from: " << loadFilename << std::endl;

    // --- Reset necessary game state before loading ---
    // Access manager through gameInstance
    gameInstance->manager.refresh();
    for(auto& e : gameInstance->manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->destroy();
    for(auto& p : gameInstance->manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->destroy();
    for(auto& o : gameInstance->manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->destroy();
    gameInstance->manager.refresh();

    // Access player data through gameInstance
    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

     if (!playerManager || !playerEntity) {
         std::cerr << "Error loading: Player or PlayerManager not initialized before load!" << std::endl;
         loadFile.close();
         return false;
     }


    // --- Load Stats ---
    std::string line;
    std::string key;
    std::string value;
    int spellLoadIndex = -1;
    std::vector<SpellComponent*> playerSpells;

    // Pre-fetch spell components
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            playerSpells.push_back(spellComp);
        }
    }

    while (std::getline(loadFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;

        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);

        try {
            // --- Load Player Stats ---
            if (key == "PlayerName") gameInstance->setPlayerName(value); 
            else if (key == "PlayerLevel") playerManager->setLevel(std::stoi(value));
            else if (key == "PlayerExperience") playerManager->setExperience(std::stoi(value));
            else if (key == "PlayerExpToNext") playerManager->setExperienceToNextLevel(std::stoi(value));
            else if (key == "PlayerEnemiesDefeated") playerManager->setEnemiesDefeated(std::stoi(value));
            else if (key == "PlayerHealth" && playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setHealth(std::stoi(value));
            else if (key == "PlayerMaxHealth" && playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setMaxHealth(std::stoi(value));
            else if (key == "PlayerPosX" && playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.x = std::stof(value);
            else if (key == "PlayerPosY" && playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.y = std::stof(value);

            // --- Load Weapon Stats ---
            else if (key == "WeaponTag" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().tag = value;
            else if (key == "WeaponLevel" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().setLevel(std::stoi(value)); // --- LOAD LEVEL ---
            else if (key == "WeaponDamage" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().damage = std::stoi(value);
            else if (key == "WeaponFireRate" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().fireRate = std::stoi(value);
            else if (key == "WeaponProjSpeed" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileSpeed = std::stof(value);
            else if (key == "WeaponSpread" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().spreadAngle = std::stof(value);
            else if (key == "WeaponProjCount" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectilesPerShot = std::stoi(value);
            else if (key == "WeaponProjSize" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileSize = std::stoi(value);
            else if (key == "WeaponProjTexture" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectileTexture = value;
            else if (key == "WeaponPierce" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().projectilePierce = std::stoi(value);
            else if (key == "WeaponBurstCount" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().shotsPerBurst = std::stoi(value);
            else if (key == "WeaponBurstDelay" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().burstDelay = std::stoi(value);

            // --- Load Spell Stats ---
             else if (key == "SpellIndex") { spellLoadIndex = std::stoi(value); }
             else if (spellLoadIndex >= 0 && spellLoadIndex < playerSpells.size()) {
                 SpellComponent* currentSpell = playerSpells[spellLoadIndex];
                 if (!currentSpell) continue;

                 if (key == "SpellTag") currentSpell->tag = value;
                 else if (key == "SpellLevel") currentSpell->setLevel(std::stoi(value)); // --- LOAD LEVEL ---
                 else if (key == "SpellDamage") currentSpell->damage = std::stoi(value);
                 else if (key == "SpellCooldown") currentSpell->cooldown = std::stoi(value);
                 else if (key == "SpellProjSpeed") currentSpell->projectileSpeed = std::stof(value);
                 else if (key == "SpellProjCount") currentSpell->projectilesPerCast = std::stoi(value);
                 else if (key == "SpellProjSize") currentSpell->projectileSize = std::stoi(value);
                 else if (key == "SpellProjTexture") currentSpell->projectileTexture = value;
                 else if (key == "SpellTrajectory") currentSpell->trajectoryMode = static_cast<SpellTrajectory>(std::stoi(value));
                 else if (key == "SpellSpiralGrowth") currentSpell->spiralGrowthRate = std::stof(value);
                 else if (key == "SpellPierce") currentSpell->projectilePierce = std::stoi(value);
             }
             else if (key == "TotalSpells") { /* Optional Check */ }


            // --- Load Game Settings ---
            else if (key == "MusicVolume") {
                 gameInstance->musicVolume = std::stoi(value);
                 Mix_VolumeMusic(gameInstance->musicVolume); // Apply loaded volume
            }
            else if (key == "SfxVolume") {
                gameInstance->sfxVolume = std::stoi(value);
                Mix_Volume(-1, gameInstance->sfxVolume); // Apply loaded volume
            }


        } catch (const std::invalid_argument& ia) {
            std::cerr << "Load Error: Invalid number format for key '" << key << "' with value '" << value << "'. Skipping." << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Load Error: Number out of range for key '" << key << "' with value '" << value << "'. Skipping." << std::endl;
        }
    }

    loadFile.close();

    // --- Post-Load Adjustments ---
    if (playerEntity->hasComponent<TransformComponent>()) {
        gameInstance->camera.x = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.x - (WINDOW_WIDTH / 2.0f));
        gameInstance->camera.y = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.y - (WINDOW_HEIGHT / 2.0f));
        // Clamp camera after loading
        if (gameInstance->camera.x < 0) gameInstance->camera.x = 0;
        if (gameInstance->camera.y < 0) gameInstance->camera.y = 0;
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE;
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE;
        if (gameInstance->camera.x > mapPixelWidth - gameInstance->camera.w) gameInstance->camera.x = mapPixelWidth - gameInstance->camera.w;
        if (gameInstance->camera.y > mapPixelHeight - gameInstance->camera.h) gameInstance->camera.y = mapPixelHeight - gameInstance->camera.h;
     }

    std::cout << "Game state loaded successfully." << std::endl;
    return true; // Indicate success
}