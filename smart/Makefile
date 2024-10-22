# Compiler and flags
CXX             ?= g++ -std=c++17
CXXFLAGS        += -I./include -I./src/parser -I./src/tracer -I$(SRC_DIR) -I$(BUILD_DIR) -g
LDFLAGS         = -L./lib

# Source and build directories
SRC_DIR         ?= ./src
BUILD_DIR       ?= ./build

# VCD Parser Files (from the first Makefile)
LEX_SRC         ?= $(SRC_DIR)/parser/VCDScanner.l
LEX_OUT         ?= $(BUILD_DIR)/VCDScanner.cpp
LEX_HEADER      ?= $(BUILD_DIR)/VCDScanner.hpp
LEX_OBJ         ?= $(BUILD_DIR)/VCDScanner.o

YAC_SRC         ?= $(SRC_DIR)/parser/VCDParser.ypp
YAC_OUT         ?= $(BUILD_DIR)/VCDParser.cpp
YAC_HEADER      ?= $(SRC_DIR)/parser/VCDParser.hpp
YAC_OBJ         ?= $(BUILD_DIR)/VCDParser.o

VCD_SRC         ?= $(SRC_DIR)/VCDFile.cpp \
                   $(SRC_DIR)/VCDValue.cpp \
                   $(SRC_DIR)/VCDFileParser.cpp


TARGET          = smart.out
TARGET_SRC      = ./src/smart.cpp

TEST_TARGET     = test.out
TEST_SRC        = ./src/test.cpp


# Other source files from the second Makefile
SRC_DIRS        = ./src/parser ./src/tracer
SRC_FILES       = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))


$(YAC_OUT) : $(YAC_SRC)
	bison -v --defines=$(YAC_HEADER) $(YAC_SRC) -o $(YAC_OUT)

$(LEX_OUT) : $(LEX_SRC) $(YAC_OUT)
	flex  -P VCDParser --header-file=$(LEX_HEADER) -o $(LEX_OUT) $(LEX_SRC)



yac: $(YAC_OUT)

flex: $(LEX_OUT)
# Run the smart executable




# Clean rule to remove output files
clean:
	rm -rf $(BUILD_DIR) $(LEX_OUT) $(LEX_HEADER) $(LEX_OBJ) \
           $(YAC_OUT) $(YAC_HEADER) $(YAC_OBJ) \
           position.hh stack.hh location.hh VCDParser.output $(TEST_TARGET) $(TARGET)
	mkdir $(BUILD_DIR)
