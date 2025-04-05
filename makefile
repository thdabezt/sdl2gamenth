# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -g -DDEBUG

# SDL2 include and library paths
SDL2_CFLAGS = -IC:/msys64/mingw64/include/SDL2 -IC:/Users/Admin/Documents/Code/Game/src/ECS
SDL2_LDFLAGS = -LC:/msys64/mingw64/lib -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Directories
SRC_DIR = src
BUILD_DIR = build

# Find all .cpp files in src/ and src/ECS/
SRC := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/ECS/*.cpp) $(wildcard $(SRC_DIR)/Scene/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

# Output binary name
TARGET = game

# Default target
all: $(BUILD_DIR) $(TARGET)

# Build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(SDL2_LDFLAGS) -o $(TARGET)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)  # Ensure the build directory exists
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Run the executable
run: $(TARGET)
	./$(TARGET)
