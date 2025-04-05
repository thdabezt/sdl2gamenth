// --- Includes ---
#include "SaveLoadManager.h"
#include "game.h"          // Access Game members & components
#include "ECS/Components.h"   // Access specific components (Health, Transform, etc.)
#include "ECS/Player.h"       // Access Player class methods/members
#include <fstream>
#include <sstream>
#include <filesystem>         // C++17: For directory operations
#include <chrono>             // For obtaining current time
#include <iomanip>            // For formatting time
#include <ctime>              // For time conversion
#include <vector>
#include <algorithm>          // For std::max
#include <SDL_mixer.h>        // For loading volume settings
#include <iostream>           // For debug logging

// --- Constructor ---

SaveLoadManager::SaveLoadManager(Game* game) : gameInstance(game) {
    if (!gameInstance) {
        // Critical error if the Game instance is not provided
        throw std::runtime_error("SaveLoadManager requires a valid Game instance pointer.");
    }
}

// --- Private Methods ---

// Generates a timestamp string in DDMMYYYY-HHMMSS format.
std::string SaveLoadManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c); // Use secure localtime_s on Windows

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%d%m%Y-%H%M%S");
    return oss.str();
}

// --- Public Methods ---

void SaveLoadManager::saveGameState(const std::string& filename) {
    // std::cout << "[DEBUG Save] Attempting to save game state to: " << (filename.empty() ? "<timestamp>" : filename) << std::endl; // Optional Debug Log

    if (!gameInstance || !gameInstance->playerManager || !gameInstance->playerEntity) {
         std::cerr << "Error saving: Game instance or player not initialized!" << std::endl;
         return;
    }

    std::string saveFilename = filename;
    std::string saveDir = "saves";

    // Ensure the saves directory exists
    try {
        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directory(saveDir);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating save directory '" << saveDir << "': " << e.what() << std::endl;
        return;
    }

    // Determine final save path
    if (saveFilename.empty()) {
        saveFilename = saveDir + "/" + getCurrentTimestamp() + ".state";
    } else if (filename == "default.state") {
         saveFilename = "default.state"; // Keep default.state in the root
    } else if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        saveFilename = saveDir + "/" + filename; // Place named saves in saves directory
    }
    // else: assume filename includes a full path

    // std::cout << "[DEBUG Save] Final save path: " << saveFilename << std::endl; // Optional Debug Log

    std::ofstream saveFile(saveFilename);
    if (!saveFile.is_open()) {
        std::cerr << "Error: Could not open save file: " << saveFilename << std::endl;
        return;
    }

    // Access game data through gameInstance pointer
    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

    // --- Save Player Data ---
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

    // *** Save PlayerLifesteal (Corrected) ***
    if (playerManager) {
         saveFile << "PlayerLifesteal:" << playerManager->getLifestealPercentage() << "\n";
    } else { std::cerr << "Warning: PlayerManager is null during save, cannot save Lifesteal!" << std::endl; }

    // --- Save Weapon Data ---
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

    // --- Save Spell Data ---
    int spellIndex = 0;
    // std::cout << "[DEBUG Save] Starting to find Spell Components to save..." << std::endl;
    if (playerEntity) { // Check if playerEntity exists
        // std::cout << "[DEBUG Save] Total components on Player: " << playerEntity->getAllComponents().size() << std::endl;
        for (const auto& compPtr : playerEntity->getAllComponents()) {
            // std::cout << "[DEBUG Save]  -> Checking a component..." << std::endl; // Log each component check
            if (compPtr) { // Check component pointer is not null
                 if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
                    //  std::cout << "[DEBUG Save]  ==> FOUND SpellComponent! Index: " << spellIndex
                    //            << ", Tag: '" << spellComp->tag << "'" // Print the found tag
                    //            << ", Level: " << spellComp->getLevel() << std::endl;

                     // --- Save Data Section ---
                     saveFile << "SpellIndex:" << spellIndex << "\n";
                     saveFile << "SpellTag:" << spellComp->tag << "\n"; // Make sure tag is correct before saving!
                     saveFile << "SpellLevel:" << spellComp->getLevel() << "\n";
                     saveFile << "SpellDamage:" << spellComp->damage << "\n";
                     saveFile << "SpellCooldown:" << spellComp->cooldown << "\n";
                     saveFile << "SpellProjSpeed:" << spellComp->projectileSpeed << "\n";
                     saveFile << "SpellProjCount:" << spellComp->projectilesPerCast << "\n";
                     saveFile << "SpellProjSize:" << spellComp->projectileSize << "\n";
                     saveFile << "SpellProjTexture:" << spellComp->projectileTexture << "\n";
                     // *** SpellDuration saving removed for consistency ***
                     // saveFile << "SpellDuration:" << spellComp->duration << "\n";
                     saveFile << "SpellTrajectory:" << static_cast<int>(spellComp->trajectoryMode) << "\n";
                     saveFile << "SpellSpiralGrowth:" << spellComp->spiralGrowthRate << "\n";
                     saveFile << "SpellPierce:" << spellComp->projectilePierce << "\n";
                     // --- End Save Data Section ---

                     spellIndex++; // Increment index only after successfully finding and saving a spell
                 } else {
                     // Optional: Log components that are not SpellComponents
                     // std::cout << "[DEBUG Save]  -> This component is not a SpellComponent." << std::endl;
                 }
            } else {
                 // std::cout << "[DEBUG Save]  -> Encountered a null component in the list!" << std::endl;
            }
        }
    } else {
         // std::cerr << "[DEBUG Save] Error: playerEntity is null when preparing to save spells!" << std::endl;
    }
    // std::cout << "[DEBUG Save] Finished searching. Total Spells found: " << spellIndex << std::endl;
    saveFile << "TotalSpells:" << spellIndex << "\n"; // Save the actual count found

    saveFile.close();
    // std::cout << "[DEBUG] Game state saved successfully to: " << saveFilename << std::endl; // Optional Debug Log
}


