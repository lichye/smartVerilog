#include "Trace.h"
#include "VCDFileParser.hpp"
#include "VCDFile.hpp"
#include <string>
#include <iostream>

Trace::Trace() 
{
    traceType = TraceType::UNKNOWN;
    vcdPath = "";
    smtPath = "";
}

Trace::Trace(TraceType ctype, std::string path){
    
    traceType = ctype;
    if(ctype == TraceType::VCD){
        vcdPath = path;
        if(vcdPath != ""){
            VCDFileParser vcdParser;
            vcdFile = vcdParser.parse_file(vcdPath);
        }
    }else if(ctype == TraceType::SMT){ 
        smtPath = path;
    }
}

Trace::~Trace(){
    std::cout<<"Trace Destructor called"<<std::endl;
    for(auto &signal : signals_map){
        for(auto &value : *signal.second){
            delete value;
        }
        delete signal.second;
    }
}

void Trace::printScope(){
    if(vcdFile == nullptr){
        std::cout<<"No VCD file to print"<<std::endl;
        return;
    }
    else{
        for(VCDScope* scope : *vcdFile -> get_scopes()){
            std::cout<<"Scope: "<<scope -> name<<std::endl;
            for(VCDSignal* signal : scope -> signals){
                std::cout<<"\t"<<signal -> hash<<"\t"<<signal -> reference;
                if(signal->lindex >= 0){
                    std::cout<<"["<<signal -> lindex<<":"<<signal -> rindex<<"]";
                }
                std::cout<<std::endl;
            }
        }
    }
    return;
}

std::vector<Value*>* Trace::getSignalValue(Signal s){
    
    if(signals_map.find(s) != signals_map.end()){
        return signals_map[s];
    }

    std::vector<Value*>* values= new std::vector<Value*>();

    if(vcdFile == nullptr){
        std::cout<<"No VCD file to get signals"<<std::endl;
        return values;
    }
    else{
        VCDSignal* signal = nullptr;
        for(VCDScope* scope : *vcdFile -> get_scopes()){
            //check if the signal is in the module
            if(scope -> name != s.moduleName){
                continue;
            }
            for(VCDSignal* sig : scope -> signals){
                if(sig -> reference == s.name){
                    if(sig -> lindex != s.lindex || sig -> rindex != s.rindex){
                        continue;
                    }
                    signal = sig;
                    break;
                }
            }
        }
        if(signal == nullptr){
            std::cout<<"Signal not found"<<std::endl;
            exit(1);
        }
        std::vector<VCDTime>* timestamps = vcdFile -> get_timestamps();
        for(VCDTime time : *timestamps){
            VCDValue* val = vcdFile -> get_signal_value_at(signal -> hash, time);
            Value* value = new Value(val);
            values->push_back(value);
        }
    }
    signals_map[s] = values;
    return values;
}

