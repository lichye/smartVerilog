#include "Trace.h"
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
         //lock the mutex of the parser
        std::lock_guard<std::mutex> lock(VCDFileParserMutex);
        vcdPath = path;
        if(vcdPath != ""){
            std::cout<<"try to parse the vcd file: "<<vcdPath<<std::endl; 
            VCDFileParser vcdParser;

            std::cout<<"Create new VCDParser"<<std::endl; 

            VCDFile* vcdFile = vcdParser.parse_file(vcdPath);
            
            assert(vcdFile != nullptr && "VCD file should not be null");

            readVCDFile(vcdFile);
            delete vcdFile;
            std::cout<<"VCD file parsed\n"<<std::endl;
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

std::mutex Trace::VCDFileParserMutex;

std::vector<Value*>* Trace::getSignalValue(Signal s){
    
    if(signals_map.find(s) != signals_map.end()){
        return signals_map[s];
    }
    else{
        assert(false&&"Signal not found");
    }
}

std::vector<std::vector<Value*>>* Trace::getConstraints(std::vector<Signal> signals){
    std::vector<std::vector<Value*>>* constraints = new std::vector<std::vector<Value*>>();
    for(Signal s : signals){
        std::vector<Value*>* values = getSignalValue(s);
        constraints->push_back(*values);
    }
    return constraints;

}

void Trace::readVCDFile(VCDFile* vcdFile){
    if(vcdFile == nullptr){
        std::cout<<"No VCD file to read"<<std::endl;
        return;
    }
    //loop the signals and the time to make the map
    for(VCDScope* scope : *vcdFile -> get_scopes()){
        for(VCDSignal* signal : scope -> signals){
            Signal s;
            s.moduleName = scope -> name;
            s.type = translateSignalType(signal);
            s.name = signal -> reference;
            s.lindex = signal -> lindex;
            s.rindex = signal -> rindex;

            std::vector<Value*>* values = new std::vector<Value*>();
            std::vector<VCDTime>* timestamps = vcdFile -> get_timestamps();
            for(VCDTime time : *timestamps){
                VCDValue* val = vcdFile -> get_signal_value_at(signal -> hash, time);
                Value* value = new Value(val);
                values->push_back(value);
            }
            signals_map[s] = values;
        }
    }
}

void Trace::printDebug(){
    for(auto &signal : signals_map){
        std::cout<<"\n";
        std::cout<<"Signal: "<<signal.first.name<<std::endl;
        std::cout<<"Signal Module name: "<<signal.first.moduleName<<std::endl;
        std::cout<<"Signal type: "<<signal.first.type<<std::endl;
        std::cout<<"Signal lindex: "<<signal.first.lindex<<std::endl;
        std::cout<<"Signal rindex: "<<signal.first.rindex<<std::endl;
    }
}

SignalType Trace::translateSignalType(VCDSignal* vcdSignal){
    if(vcdSignal->type == VCD_VAR_REAL){
        return SignalType::DOUBLE;
    }
    else{
        if(vcdSignal->size==1){
            return SignalType::BOOLEAN;
        }
        else{
            return SignalType::BITS;
        }
    }
}