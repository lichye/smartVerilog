#include "State.h"
#include "utils.h"
#include <iostream>
#include <cassert>

State::State(){
    printDebug("State Constructor called",10);
    signalAllExist = false;
    reachable = false;
    signals = new std::vector<Signal>();   
}

State::~State(){
    
    //delete signals;
    printDebug("State Destructor called",10);
}

State::State(bool reachable){
    this->reachable = reachable;
}

bool State::isUndefined(){
    for(auto value : values){
        if(value->isUndefined()){
            return true;
        }
    }
    return false;
}

void State::addValue(Signal signal, Value* value){
    if(signalAllExist){
        printDebug("Signal already exists",1);
        exit(1);
    }

    signals->push_back(signal);
    values.push_back(value);
}

void State::addValue(Value* value){
    if(!signalAllExist){
        printDebug("Signal does not exist",1);
        exit(1);
    }

    values.push_back(value);
}

void State::setSignals(std::vector<Signal>* inputSignals){
    for(auto signal : *inputSignals){
        signals->push_back(signal);
    }
    signalAllExist = true;
} 

std::string State::toString(){
    std::string str = "";
    assert(signals->size() == values.size());

    for(int i=0;i<signals->size();i++){
        str += (*signals)[i].name + " : " + values[i]->toString() + "\n";
    }
    return str;
}
