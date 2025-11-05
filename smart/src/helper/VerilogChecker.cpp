#include "VerilogChecker.h"
#include "State.h"
#include "StateMaker.h"
#include "Timer.h"
#include "Utils.h"
#include <unistd.h>

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
#include <algorithm>
#include <cctype>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

VerilogChecker::VerilogChecker() {
    verilogSrcPath = "";
    formalFilePath = "";
    tracePath = "";
    moduleTime = "";
    bound = 10;
}

VerilogChecker::VerilogChecker(std::string verilogSrcPath, BackEndSolver solver) {
    this->verilogSrcPath = verilogSrcPath;
    this->solver = solver;
    moduleTime = "";
    bound = 10;
}

VerilogChecker::VerilogChecker(std::string verilogSrcPath,std::string homePath,BackEndSolver solver) {
    this->verilogSrcPath = verilogSrcPath;
    this->homePath = homePath;
    this->solver = solver;
    moduleTime = "";
    bound = 10;
    std::string relatePath = homePath+"/runtime/verilog";

    try {
        for (const auto& entry : fs::directory_iterator(relatePath)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".sv" && path != verilogSrcPath) {
                    relatedFilePaths.push_back(path);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        ;
    }
}

VerilogChecker::~VerilogChecker() {
}

void VerilogChecker::setVerilogSrcPath(std::string path) {
    verilogSrcPath = path;
}

void VerilogChecker::setformalFilePath(std::string path) {
    formalFilePath = path;
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
    std::string currentModuleName = "";
    bool isModule = false;
    bool insertProperties = false;
    bool insideBlockComment = false;

    while (std::getline(inputFile, line)) {
        lines.push_back(line);

        std::string trimmed = trim(line);

        if (trimmed.find("/*") != std::string::npos) {
            insideBlockComment = true;
        }
        if (insideBlockComment) {
            if (trimmed.find("*/") != std::string::npos) {
                insideBlockComment = false;
            }
            continue;
        }

        size_t commentPos = line.find("//");
        size_t modulePos = line.find("module");
        if ((commentPos != std::string::npos && (modulePos == std::string::npos || modulePos > commentPos))) {
            continue;
        }

        if (line.find("endmodule") != std::string::npos && isModule) {
            assert(properties.size() == propertyTypes.size());
            for (int i = 0; i < properties.size(); i++) {
                lines.insert(lines.end() - 1, "    assert property (" + properties[i] + ");\n");
                insertProperties = true;
            }
            isModule = false;
        }

        if (line.find("module") != std::string::npos) {
            int start = line.find("module") + 6;

            // Skip whitespace
            while (start < line.size() && isspace(line[start])) {
                ++start;
            }

            // Find the end of the module name (before space, '#' or '(' )
            int end = start;
            while (end < line.size() && !isspace(line[end]) && line[end] != '#' && line[end] != '(') {
                ++end;
            }

            currentModuleName = line.substr(start, end - start);
            currentModuleName = trim(currentModuleName);

            printDebug("Current module name: " + currentModuleName, 4);
            // print("Current module name: " + currentModuleName + "\n");
        }


        if (currentModuleName == topModule) {
            printDebug("Current module is top module", 4);
            isModule = true;
        } else {
            printDebug("Current module name: " + currentModuleName, 4);
            printDebug("Top module name: " + topModule, 4);
            isModule = false;
        }
    }
    inputFile.close();

   
    std::ofstream outputFile(formalFilePath);
    if(!outputFile.is_open()) {
        printError("Error: Unable to open output file "+formalFilePath+"\n");
        exit(1);
    }
    for(auto &line : lines) {
        outputFile << line << std::endl;
    }
    outputFile.close();
    printDebug("Modified Verilog file is stored in " + formalFilePath,3);

    if(!insertProperties) {
        printError("Error: Unable to insert properties in the module "+topModule+"\n");
        exit(1);
    }
    // print("Successfully wrote formal file "+formalFilePath+"\n");
}

void VerilogChecker::addProperty(State* state,PropertyType type) {
    propertyTypes.push_back(type);

    //if we want to verifiy that a state is reachable
    //we should make !state as the property
    //if there is verified, then the state is unreachable otherwise it is reachable
    if(type == PropertyType::REACHABILITY_PROPERTY) {
        assert(!state->isUndefined());//only defined states can be verified

        properties.push_back(moduleTime+" (!"+state->toVerilogExpr()+")");
    }
}

void VerilogChecker::addProperty(SygusFunction* func,PropertyType type) {
    propertyTypes.push_back(type);

    if(type == PropertyType::REACHABILITY_PROPERTY){
        properties.push_back(moduleTime+" "+func->getBodyVerilogExpr());
    }
    else if(type == PropertyType::SAFT_PROPERTY){
        properties.push_back(moduleTime+func->getBodyVerilogExpr());
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
    command += "ebmc "+formalFilePath;
    // command += " --bound "+std::to_string(bound);

    for(auto &path : relatedFilePaths) {
        command +=" "+path+ " ";
    }
    
    if(unboundChecker)
        command += " --k-induction";
    else
        command += " --bound "+std::to_string(bound);

    if(topModule != "")
        command += " --top "+topModule;
    // command += " --vcd test.vcd";
    command += " > /dev/null 2>&1";
    printDebug("Running EBMC with command: "+command+"\n",1);
    timer->start(timerType::FM_Timer);
    int status = system(command.c_str());
    timer->stop(timerType::FM_Timer);
    return status==0;
}

bool VerilogChecker::runEBMC(std::string tracePath){
    std::string command = "";
    command += "ebmc "+formalFilePath;
    // command += " --bound "+std::to_string(bound);

    for(auto &path : relatedFilePaths) {
        command +=" "+path+ " ";
    }

    if(unboundChecker)
        command += " --k-induction";
    else
        command += " --bound "+std::to_string(bound);

    if(topModule != "")
        command += " --top "+topModule;
    command += " --vcd "+tracePath;
    command += " > /dev/null 2>&1";
    printDebug("Running EBMC with command: "+command+"\n",1);
    int status = system(command.c_str());
    return status==0;
}

//The check function will return true if the state is reachable
bool VerilogChecker::checkStateReachability(State* state) {
    cleanProperties();
    generateFormalFilePath(PropertyType::REACHABILITY_PROPERTY);
    addProperty(state,PropertyType::REACHABILITY_PROPERTY);
    writeVerilogFile();
    bool result = false;
    if(solver==BackEndSolver::EBMC) {
        result = runEBMC();
    }
    else if(solver==BackEndSolver::SBY) {
        result = runSby();
    }
    if(deleteTempFile)
        deleteVerilogFile();
    return !result;
}

//The check function will return true if the property is verified/ the state is safe
bool VerilogChecker::checkExprSafety(SygusFunction* func,std::string tracePath) {
    cleanProperties();
    generateFormalFilePath(PropertyType::SAFT_PROPERTY);
    addProperty(func,PropertyType::SAFT_PROPERTY);
    writeVerilogFile();


    bool result = false;
    if(solver==BackEndSolver::EBMC) {
        result = runEBMC(tracePath);
    }
    else if(solver==BackEndSolver::SBY) {
        result = runSby();
        if(!result){
            //if the result is failure, we need to move vcd file to the right place
            std::string vcdPath = homePath + "/runtime/formal/dis_task/engine_0/trace.vcd";
            std::string command = "cp "+vcdPath+" "+tracePath;
            int status = system(command.c_str());
            if(status!=0) {
                printError("Error: Unable to copy vcd file "+vcdPath+" to "+tracePath+"\n");
                exit(1);
            }
        }
    }
    
    //if the property is verified, then the state is safe
    printDebug("The safety result is: "+std::to_string(result),1);
    // if(deleteTempFile)
    //     deleteVerilogFile();
    return result;
}

std::string VerilogChecker::generateFormalFilePath(PropertyType type) {

    auto now = std::chrono::system_clock::now();

    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&timeNow), "%Y%m%d_%H%M%S") 
        << "_" << std::setw(3) << std::setfill('0') << millis;

    std::string filename = homePath + "/runtime/formal";
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
    filename += std::to_string(getpid());
    filename += "_";
    filename +=".sv";
    formalFilePath = filename;
    return filename;
}