bool SaveLoadManager::loadGameState(const std::string& filename) {
    // std::cout << "[DEBUG Load] Attempting to load game state from: " << filename << std::endl; // Optional Debug Log

    if (!gameInstance) {
         std::cerr << "Error loading: Game instance not available!" << std::endl;
         return false;
     }

    std::string loadFilename = filename;
    std::string saveDir = "saves";

    // Adjust path for loading non-default saves
    if (filename != "default.state" && filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        loadFilename = saveDir + "/" + filename;
    }

    // std::cout << "[DEBUG Load] Final load path: " << loadFilename << std::endl; // Optional Debug Log

    std::ifstream loadFile(loadFilename);
    if (!loadFile.is_open()) {
        std::cerr << "Error: Could not open load file: " << loadFilename << std::endl;
        return false;
    }

    // std::cout << "[DEBUG Load] Resetting current game state before loading..." << std::endl; // Optional Debug Log
    // Reset state BEFORE loading
    gameInstance->manager.refresh(); // Clear entities first
    for(auto& e : gameInstance->manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->destroy();
    for(auto& p : gameInstance->manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->destroy();
    for(auto& o : gameInstance->manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->destroy();
    gameInstance->manager.refresh(); // Process destruction
    // std::cout << "[DEBUG Load] State reset complete." << std::endl; // Optional Debug Log

    Player* playerManager = gameInstance->playerManager;
    Entity* playerEntity = gameInstance->playerEntity;

     if (!playerManager || !playerEntity) {
        std::cerr << "Error loading: Player or PlayerManager not initialized before load!" << std::endl;
        loadFile.close();
        return false;
     }

    // --- Load Data ---
    std::string line, key, value;
    int spellLoadIndex = -1;
    std::vector<SpellComponent*> playerSpells;

    // Pre-fetch spell components for easier assignment
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            playerSpells.push_back(spellComp);
        }
    }
    // std::cout << "[DEBUG Load] Pre-fetched " << playerSpells.size() << " existing spell components." << std::endl; // Optional Debug Log

    // std::cout << "[DEBUG Load] Starting line-by-line file read..." << std::endl; // Optional Debug Log
    while (std::getline(loadFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) {
            // std::cerr << "[DEBUG Load] Skipping invalid line (no ':'): " << line << std::endl; // Optional Debug Log
            continue;
        }

        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);

        // Optional Debug Log for each key-value pair
        // std::cout << "[DEBUG Load] Loading Key: '" << key << "', Value: '" << value << "'" << std::endl;

        try {
            // Player Stats
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
            // *** Load PlayerLifesteal ***
            else if (key == "PlayerLifesteal") {
                if (playerManager) playerManager->setLifestealPercentage(std::stof(value));
                else { std::cerr << "Warning: Cannot load PlayerLifesteal, PlayerManager is null." << std::endl; }
            }


            // Weapon Stats
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

            // Spell Stats
            else if (key == "SpellIndex") {
                 spellLoadIndex = std::stoi(value);
                 // Optional Debug Log for index boundary check
                 // if (spellLoadIndex < 0 || static_cast<size_t>(spellLoadIndex) >= playerSpells.size()) {
                 //     std::cerr << "[DEBUG Load] Warning: SpellIndex " << spellLoadIndex << " is out of bounds for pre-fetched spells (count=" << playerSpells.size() << ")." << std::endl;
                 // }
            }
            // Check if spellLoadIndex is valid before attempting to access playerSpells
            else if (spellLoadIndex >= 0 && static_cast<size_t>(spellLoadIndex) < playerSpells.size()) {
                SpellComponent* currentSpell = playerSpells[spellLoadIndex];
                if (!currentSpell) {
                     std::cerr << "Warning: Pre-fetched spell component at index " << spellLoadIndex << " is unexpectedly null. Skipping load for key '" << key << "'." << std::endl;
                     continue; // Skip to next line
                }

                if (key == "SpellTag") {
                    // std::cout << "[DEBUG Load] Setting Spell " << spellLoadIndex << " Tag to: '" << value << "'" << std::endl; // Optional Debug Log
                    currentSpell->tag = value;
                }
                else if (key == "SpellLevel") {
                     currentSpell->setLevel(std::stoi(value));
                     // std::cout << "[DEBUG Load] Set Spell " << spellLoadIndex << " Level to: " << currentSpell->getLevel() << std::endl; // Optional Debug Log
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
                     // std::cout << "[DEBUG Load] Set Spell " << spellLoadIndex << " ProjCount to: " << currentSpell->projectilesPerCast << std::endl; // Optional Debug Log
                }
                else if (key == "SpellProjSize") {
                     currentSpell->projectileSize = std::stoi(value);
                }
                else if (key == "SpellProjTexture") {
                     currentSpell->projectileTexture = value;
                }
                // *** SpellDuration is not loaded ***
                // else if (key == "SpellDuration") { /* Do nothing */ }
                else if (key == "SpellTrajectory") {
                     currentSpell->trajectoryMode = static_cast<SpellTrajectory>(std::stoi(value));
                }
                else if (key == "SpellSpiralGrowth") {
                     currentSpell->spiralGrowthRate = std::stof(value);
                }
                else if (key == "SpellPierce") {
                     currentSpell->projectilePierce = std::stoi(value);
                }
                // Add else if for other spell keys if needed
            }
            else if (key == "TotalSpells") {
                 // Optional check: Compare std::stoi(value) with playerSpells.size()
                 // std::cout << "[DEBUG Load] TotalSpells in file: " << value << std::endl; // Optional Debug Log
            }
            // Add else for unrecognized keys
            // else {
            //     // Only log if it's not an out-of-bounds SpellIndex related key
            //     if (key != "SpellTag" && key != "SpellLevel" && key != "SpellDamage" /* etc... */) {
            //          std::cerr << "[DEBUG Load] Warning: Unrecognized key in save file: '" << key << "'" << std::endl;
            //     }
            // }

        } catch (const std::invalid_argument& ia) {
            std::cerr << "[DEBUG Load] Load Error (Invalid Argument): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << ia.what() << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "[DEBUG Load] Load Error (Out of Range): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << oor.what() << std::endl;
        } catch (const std::exception& e) { // Catch other potential exceptions
            std::cerr << "[DEBUG Load] Load Error (General Exception): Key='" << key << "', Value='" << value << "'. Skipping. Error: " << e.what() << std::endl;
        }
    } // End while getline

    loadFile.close();

    // --- Post-Load Adjustments ---
    // std::cout << "[DEBUG Load] Performing post-load adjustments..." << std::endl; // Optional Debug Log
    if (playerEntity->hasComponent<TransformComponent>()) {
        // Update camera position based on loaded player position
        // Ensure camera dimensions are current before calculating offsets
        int currentWindowWidth = WINDOW_WIDTH, currentWindowHeight = WINDOW_HEIGHT; // Use constants as fallback
        if(Game::renderer) { SDL_GetRendererOutputSize(Game::renderer, &currentWindowWidth, &currentWindowHeight); }
        Game::camera.w = currentWindowWidth;
        Game::camera.h = currentWindowHeight;

        Game::camera.x = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.x - (Game::camera.w / 2.0f));
        Game::camera.y = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.y - (Game::camera.h / 2.0f));

        // Clamp camera after loading
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE; // Assuming TILE_SIZE is accessible or defined globally/passed
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE; // Assuming MAP_HEIGHT is accessible
        Game::camera.x = std::max(0, std::min(Game::camera.x, mapPixelWidth - Game::camera.w));
        Game::camera.y = std::max(0, std::min(Game::camera.y, mapPixelHeight - Game::camera.h));
        // std::cout << "[DEBUG Load] Camera clamped to: x=" << Game::camera.x << ", y=" << Game::camera.y << ", w=" << Game::camera.w << ", h=" << Game::camera.h << std::endl; // Optional Debug Log
     }

    // Ensure health doesn't exceed max health after loading potential individual values
     if (playerEntity->hasComponent<HealthComponent>()) {
          auto& health = playerEntity->getComponent<HealthComponent>();
          if (health.getHealth() > health.getMaxHealth()) {
               health.setHealth(health.getMaxHealth());
               // std::cout << "[DEBUG Load] Player health clamped to max health (" << health.getMaxHealth() << ")." << std::endl; // Optional Debug Log
          }
     }

     // std::cout << "[DEBUG Load] Updating spawn pool weights..." << std::endl; // Optional Debug Log
     gameInstance->updateSpawnPoolAndWeights();

    // std::cout << "[DEBUG Load] Game state loaded successfully." << std::endl; // Optional Debug Log
    return true; // Indicate success
}