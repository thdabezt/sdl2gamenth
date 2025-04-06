#pragma once

#include <string>

class Game;

class SaveLoadManager {
private:

    Game* gameInstance; 

    std::string getCurrentTimestamp(); 

public:

    SaveLoadManager(Game* game); 
    ~SaveLoadManager() = default; 

    void saveGameState(const std::string& filename = "");

    bool loadGameState(const std::string& filename);

}; 