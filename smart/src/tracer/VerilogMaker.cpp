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

void VerilogMaker::addExprToVerilog(std::string inputFilePath, std::string outputFilePath) {
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
            assert(exprs.size() == exprTypes.size());
            for(int i=0;i<exprs.size();i++) {
                if(exprTypes[i] == ExprType::UNREACHABLE_STATE) {
                    lines.insert(lines.end() - 1, "    assert property (" + exprs[i] + ");\n");
                }
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
    print("Modified Verilog file is stored in " + outputFilePath + "\n");
}

void VerilogMaker::storeExpr(State* state,ExprType type) {
    if(type == ExprType::UNREACHABLE_STATE) {
        exprs.push_back("(!"+state->toVerilogExpr()+")");
        exprTypes.push_back(type);
    }

}