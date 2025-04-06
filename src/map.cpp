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

Map::Map(Manager& manager, std::string tID, int mscale, int tsize)
 : manager_ref(manager), texID(std::move(tID)), mapscale(mscale), tileSize(tsize)
{
    scaledSize = tsize * mscale;
}

Map::~Map() { }

void Map::LoadMap(std::string path, int sizeX, int sizeY, int griWidth, std::vector<Vector2D>& outSpawnPoints) { 
    std::ifstream mapFile(path);
    if (!mapFile.is_open()) {
         std::cerr << "Error: Could not open map file: " << path << std::endl;
         return;
    }
    std::string line;
    std::vector<std::vector<int>> mapData;

    outSpawnPoints.clear();

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
             } catch (const std::exception& e) { 
                 std::cerr << "Map Load Error: Invalid value '" << value << "' (" << e.what() << "). Skipping row." << std::endl;
                 row.clear();
                 break;
             }
        }
        if (!row.empty()) {
            mapData.push_back(row);
        }
    }

    for (int y = 0; y < sizeY && y < static_cast<int>(mapData.size()); ++y) {
        for (int x = 0; x < sizeX && x < static_cast<int>(mapData[y].size()); ++x) {
            int tileCode = mapData[y][x];
            if (tileCode < 0) continue;

            int srcX = tileCode % griWidth;
            int srcY = tileCode / griWidth;
            AddTile(srcX * tileSize, srcY * tileSize, x * scaledSize, y * scaledSize);

             if (tileCode == 10|| tileCode == 11 || tileCode == 12 || tileCode == 13) {

                 outSpawnPoints.emplace_back(static_cast<float>(x * scaledSize), static_cast<float>(y * scaledSize));
             }
        }
    }

     std::cout << "Map loaded. Found " << outSpawnPoints.size() << " spawn points." << std::endl; 

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
            if (!mapData.empty() && currentY < static_cast<int>(mapData.size()) && currentX < static_cast<int>(mapData[currentY].size())) {
                if (mapData[currentY][currentX] == 6 || mapData[currentY][currentX] == 2 || mapData[currentY][currentX] == 4 || mapData[currentY][currentX] == 1 ||mapData[currentY][currentX] == 8 || mapData[currentY][currentX] == 3 || mapData[currentY][currentX] == 5) { 
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