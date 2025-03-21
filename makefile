# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -g

# SDL2 include and library paths (for Windows)
SDL2_CFLAGS = -IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main
SDL2_LDFLAGS = -LC:/msys64/mingw64/lib -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_image

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source and Object files
SRC = $(SRC_DIR)/game.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/TextureManager.cpp $(SRC_DIR)/GameObject.cpp
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)


# Output binary name
TARGET = game

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(SDL2_LDFLAGS) -o $(TARGET)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

