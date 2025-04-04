#pragma once

// --- Includes ---
#include <string>


// --- Forward Declarations ---
class Game;

// --- Class Definition ---

class SaveLoadManager {
private:
    // --- Private Members ---
    Game* gameInstance; // Pointer to the main Game instance to access data

    // --- Private Methods ---
    std::string getCurrentTimestamp(); // Helper to get timestamp string

public:
    // --- Constructor & Destructor ---
    SaveLoadManager(Game* game); // Constructor takes a pointer to the Game instance
    ~SaveLoadManager() = default; // Default destructor is sufficient

    // --- Public Methods ---
    // Saves the current game state. If filename is empty, uses a timestamp.
    void saveGameState(const std::string& filename = "");
    // Loads game state from the specified file. Returns true on success, false on failure.
    bool loadGameState(const std::string& filename);


}; // End SaveLoadManager class