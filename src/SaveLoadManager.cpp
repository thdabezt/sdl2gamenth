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
    }
    if (playerEntity->hasComponent<TransformComponent>()) {
        auto& transform = playerEntity->getComponent<TransformComponent>();
        saveFile << "PlayerPosX:" << transform.position.x << "\n";
        saveFile << "PlayerPosY:" << transform.position.y << "\n";
    }

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
    }

    // --- Save Spell Data ---
    int spellIndex = 0;
    for (const auto& compPtr : playerEntity->getAllComponents()) {
        if (SpellComponent* spellComp = dynamic_cast<SpellComponent*>(compPtr.get())) {
            saveFile << "SpellIndex:" << spellIndex << "\n";
            saveFile << "SpellTag:" << spellComp->tag << "\n";
            saveFile << "SpellLevel:" << spellComp->getLevel() << "\n";
            saveFile << "SpellDamage:" << spellComp->damage << "\n";
            saveFile << "SpellCooldown:" << spellComp->cooldown << "\n"; // Save cooldown
            saveFile << "SpellProjSpeed:" << spellComp->projectileSpeed << "\n";
            saveFile << "SpellProjCount:" << spellComp->projectilesPerCast << "\n";
            saveFile << "SpellProjSize:" << spellComp->projectileSize << "\n";
            saveFile << "SpellProjTexture:" << spellComp->projectileTexture << "\n";
            saveFile << "SpellDuration:" << spellComp->duration << "\n";
            saveFile << "SpellTrajectory:" << static_cast<int>(spellComp->trajectoryMode) << "\n";
            saveFile << "SpellSpiralGrowth:" << spellComp->spiralGrowthRate << "\n";
            saveFile << "SpellPierce:" << spellComp->projectilePierce << "\n";
            spellIndex++;
        }
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

    // Adjust path for loading non-default saves
    if (filename != "default.state" && filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
        loadFilename = saveDir + "/" + filename;
    }

    std::ifstream loadFile(loadFilename);
    if (!loadFile.is_open()) {
        std::cerr << "Error: Could not open load file: " << loadFilename << std::endl;
        return false;
    }

    // Reset state BEFORE loading
    gameInstance->manager.refresh(); // Clear entities first
    for(auto& e : gameInstance->manager.getGroup(Game::groupEnemies)) if(e && e->isActive()) e->destroy();
    for(auto& p : gameInstance->manager.getGroup(Game::groupProjectiles)) if(p && p->isActive()) p->destroy();
    for(auto& o : gameInstance->manager.getGroup(Game::groupExpOrbs)) if(o && o->isActive()) o->destroy();
    gameInstance->manager.refresh(); // Process destruction

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

    while (std::getline(loadFile, line)) {
        std::size_t separatorPos = line.find(':');
        if (separatorPos == std::string::npos) continue;

        key = line.substr(0, separatorPos);
        value = line.substr(separatorPos + 1);

        try {
            // Player Stats
            if (key == "PlayerName") gameInstance->setPlayerName(value);
            else if (key == "PlayerLevel") playerManager->setLevel(std::stoi(value));
            else if (key == "PlayerExperience") playerManager->setExperience(std::stoi(value));
            else if (key == "PlayerExpToNext") playerManager->setExperienceToNextLevel(std::stoi(value));
            else if (key == "PlayerEnemiesDefeated") playerManager->setEnemiesDefeated(std::stoi(value));
            else if (key == "PlayerHealth" && playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setHealth(std::stoi(value));
            else if (key == "PlayerMaxHealth" && playerEntity->hasComponent<HealthComponent>()) playerEntity->getComponent<HealthComponent>().setMaxHealth(std::stoi(value));
            else if (key == "PlayerPosX" && playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.x = std::stof(value);
            else if (key == "PlayerPosY" && playerEntity->hasComponent<TransformComponent>()) playerEntity->getComponent<TransformComponent>().position.y = std::stof(value);

            // Weapon Stats
            else if (key == "WeaponTag" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().tag = value;
            else if (key == "WeaponLevel" && playerEntity->hasComponent<WeaponComponent>()) playerEntity->getComponent<WeaponComponent>().setLevel(std::stoi(value));
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

            // Spell Stats
            else if (key == "SpellIndex") { spellLoadIndex = std::stoi(value); }
            else if (spellLoadIndex >= 0 && spellLoadIndex < playerSpells.size()) {
                SpellComponent* currentSpell = playerSpells[spellLoadIndex];
                if (!currentSpell) continue; // Skip if somehow the spell pointer is invalid

                if (key == "SpellTag") currentSpell->tag = value;
                else if (key == "SpellLevel") currentSpell->setLevel(std::stoi(value));
                else if (key == "SpellDamage") currentSpell->damage = std::stoi(value);
                else if (key == "SpellCooldown") currentSpell->cooldown = std::stoi(value); // Load cooldown
                else if (key == "SpellProjSpeed") currentSpell->projectileSpeed = std::stof(value);
                else if (key == "SpellProjCount") currentSpell->projectilesPerCast = std::stoi(value);
                else if (key == "SpellProjSize") currentSpell->projectileSize = std::stoi(value);
                else if (key == "SpellProjTexture") currentSpell->projectileTexture = value;
                else if (key == "SpellDuration") currentSpell->duration = std::stoi(value);
                else if (key == "SpellTrajectory") currentSpell->trajectoryMode = static_cast<SpellTrajectory>(std::stoi(value));
                else if (key == "SpellSpiralGrowth") currentSpell->spiralGrowthRate = std::stof(value);
                else if (key == "SpellPierce") currentSpell->projectilePierce = std::stoi(value);
             }

        } catch (const std::invalid_argument& ia) {
            std::cerr << "Load Error: Invalid number format for key '" << key << "' with value '" << value << "'. Skipping. Error: " << ia.what() << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Load Error: Number out of range for key '" << key << "' with value '" << value << "'. Skipping. Error: " << oor.what() << std::endl;
        } catch (const std::exception& e) { // Catch other potential exceptions
            std::cerr << "Load Error: Exception parsing key '" << key << "' with value '" << value << "'. Skipping. Error: " << e.what() << std::endl;
        }
    } // End while getline

    loadFile.close();

    // --- Post-Load Adjustments ---
    if (playerEntity->hasComponent<TransformComponent>()) {
        // Update camera position based on loaded player position
        gameInstance->camera.x = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.x - (Game::camera.w / 2.0f));
        gameInstance->camera.y = static_cast<int>(playerEntity->getComponent<TransformComponent>().position.y - (Game::camera.h / 2.0f));
        // Clamp camera after loading
        int mapPixelWidth = MAP_WIDTH * TILE_SIZE;
        int mapPixelHeight = MAP_HEIGHT * TILE_SIZE;
        gameInstance->camera.x = std::max(0, std::min(gameInstance->camera.x, mapPixelWidth - gameInstance->camera.w));
        gameInstance->camera.y = std::max(0, std::min(gameInstance->camera.y, mapPixelHeight - gameInstance->camera.h));
     }
    // Ensure health doesn't exceed max health after loading potential individual values
     if (playerEntity->hasComponent<HealthComponent>()) {
          auto& health = playerEntity->getComponent<HealthComponent>();
          if (health.getHealth() > health.getMaxHealth()) {
               health.setHealth(health.getMaxHealth());
          }
     }

     gameInstance->updateSpawnPoolAndWeights();
    return true; // Indicate success
}