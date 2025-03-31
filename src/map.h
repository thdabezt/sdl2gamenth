// Modify src/map.h

#pragma once
#include <string>
#include <vector>      // Include vector header
#include "Vector2D.h"   // Include Vector2D header
#include "ECS/ECS.h"

class Map {
public:
    Map(Manager& manager, std::string tID, int mscale, int tsize);
    ~Map();

    // Updated LoadMap signature
    void LoadMap(std::string path, int sizeX, int sizeY, int griWidth, std::vector<Vector2D>& outSpawnPoints);
    void AddTile(int srcX, int srcY, int xpos, int ypos);

private:
    Manager& manager_ref;
    std::string texID;
    int mapscale;
    int tileSize;
    int scaledSize;
};