#pragma once

#include <string>
#include <vector> 
#include "Vector2D.h" 
#include "ECS/ECS.h"

class Map {
public:
    Map(Manager& manager, std::string tID, int mscale, int tsize);
    ~Map();

    void LoadMap(std::string path, int sizeX, int sizeY, int griWidth, std::vector<Vector2D>& outSpawnPoints);
    void AddTile(int srcX, int srcY, int xpos, int ypos);

private:
    Manager& manager_ref;
    std::string texID;
    int mapscale;
    int tileSize;
    int scaledSize;
};