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

// Add a vector to store the positions of '1' tiles
extern std::vector<Vector2D> spawnPoints;

extern Manager manager;

Map::Map(std::string tID, int mscale, int tsize) : texID(tID), mapscale(mscale), tileSize(tsize) {
    scaledSize = tsize * mscale;
}
Map::~Map() {
    
}
void Map::LoadMap(std::string path, int sizeX, int sizeY, int griWidth) {
    std::ifstream mapFile(path);
    std::string line;
    std::vector<std::vector<int>> mapData;
  
    while (std::getline(mapFile, line)) {
        std::istringstream ss(line);
        std::vector<int> row;
        std::string value;
  
        while (std::getline(ss, value, ',')) {
            row.push_back(std::stoi(value));
        }
  
        mapData.push_back(row);
    }
  
    for (int y = 0; y < sizeY; ++y) {
        for (int x = 0; x < sizeX; ++x) {
            int tileCode = mapData[y][x];
            int srcX = tileCode % griWidth;
            int srcY = tileCode / griWidth;
            AddTile(srcX * tileSize, srcY * tileSize, x * scaledSize, y * scaledSize); 
             // Store the position of '1' tiles
             if (tileCode == 1) {
                spawnPoints.emplace_back(x * scaledSize, y * scaledSize);
                std::cout << "Added spawn point at: (" << x * scaledSize << ", " << y * scaledSize << ")" << std::endl;
            }
        }
        
    }
    mapFile.clear();
    mapFile.seekg(0);

	// for (int y = 0; y < sizeY; y++)
	// {
	// 	for (int x = 0; x < sizeX; x++)
	// 	{
    //         char c;
	// 		mapFile.get(c);
	// 		mapFile.get(c);
    //         mapFile.ignore();
	// 	}
	// }
    
    for (int y = 0; y < sizeY; y++)
	{
		for (int x = 0; x < sizeX; x++)
		{
            char c;
			mapFile.get(c);
			if (c == '2')
			{
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, y * scaledSize, scaledSize);
				tcol.addGroup(Game::groupColliders);
			}
			mapFile.ignore();
		}
	}
    mapFile.close();
	
}
void Map::AddTile(int srcX, int srcY, int xpos, int ypos) {
    auto& tile(manager.addEntity());
    tile.addComponent<TileComponent>(srcX, srcY, xpos, ypos, tileSize, mapscale, texID);
    tile.addGroup(Game::groupMap);
}
