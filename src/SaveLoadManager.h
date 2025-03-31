#pragma once

#include <string>
#include <vector> // Include vector for spell loading logic if needed later

// Forward declaration to avoid circular includes
class Game;
class SpellComponent; // Forward declare if used in function signatures

class SaveLoadManager {
private:
    Game* gameInstance; // Pointer to the main Game instance to access data

    // Helper to get current timestamp as string
    std::string getCurrentTimestamp();
    bool printLog = false;
public:
    // Constructor takes a pointer to the Game instance
    SaveLoadManager(Game* game);
    ~SaveLoadManager() = default; // Default destructor is likely sufficient

    // Save/Load Function Declarations
    void saveGameState(const std::string& filename = ""); // Default saves with timestamp
    bool loadGameState(const std::string& filename);

    // Optional: Add helper functions if needed, e.g., for listing saves
    // std::vector<std::string> listSaveFiles();
};