void VerilogChecker::setBound(int bound) {
    this->bound = bound;
}

void VerilogChecker::setTopModule(std::string topModule) {
    this->topModule = topModule;
}

void VerilogChecker::setModuleTime(std::string moduleTime) {
    this->moduleTime = moduleTime;
}

bool VerilogChecker::runSby() {
    //we should write an dis.sby file
    std::string sbyFilePath = "runtime/formal/dis.sby";
    std::ofstream sbyFile(sbyFilePath);
    if(!sbyFile.is_open()) {
        printError("Error: Unable to open sby file "+sbyFilePath+"\n");
        exit(1);
    }    
    sbyFile << "[tasks]" << std::endl;
    sbyFile << "task bmc_check files" << std::endl;
    sbyFile << std::endl;

    sbyFile << "[options]" << std::endl;
    sbyFile << "bmc_check:" << std::endl;
    sbyFile << "mode bmc" << std::endl;
    sbyFile << "depth " << 3 << std::endl;
    // sbyFile << "depth " << bound << std::endl;
    sbyFile << "timeout 120" << std::endl;
    sbyFile << "vcd_sim on"<<std::endl;
    // sbyFile << "append 10"<<std::endl;
    // sbyFile << "skip 5"<<std::endl;
    
    // sbyFile << "vcd_sim on" << std::endl;
    sbyFile << std::endl;

    sbyFile << "[engines]" << std::endl;
    sbyFile << "bmc_check:" << std::endl;
    sbyFile << "smtbmc --unroll"<<std::endl;
    sbyFile << std::endl;

    sbyFile << "[script]" << std::endl;
    sbyFile << "files:" << std::endl;
    sbyFile << "read -sv " << formalFilePath << std::endl;
    for(auto &path : relatedFilePaths) {
        sbyFile << "read -sv " << path << std::endl;
    }
    sbyFile << "prep -top " << topModule << std::endl;
    sbyFile.close();

    // print("Successfully wrote sby file "+sbyFilePath+"\n");


    std::string command = "";
    command += "sby "+sbyFilePath;
    command += " -f";

    command += " > /dev/null 2>&1";
    printDebug("Running SBY with command: "+command+"\n",1);
    timer->start(timerType::FM_Timer);
    int status = system(command.c_str());
    // print("The command is "+command+"\n");
    // print("The status is "+std::to_string(status)+"\n");
    timer->stop(timerType::FM_Timer);
    printDebug("SBY result is "+std::to_string(status),1);
    return status==0;
}

