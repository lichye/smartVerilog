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

    //lock the file parser mutex
    std::lock_guard<std::mutex> lock(VCDFileParserMutex);
    if(ctype == TraceType::SIM){
        vcdPath = path;
        if(vcdPath != ""){
            std::cout<<"try to parse the vcd file in: "<<vcdPath<<std::endl; 
            VCDFileParser vcdParser;
            VCDFile* vcdFile = vcdParser.parse_file(vcdPath);
            assert(vcdFile != nullptr && "VCD file should not be null");

            readVCDFile(vcdFile);
            delete vcdFile;
            std::cout<<"Trace Gathered\n"<<std::endl;
        }
    }else if(ctype == TraceType::SMT){ 
        smtPath = path;
        if(smtPath != ""){
            std::cout<<"try to parse the vcd file in: "<<smtPath<<std::endl; 
            VCDFileParser vcdParser;
            VCDFile* vcdFile = vcdParser.parse_file(smtPath);  
            assert(vcdFile != nullptr && "VCD file should not be null");
            
            readVCDFile(vcdFile);
            delete vcdFile;
            std::cout<<"Trace Gathered\n"<<std::endl;
        }
        
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
        std::cout<<"Signal not found: "<<s.name<<std::endl;
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

std::vector<std::vector<Value*>>* Trace::getConstraints(std::vector<Signal>* signals){
    std::vector<std::vector<Value*>>* constraints = new std::vector<std::vector<Value*>>();
    for(Signal s : *signals){
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
            //if there exists timestamps, means there is change in the signal
            for(VCDTime time : *timestamps){
                VCDValue* val = vcdFile -> get_signal_value_at(signal -> hash, time);
                
                //if there is no value for the signal, then we make a good value for that
                if(val == nullptr){
                    if(traceType==TraceType::SMT){
                        Value* value = Value::makeRandomValue(s.type);
                        values->push_back(value);
                    }
                    else{
                        std::cout<<"Simulation trace should not have null value"<<std::endl;
                        exit(1);
                    }
                }
                else{
                    Value* value = new Value(val);
                    values->push_back(value);
                }
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

std::vector<Signal>* Trace::getAllSignals(){
    std::vector<Signal>* signals = new std::vector<Signal>();
    for(auto &signal : signals_map){
        Signal signalCopy = signal.first;
        signals->push_back(signalCopy);
    }
    return signals;
}
