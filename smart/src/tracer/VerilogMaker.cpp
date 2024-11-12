#include "VerilogMaker.h"
#include "State.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>

VerilogMaker::VerilogMaker() {
}

VerilogMaker::~VerilogMaker() {
}

void VerilogMaker::writeVerilogFile(std::string inputFilePath, std::string outputFilePath) {
    std::ifstream inputFile(inputFilePath);
    if(!inputFile.is_open()) {
        printError("Error: Unable to open input file "+inputFilePath+"\n");
        exit(1);
    }
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(inputFile, line)) {
        lines.push_back(line);
        if(line.find("endmodule") != std::string::npos) {
            assert(properties.size() == propertyTypes.size());
            for(int i=0;i<properties.size();i++) {
                lines.insert(lines.end() - 1, "    assert property (" + properties[i] + ");\n");
            }
        }
    }
    inputFile.close();

    std::ofstream outputFile(outputFilePath);
    if(!outputFile.is_open()) {
        printError("Error: Unable to open output file "+outputFilePath+"\n");
        exit(1);
    }
    for(auto &line : lines) {
        outputFile << line << std::endl;
    }
    outputFile.close();
    printDebug("Modified Verilog file is stored in " + outputFilePath,3);
}

//TODO:
//we do not care about the initial state now
void VerilogMaker::addProperty(State* state,PropertyType type) {
    propertyTypes.push_back(type);

    //if we want to verifiy that a state is reachable
    //we should make !state as the property
    //if there is verified, then the state is unreachable otherwise it is reachable
    if(type == PropertyType::REACHABILITY_PROPERTY) {
        assert(!state->isUndefined());//only defined states can be verified

        properties.push_back("##[0:$] (!"+state->toVerilogExpr()+")");
    }
}