# Compiler and flags
CXX             ?= g++
CXXFLAGS        += -std=c++17 -I./include -I./src/parser -I./src/tracer -I$(SRC_DIR) -I$(BUILD_DIR) -g
LDFLAGS         = -L./lib

# Source and build directories
SRC_DIR         ?= ./src
BUILD_DIR       ?= ./build
RUNTIME_DIR     ?= ./runtime

# Ensure subdirectories exist
BUILD_SUBDIRS   = $(BUILD_DIR)/parser $(BUILD_DIR)/tracer

# VCD Parser Files (from the first Makefile)
LEX_SRC         ?= $(SRC_DIR)/parser/VCDScanner.l
LEX_OUT         ?= $(BUILD_DIR)/VCDScanner.cpp
LEX_HEADER      ?= $(BUILD_DIR)/VCDScanner.hpp

YAC_SRC         ?= $(SRC_DIR)/parser/VCDParser.ypp
YAC_HEADER      ?= $(BUILD_DIR)/VCDParser.hpp
YAC_OUT         ?= $(BUILD_DIR)/VCDParser.cpp

VCD_SRC         ?= $(SRC_DIR)/VCDFile.cpp \
                   $(SRC_DIR)/VCDValue.cpp \
                   $(SRC_DIR)/VCDFileParser.cpp

SMART_TARGET	= smart.out
SMART_SRC      	= ./src/smart.cpp

SRC_DIRS        = ./src/parser ./src/tracer

SRC_FILES       = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

# Object files for the main source files
OBJS            = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Rule to create build directories
$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

# Pattern rule to compile .cpp to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_SUBDIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ): $(TEST_SRC) | $(BUILD_SUBDIRS)
	$(CXX) $(CXXFLAGS) -c $(TEST_SRC) -o $(TEST_OBJ)

# Rules for Bison and Flex
$(YAC_OUT): $(YAC_SRC) | $(BUILD_SUBDIRS)
	bison -v --defines=$(YAC_HEADER) $(YAC_SRC) -o $(YAC_OUT)

$(LEX_OUT): $(LEX_SRC) $(YAC_OUT) | $(BUILD_SUBDIRS)
	flex  -P VCDParser --header-file=$(LEX_HEADER) -o $(LEX_OUT) $(LEX_SRC)

# Main compilation rule for smart.out
compile: $(LEX_OUT) $(YAC_OUT) $(OBJS) $(SMART_SRC)
	$(CXX) $(CXXFLAGS) -o $(SMART_TARGET) $^

run:
	./$(SMART_TARGET)
	cvc5 --sygus sygus.sl

# Clean rule to remove output files
clean:
	rm -rf $(BUILD_DIR) $(LEX_OUT) $(LEX_HEADER) $(LEX_OBJ) \
           $(YAC_OUT) $(YAC_HEADER) $(YAC_OBJ) \
           position.hh stack.hh location.hh VCDParser.output $(TEST_TARGET) $(SMART_TARGET) \
		   sygus.sl

	mkdir $(BUILD_DIR)
