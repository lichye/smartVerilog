#include "Module.h"
#include "Trace.h"
#include <vector>
#include <filesystem>
#include <set>
namespace fs = std::filesystem;

Module::Module(std::string inputModuleName){
    this->moduleName = inputModuleName;
}

void Module::addTrace(TraceType traceType,std::string input_tracePath){
    for(auto tracePath:tracePaths){
        if(tracePath == input_tracePath){
            printDebug("Trace already exists",2);
            return;
        }
    }
    Trace* trace = new Trace(traceType,input_tracePath);
    traces.push_back(trace);
    tracePaths.push_back(input_tracePath);
}

void Module::addTracesfromDir(TraceType traceType, std::string dirPath){
    for (const auto & entry : fs::directory_iterator(dirPath)){
        addTrace(traceType,entry.path());
    }
}

std::vector<Signal>* Module::getAllSignals(){
    std::vector<Signal>* retSignals = new std::vector<Signal>();
    assert(!traces.empty());
    return traces[0]->getAllSignals(moduleName);
}

std::vector<Constrains> Module::getAllConstraints(std::vector<Signal>* signals){
    std::vector<Constrains> allConstraints;
    for(auto trace : traces){
        Constrains constraints;
        constraints.constraints = *trace->getConstraints(signals);
        constraints.tracePath = trace->getPath();
        constraints.isTrue = true;
        allConstraints.push_back(constraints);
    }
    return allConstraints;
}

Constrains Module::getConstrain(std::string tracePath,std::vector<Signal>* signals){
    printDebug("Run Module::getConstrain",2);
    printDebug("Getting constraints from the trace: "+tracePath,2);
    printDebug("this is trace's sizes: "+std::to_string(traces.size()),3);
    try{
        for(auto trace : traces){
        if(trace->getPath() == tracePath){
            printDebug("Trace found",3);
            Constrains constraints;
            constraints.constraints = *trace->getConstraints(signals);
            constraints.tracePath = tracePath;
            constraints.isTrue = true;
            return constraints;
        }
    }
    }catch (const std::exception& e) { // 捕获标准异常
        printDebug(std::string("Standard exception caught: ") + e.what(), 1);
        print("Trace not found");
    }
    print("Trace not found"); 
    exit(1);
}