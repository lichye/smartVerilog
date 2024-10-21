# Compiler and flags
CXX = g++ -std=c++17
CXXFLAGS = -I./include -I./src/parser -I./src/tracer -L./lib

# Source and build directories
SRC_DIRS = ./src/parser ./src/tracer
BUILD_DIR = ./build

# Libraries
LIBS = ./src/parser/libverilog-vcd-parser.a

# Find all .cpp files in SRC_DIRS
SRC_FILES = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

# Generate object files with full paths in BUILD_DIR, keeping directory structure
OBJ_FILES = $(patsubst ./src/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))


# Set another target
TARGET = smart.out
TARGET_SRC = ./src/smart.cpp

# Output executable
TEST_TARGET = test.out

# Rule to compile all source files into object files
$(BUILD_DIR)/%.o: ./src/%.cpp
	@mkdir -p $(dir $@)   # Ensure that the build directory and subdirectory are created
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build target executable
$(TARGET): $(OBJ_FILES) $(TARGET_SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ_FILES) $(TARGET_SRC) $(LIBS)


# Build test target executable
$(TEST_TARGET): $(OBJ_FILES) ./src/test.cpp
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(OBJ_FILES) ./src/test.cpp $(LIBS)


# Run the smart executable
smart: $(TARGET)
	./$(TARGET)

# Test rule to run the executable
test: $(TEST_TARGET)
	./$(TEST_TARGET)


# Clean rule to remove output files
clean:
	rm -rf $(BUILD_DIR) $(TEST_TARGET) $(TARGET)
