SRC_DIR := src
BIN_DIR := bin

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRC_FILES))
TARGET := $(BIN_DIR)/App.exe

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

build: $(TARGET)

run: build
	./$(TARGET)

clean:
	del /Q $(BIN_DIR)\*.o $(TARGET)

.PHONY: build run clean
