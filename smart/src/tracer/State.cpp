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
    for(auto value : values){
        delete value;
    }
    printDebug("State Destructor called",10);
}

State::State(bool reachable){
    this->reachable = reachable;
}

std::string State::toString(){
    std::string str = "";
    assert(signals->size() == values.size());

    for(int i=0;i<signals->size();i++){
        str += (*signals)[i].name + " : " + values[i]->toString() + "\n";
    }
    return str;
}

std::string State::toVerilogExpr(){
    std::string str ="(";
    assert(signals->size() == values.size());
    for(int i=0;i<signals->size()-1;i++){
        str += (*signals)[i].name + "==" + values[i]->toVerilogString() + " && ";
    }
    str += (*signals)[signals->size()-1].name + "==" + values[signals->size()-1]->toVerilogString();
    str += ")";
    return str;
}
void State::addValue(Value* value){
    if(!signalAllExist){
        printDebug("Signal does not exist",1);
        exit(1);
    }
    Value* newValue = value->clone();
    values.push_back(newValue);
}


void State::addValue(Signal signal, Value* value){
    Value* newValue = value->clone();
    if(signalAllExist){
        printDebug("Signal already exists",1);
        exit(1);
    }

    signals->push_back(signal);
    values.push_back(value);
}

bool State::isUndefined(){
    for(auto value : values){
        if(value->isUndefined()){
            return true;
        }
    }
    return false;
}

void State::setSignals(std::vector<Signal>* inputSignals){
    for(auto signal : *inputSignals){
        signals->push_back(signal);
    }
    signalAllExist = true;
} 


