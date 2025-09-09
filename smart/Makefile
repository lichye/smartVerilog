# Source and build directories
SRC_DIR         ?= ./src
BUILD_DIR       ?= ./build
RUNTIME_DIR     ?= ./runtime
PYTHON_DIR      ?= ./src/python
SMART_TARGET	= smart.out
ASSERTION_TARGET = assertion.out
SMART_SRC      	= ./src/smart.cpp
ASSERTION_SRC   = ./src/assertion.cpp
SRC_DIRS        = ./src/parser ./src/tracer ./src/helper ./src/sygus

# Compiler and flags
CXX             ?= g++
INCLUDES := \
	-I ./src/helper/ \
	-I ./src/parser \
	-I ./src/tracer \
	-I ./src/sygus \
	-I $(BUILD_DIR) \
	-I $(SRC_DIR) \

CXXFLAGS        += $(INCLUDES) -std=c++17 -g
LDFLAGS         = -L./lib

# Ensure subdirectories exist
BUILD_SUBDIRS   = $(BUILD_DIR)/parser $(BUILD_DIR)/tracer $(BUILD_DIR)/helper $(BUILD_DIR)/sygus

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

compile: $(LEX_OUT) $(YAC_OUT) $(OBJS) $(SMART_SRC)
	$(CXX) $(CXXFLAGS) -o $(SMART_TARGET) $^

smart:
	./$(SMART_TARGET)

sim:
	python $(RUNTIME_DIR)/cocotb/sim.py $(RUNTIME_DIR)/verilog/addsub.sv
	mv sim_build/dump.vcd $(RUNTIME_DIR)/sim_results/sim.vcd

# clean is clean the compiled code
all_clean: clean 
	rm -rf $(RUNTIME_DIR)
	rm -rf sim_build
	rm -rf $(SMART_TARGET)
	rm -rf user/*
	rm -rf sygus.sl
	rm -rf benchmarks/*
	rm -rf $(BUILD_DIR) $(LEX_OUT) $(LEX_HEADER) $(LEX_OBJ) \
           $(YAC_OUT) $(YAC_HEADER) $(YAC_OBJ) \
           position.hh stack.hh location.hh VCDParser.output $(TEST_TARGET) $(SMART_TARGET)
	mkdir $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LEX_OUT) $(LEX_HEADER) $(YAC_OUT) $(YAC_HEADER)
	rm -rf $(OBJS) $(SMART_TARGET) $(ASSERTION_TARGET)
	rm -rf $(RUNTIME_DIR)/smt_results/*.vcd
	mkdir $(BUILD_DIR)

smart_clean:
	rm -rf $(SMART_TARGET)
	rm -rf $(RUNTIME_DIR)/smt_results/*.vcd
	rm -rf sygus.sl


