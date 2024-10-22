# Compiler and flags
CXX             ?= g++ 
CXXFLAGS        += -std=c++17 -I./include -I./src/parser -I./src/tracer -I$(SRC_DIR) -I$(BUILD_DIR) -g
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
YAC_HEADER      ?= $(BUILD_DIR)/VCDParser.hpp
YAC_OBJ         ?= $(BUILD_DIR)/VCDParser.o

VCD_SRC         ?= $(SRC_DIR)/VCDFile.cpp \
                   $(SRC_DIR)/VCDValue.cpp \
                   $(SRC_DIR)/VCDFileParser.cpp


SMART_TARGET	= smart.out
SMART_SRC      	= ./src/smart.cpp

TEST_TARGET     = test.out
TEST_SRC        = ./src/test.cpp


# Other source files from the second Makefile
SRC_DIRS        = ./src/parser ./src/tracer
SRC_FILES       = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))


$(YAC_OUT) : $(YAC_SRC)
	bison -v --defines=$(YAC_HEADER) $(YAC_SRC) -o $(YAC_OUT)

$(LEX_OUT) : $(LEX_SRC) $(YAC_OUT)
	flex  -P VCDParser --header-file=$(LEX_HEADER) -o $(LEX_OUT) $(LEX_SRC)

test: $(TEST_SRC) $(SRC_FILES) $(YAC_OBJ) $(LEX_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $^
	./$(TEST_TARGET)

smart: $(SMART_SRC) $(SRC_FILES) $(YAC_OBJ) $(LEX_OBJ)
	$(CXX) $(CXXFLAGS) -o $(SMART_TARGET) $^
	./$(SMART_TARGET)

# Clean rule to remove output files
clean:
	rm -rf $(BUILD_DIR) $(LEX_OUT) $(LEX_HEADER) $(LEX_OBJ) \
           $(YAC_OUT) $(YAC_HEADER) $(YAC_OBJ) \
           position.hh stack.hh location.hh VCDParser.output $(TEST_TARGET) $(SMART_TARGET)
	mkdir $(BUILD_DIR)
