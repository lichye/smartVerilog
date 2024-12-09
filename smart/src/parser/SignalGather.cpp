#include "SignalGather.h"
#include "ini.h"
#include <iostream>
#include <fstream>
#include <exception>

SignalGather::SignalGather(){
    try{
        inih::INIReader reader("tests/config.ini");
        std::set<std::string> sectionList = reader.Sections();
        for(auto &section : sectionList){
            Signal signal;
            signal.moduleName = reader.Get(section,"modulename");
            signal.name = reader.Get(section,"signalname");
            signal.lindex = std::stoi(reader.Get(section,"lindex"));
            signal.rindex = std::stoi(reader.Get(section,"rindex"));
            signal.type = static_cast<SignalType>(std::stoi(reader.Get(section,"signaltype")));
            signals.push_back(signal);
        }
    } catch (...){
        Signal signal;
    }
    
}

SignalGather::SignalGather(std::string path){
    inih::INIReader reader(path);
    std::set<std::string> sectionList = reader.Sections();
    if(sectionList.size() == 0){
        print("No signals found, so use the all Signal model\n");
        return;
    }
    for(auto &section : sectionList){
        Signal signal;
        signal.moduleName = reader.Get(section,"modulename");
        signal.name = reader.Get(section,"signalname");
        signal.lindex = std::stoi(reader.Get(section,"lindex"));
        signal.rindex = std::stoi(reader.Get(section,"rindex"));
        signal.type = static_cast<SignalType>(std::stoi(reader.Get(section,"signaltype")));
        signals.push_back(signal);
    }
}

SignalGather::SignalGather(std::vector<VCDSignal>){

}

SignalGather::~SignalGather(){

}

std::vector<Signal>* SignalGather::getAllSignals(){
    std::vector<Signal>* results = new std::vector<Signal>();
    for(auto &signal : signals){
        Signal signalCopy = signal;
        results->push_back(signalCopy);
    }
    return results;
}

Signal SignalGather::getOneSignal(int index){
    if(index >= signals.size()){
        throw std::runtime_error("Index out of bounds");
    }
    
    return signals[index];
}

int SignalGather::getSignalSize(){
    return signals.size();
}

bool SignalGather::hasSignal(){
    return signals.size() > 0;
}