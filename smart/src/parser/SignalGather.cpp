#include "SignalGather.h"
#include "ini.h"
#include <iostream>
#include <fstream>

SignalGather::SignalGather(){

    inih::INIReader reader("/home/magna/smartVerilog/smart/tests/config.ini");
    std::set<std::string> sectionList = reader.Sections();
    for(auto &section : sectionList){
        Signal* signal = new Signal();
        signal->moduleName = reader.Get(section,"moduleName");
        signal -> name = reader.Get(section,"signalName");
        signal -> lindex = std::stoi(reader.Get(section,"lindex"));
        signal -> rindex = std::stoi(reader.Get(section,"rindex"));
        signal -> type = static_cast<SignalType>(std::stoi(reader.Get(section,"signalType")));
        signals.push_back(signal);
    }
}

SignalGather::SignalGather(std::string path){

}

SignalGather::SignalGather(std::vector<VCDSignal>){

}

SignalGather::~SignalGather(){
    for(auto &signal : signals){
        delete signal;
    }
}

const std::vector<Signal*>* SignalGather::getAllSignals(){
    return &signals;
}

const Signal SignalGather::getOneSignal(int index){
    if(index >= signals.size()){
        throw std::runtime_error("Index out of bounds");
    }
    
    return *signals[index];
}