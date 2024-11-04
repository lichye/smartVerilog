#include "SignalGather.h"
#include "ini.h"
#include <iostream>
#include <fstream>

SignalGather::SignalGather(){

    inih::INIReader reader("tests/config.ini");
    std::set<std::string> sectionList = reader.Sections();
    for(auto &section : sectionList){
        Signal signal;
        signal.moduleName = reader.Get(section,"moduleName");
        signal.name = reader.Get(section,"signalName");
        signal.lindex = std::stoi(reader.Get(section,"lindex"));
        signal.rindex = std::stoi(reader.Get(section,"rindex"));
        signal.type = static_cast<SignalType>(std::stoi(reader.Get(section,"signalType")));
        signals.push_back(signal);
    }
}

SignalGather::SignalGather(std::string path){
    inih::INIReader reader(path);
    std::set<std::string> sectionList = reader.Sections();
    for(auto &section : sectionList){
        Signal signal;
        signal.moduleName = reader.Get(section,"moduleName");
        signal.name = reader.Get(section,"signalName");
        signal.lindex = std::stoi(reader.Get(section,"lindex"));
        signal.rindex = std::stoi(reader.Get(section,"rindex"));
        signal.type = static_cast<SignalType>(std::stoi(reader.Get(section,"signalType")));
        signals.push_back(signal);
    }
}

SignalGather::SignalGather(std::vector<VCDSignal>){

}

SignalGather::~SignalGather(){

}

std::vector<Signal>* SignalGather::getAllSignals(){
    return &signals;
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