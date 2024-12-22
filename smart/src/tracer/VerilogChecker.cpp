#include "VerilogChecker.h"
#include "State.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>


VerilogChecker::VerilogChecker() {
    verilogSrcPath = "";
    ebmcPath = "";
    tracePath = "";
    bound = 10;
}

VerilogChecker::VerilogChecker(std::string verilogSrcPath,std::string ebmcPath) {
    this->verilogSrcPath = verilogSrcPath;
    this->ebmcPath = ebmcPath;
    bound = 10;
}

VerilogChecker::~VerilogChecker() {
}

void VerilogChecker::setVerilogSrcPath(std::string path) {
    verilogSrcPath = path;
}

void VerilogChecker::setEBMCPath(std::string path) {
    ebmcPath = path;
}

void VerilogChecker::setTracePath(std::string path) {
    tracePath = path;
}

void VerilogChecker::writeVerilogFile() {
    std::ifstream inputFile(verilogSrcPath);
    if(!inputFile.is_open()) {
        printError("Error: Unable to open input file "+verilogSrcPath+"\n");
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

    std::ofstream outputFile(ebmcPath);
    if(!outputFile.is_open()) {
        printError("Error: Unable to open output file "+ebmcPath+"\n");
        exit(1);
    }
    for(auto &line : lines) {
        outputFile << line << std::endl;
    }
    outputFile.close();
    printDebug("Modified Verilog file is stored in " + ebmcPath,3);
}

void VerilogChecker::addProperty(State* state,PropertyType type) {
    propertyTypes.push_back(type);

    //if we want to verifiy that a state is reachable
    //we should make !state as the property
    //if there is verified, then the state is unreachable otherwise it is reachable
    if(type == PropertyType::REACHABILITY_PROPERTY) {
        assert(!state->isUndefined());//only defined states can be verified

        properties.push_back("##1 (!"+state->toVerilogExpr()+")");
    }
}

void VerilogChecker::addProperty(SygusFunction* func,PropertyType type) {
    propertyTypes.push_back(type);

    if(type == PropertyType::REACHABILITY_PROPERTY){
        properties.push_back("##1 "+func->getBodyVerilogExpr());
    }
    else if(type == PropertyType::SAFT_PROPERTY){
        properties.push_back("##1 "+func->getBodyVerilogExpr());
    }
    else{
        printError("Error: Property type not supported\n");
        exit(1);
    }
    
}

void VerilogChecker::cleanProperties() {
    properties.clear();
    propertyTypes.clear();
}

bool VerilogChecker::runEBMC(){
    std::string command = "";
    command += "ebmc "+ebmcPath;
    command += " --bound "+std::to_string(bound);
    command += " --vcd test.vcd";
    command += " > /dev/null 2>&1";
    printDebug("Running EBMC with command: "+command+"\n",1);
    int status = system(command.c_str());
    return status==0;
}

bool VerilogChecker::runEBMC(std::string tracePath){
    std::string command = "";
    command += "ebmc "+ebmcPath;
    command += " --bound "+std::to_string(bound);
    command += " --vcd "+tracePath;
    command += " > /dev/null 2>&1";
    printDebug("Running EBMC with command: "+command+"\n",1);
    int status = system(command.c_str());
    return status==0;
}
//The check function will return true if the state is reachable
bool VerilogChecker::checkStateReachability(State* state) {
    cleanProperties();
    generateEbmcPath(PropertyType::REACHABILITY_PROPERTY);
    addProperty(state,PropertyType::REACHABILITY_PROPERTY);
    writeVerilogFile();
    bool result = runEBMC();
    //if the property is verified, then the state is unreachable
    printDebug("The reachability result is: "+std::to_string(result),1);
    return !result;
}

//The check function will return true if the property is verified/ the state is safe
bool VerilogChecker::checkExprSafety(SygusFunction* func,std::string tracePath) {
    cleanProperties();
    generateEbmcPath(PropertyType::SAFT_PROPERTY);
    addProperty(func,PropertyType::SAFT_PROPERTY);
    writeVerilogFile();
    bool result = runEBMC(tracePath);
    //if the property is verified, then the state is safe
    printDebug("The safety result is: "+std::to_string(result),1);
    return result;
}

std::string VerilogChecker::generateEbmcPath(PropertyType type) {

    auto now = std::chrono::system_clock::now();

    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&timeNow), "%Y%m%d_%H%M%S") 
        << "_" << std::setw(3) << std::setfill('0') << millis;

    std::string filename="";
    filename += "runtime/ebmc/";
    if(type == PropertyType::REACHABILITY_PROPERTY) {
        filename +="/reachability_";
    }
    else if(type == PropertyType::SAFT_PROPERTY) {
        filename +="/safety_";
    }
    else {
        printError("Error: Property type not supported\n");
        exit(1);
    }
    filename += oss.str();
    filename +=".sv";
    ebmcPath = filename;
    return filename;
}
