// Modify src/map.cpp

#include "map.h"
#include "game.h"
#include "constants.h"
#include "ECS/ECS.h"
#include "ECS/Components.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

// --- REMOVE extern spawnPoints ---
// extern std::vector<Vector2D> spawnPoints;


Map::Map(Manager& manager, std::string tID, int mscale, int tsize)
 : manager_ref(manager), texID(std::move(tID)), mapscale(mscale), tileSize(tsize)
{
    scaledSize = tsize * mscale;
}

Map::~Map() { }

// Updated LoadMap definition
void Map::LoadMap(std::string path, int sizeX, int sizeY, int griWidth, std::vector<Vector2D>& outSpawnPoints) { // Added reference param
    std::ifstream mapFile(path);
    if (!mapFile.is_open()) {
         std::cerr << "Error: Could not open map file: " << path << std::endl;
         return;
    }
    std::string line;
    std::vector<std::vector<int>> mapData;

    // Clear the output vector at the start
    outSpawnPoints.clear();

    // --- Load Tile Data (Pass 1) ---
    while (std::getline(mapFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::vector<int> row;
        std::string value;
        int tileCode;
        while (std::getline(ss, value, ',')) {
             try {
                 tileCode = std::stoi(value);
                 row.push_back(tileCode);
             } catch (const std::exception& e) { // Catch standard exceptions
                 std::cerr << "Map Load Error: Invalid value '" << value << "' (" << e.what() << "). Skipping row." << std::endl;
                 row.clear();
                 break;
             }
        }
        if (!row.empty()) {
            mapData.push_back(row);
        }
    }

    // Add Tiles and populate outSpawnPoints based on mapData
    for (int y = 0; y < sizeY && y < mapData.size(); ++y) {
        for (int x = 0; x < sizeX && x < mapData[y].size(); ++x) {
            int tileCode = mapData[y][x];
            if (tileCode < 0) continue;

            int srcX = tileCode % griWidth;
            int srcY = tileCode / griWidth;
            AddTile(srcX * tileSize, srcY * tileSize, x * scaledSize, y * scaledSize);

             // Store the position of '1' tiles (assuming '1' is designated spawn area)
             if (tileCode == 1) {
                 // --- UNCOMMENT and use the passed reference ---
                 outSpawnPoints.emplace_back(static_cast<float>(x * scaledSize), static_cast<float>(y * scaledSize));
             }
        }
    }

     std::cout << "Map loaded. Found " << outSpawnPoints.size() << " spawn points." << std::endl; // Debug output

    // --- Load Collider Data (Pass 2 - Assuming separate section or re-read) ---
    mapFile.clear();
    mapFile.seekg(0);
    int currentY = 0;
    while (std::getline(mapFile, line)) {
         if (line.empty() || line[0] == '#') continue;
         if (currentY >= sizeY) break;
        std::istringstream ss(line);
        std::string value;
        int currentX = 0;
         while (std::getline(ss, value, ',')) {
              if (currentX >= sizeX) break;
             if (!mapData.empty() && currentY < mapData.size() && currentX < mapData[currentY].size()) {
                  if (mapData[currentY][currentX] == 2) { // Check if this tile code was '2'
                      auto& tcol(manager_ref.addEntity());
                      tcol.addComponent<ColliderComponent>("terrain", currentX * scaledSize, currentY * scaledSize, scaledSize);
                      tcol.addGroup(Game::groupColliders);
                  }
             }
             currentX++;
         }
         currentY++;
    }

    mapFile.close();
}

void Map::AddTile(int srcX, int srcY, int xpos, int ypos) {
    auto& tile(manager_ref.addEntity());
    tile.addComponent<TileComponent>(srcX, srcY, xpos, ypos, tileSize, mapscale, texID);
    tile.addGroup(Game::groupMap);
}