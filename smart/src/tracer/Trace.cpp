#include "Trace.h"
#include <string>
#include <iostream>

std::mutex Trace::VCDFileParserMutex;

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
            printDebug("Start to parse the vcd file in: "+vcdPath+"\n",1);
            VCDFileParser vcdParser;
            VCDFile* vcdFile = vcdParser.parse_file(vcdPath);
            assert(vcdFile != nullptr && "VCD file should not be null");

            readVCDFile(vcdFile);
            delete vcdFile;
            printDebug("Trace Gathered Successfully\n",1);
        }
    }else if(ctype == TraceType::SMT){ 
        smtPath = path;
        if(smtPath != ""){
            printDebug("Start to parse the vcd file in: "+smtPath+"\n",1);
            VCDFileParser vcdParser;
            VCDFile* vcdFile = vcdParser.parse_file(smtPath);  
            assert(vcdFile != nullptr && "VCD file should not be null");
            
            readVCDFile(vcdFile);
            delete vcdFile;
            printDebug("Trace Gathered Successfully\n",1);
        }
        
    }
    
}

Trace::~Trace(){
    printDebug("Trace Destructor called\n",1);
    for(auto &signal : signals_map){
        for(auto &value : *signal.second){
            delete value;
        }
        delete signal.second;
    }
    for(auto &state : states){
        delete state;
    }
}

std::vector<Value*>* Trace::getSignalValue(Signal s){
    
    if(signals_map.find(s) != signals_map.end()){
        return signals_map[s];
    }
    else{
        //if the signal is not found, we return an unknown value
        std::vector<Value*>* values = new std::vector<Value*>();
        for (int i =0; i<valueLength; i++){
            Value* value = new Value();
            values->push_back(value);
        }
        return values;
    }
}

std::vector<std::vector<Value*>>* Trace::getConstraints(std::vector<Signal>* signals){
    printDebug("Run Trace::getConstraints",2);
    std::vector<std::vector<Value*>>* constraints = new std::vector<std::vector<Value*>>();
    for(Signal s : *signals){
        std::vector<Value*>* values = getSignalValue(s);
        constraints->push_back(*values);
    }
    printDebug("Get Constraints Done",2);
    return constraints;
}

std::vector<Signal>* Trace::getAllSignals(){
    std::vector<Signal>* signals = new std::vector<Signal>();
    for(auto &signal : signals_map){
        Signal signalCopy = signal.first;
        signals->push_back(signalCopy);
    }
    return signals;
}

std::vector<Signal>* Trace::getAllSignals(std::string moduleName){
    std::vector<Signal>* signals = new std::vector<Signal>();
    for(auto &signal : signals_map){
        if(signal.first.moduleName == moduleName){
            Signal signalCopy = signal.first;
            signals->push_back(signalCopy);
        }
    }
    if(signals->size() == 0){
        print("We do not have any signals for the module: "+moduleName);
    }
    assert(signals->size() != 0);
    return signals;
}

std::string Trace::getPath(){
    if(traceType == TraceType::SIM){
        return vcdPath;
    }
    else if(traceType == TraceType::SMT){
        return smtPath;
    }
    else{
        return "";
    }
}

bool Trace::checkStateExist(State* state){
    for(auto &s : states){
        if(s->toString() == state->toString()){
            return true;
        }
    }
    return false;
}

void Trace::readVCDFile(VCDFile* vcdFile){
    if(vcdFile == nullptr){
        std::cout<<"No VCD file to read"<<std::endl;
        return;
    }

    std::vector<VCDTime>* timestamps = vcdFile -> get_timestamps();

    //loop the signals and the time to make the map
    for(VCDScope* scope : *vcdFile -> get_scopes()){
        for(VCDSignal* signal : scope -> signals){
            
            Signal s = createSignal(signal,scope->name);

            std::vector<Value*>* values = new std::vector<Value*>();
            
            //if there exists timestamps, means there is change in the signal
            for(VCDTime time : *timestamps){
                VCDValue* val = vcdFile -> get_signal_value_at(signal -> hash, time);
                //if there is no value for the signal, then we make a X value for that
                if(val == nullptr){
                    if(traceType==TraceType::SMT){
                        Value* value = Value::makeXValue(s.type,s.lindex - s.rindex + 1);
                        printDebug("Unknown Signal: "+s.name+" has length: "+std::to_string(s.lindex - s.rindex + 1),3);
                        // std::cout<<"Unknown Signal: "<<s.name<<" has length: "<<s.lindex - s.rindex + 1<<std::endl;
                        values->push_back(value);
                    }
                    else{
                        printError("Simulation trace should not have unknown value\n");
                    }
                }
                else{
                    Value* value = new Value(val);
                    values->push_back(value);
                }
            }
            signals_map[s] = values;
            valueLength = values->size();
        }
    }

    for(VCDTime time: *timestamps){
        printDebug("VCD Time: "+std::to_string(time),3);
        //Each time, there is a state
        State* state = new State();
        for(VCDScope* scope: *vcdFile->get_scopes()){
            for(VCDSignal* signal: scope->signals){
                Signal s = createSignal(signal,scope->name);
                VCDValue* val = vcdFile->get_signal_value_at(signal->hash,time);
                if(val == nullptr){
                    if(traceType == TraceType::SMT){
                        Value* value = Value::makeXValue(s.type,s.lindex - s.rindex + 1);
                        state->addValue(s,value);
                    }
                    else{
                        printError("Simulation trace should not have unknown value\n");
                    }
                }
                else{
                    Value* value = new Value(val);
                    state->addValue(s,value);
                }
            }
        }
        states.push_back(state);
    }
}

Signal Trace::createSignal(VCDSignal* vcdSignal,std::string moduleName){
    Signal s;
    assert(vcdSignal!=nullptr);
    //assert(vcdSignal->scope != nullptr);
    s.moduleName = moduleName;//189
    s.type = translateSignalType(vcdSignal);
    s.name = vcdSignal -> reference;
    s.lindex = vcdSignal -> lindex;
    s.rindex = vcdSignal -> rindex;
    return s;
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