void VerilogChecker::setHomePath(std::string path) {
    homePath = path;
}

Constrains VerilogChecker::fixupConstrains(Constrains constrains) {
    int timeStamp = constrains.constraints[0].size();
    int signalSize = constrains.constraints.size();
    bool isUnDefined = false;

    Constrains newConstrains;
    
    for(auto constraint : constrains.constraints) {
        std::vector<Value*> newConstraint;
        newConstrains.constraints.push_back(newConstraint);
    }
    newConstrains.tracePath = constrains.tracePath; 
    newConstrains.isTrue = constrains.isTrue;

    for(int time=0;time<timeStamp;time++) {
        int count = 0;
        isUnDefined = false;
        //time Stamp i
        for(int signalIndex=0;signalIndex<signalSize;signalIndex++) {
            //signal i
            Value* value = constrains.constraints[signalIndex][time];
            if(value->isUndefined()) {
                isUnDefined = true;
                count++;
                break;
            }
        }
        //if there is any undefined value, we should fix up the value
        if(isUnDefined) {
            //Time is time
            std::vector<Value*> values;
            for(int signalIndex=0;signalIndex<signalSize;signalIndex++) {
                //signal i
                Value* value = constrains.constraints[signalIndex][time];
                values.push_back(value);
            }

            // print("\t\tSMT File constains "+std::to_string(count)+" undefined values");

            //make the state reachable
            State* state = makeReachableState(values);
            std::vector<Value*> newValues = state->getValues();
            
            //add the new values to the new constrains
            for(int signalIndex=0;signalIndex<signalSize;signalIndex++) {
                newConstrains.constraints[signalIndex].push_back(newValues[signalIndex]);
            }

            continue;
        //else we just use the original value
        }else{
            for(int signalIndex=0;signalIndex<signalSize;signalIndex++) {
                //signal i
                Value* value = constrains.constraints[signalIndex][time];
                if(value->isUndefined()){
                    print("Error: Undefined value in the constrains\n");
                    exit(1);
                }
                newConstrains.constraints[signalIndex].push_back(value);
            }
        }
    }
    return newConstrains;
}

void VerilogChecker::setSignals(std::vector<Signal>* setSignals) {
    this->signals = setSignals;
}

State* VerilogChecker::makeReachableState(std::vector<Value*> values){
    // print("\t\tTry to fix up the state\n");
    StateMaker sm(signals);
    State* state = sm.fixUpState(values);

    if(checkStateReachability(state)) {
        return state;
    }
    else{
        state = sm.fixUpState(state,values);
        return state;
    }
}

void VerilogChecker::setTimer(Timer* timer) {
    this->timer = timer;
}

void VerilogChecker::deleteVerilogFile(){
    std::string command = "rm -rf "+formalFilePath;
    int status = system(command.c_str());
    if(status!=0) {
        printError("Error: Unable to delete file "+formalFilePath+"\n");
        exit(1);
    }
    printDebug("Deleted file "+formalFilePath,3);
}