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
    delete signals;
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

// StateMaker class function definitions below
StateMaker::StateMaker(){
    printDebug("StateMaker with no arguments Constructor called",10);
}

StateMaker::StateMaker(std::vector<Signal>* signals){
    this->signals = signals;
    printDebug("StateMaker Constructor called",10);
}

StateMaker::StateMaker(std::vector<Trace*>* traces,std::vector<Signal>* signals){
    for(auto trace : *traces){
        this->traces.push_back(trace);
    }
    this->signals = signals;
    
    printDebug("StateMaker Constructor called",10);
}

StateMaker::~StateMaker(){
    for(auto state : states){
        delete state;
    }
    
    for(auto value: randomValues){
        delete value;
    }

    printDebug("StateMaker Destructor called",10);
}

State* StateMaker::makeRandomState(){
    //check there is signals to make the state
    if(signals->size() == 0){
        printDebug("No signals to make state",1);
        exit(1);
    }

    State* state = new State();
    state->setSignals(signals);
    for(auto signal : *signals){
        Value* value = Value::makeRandomValue(signal.type,signal.lindex - signal.rindex + 1);
        state->addValue(value);
        randomValues.push_back(value);
    }
    states.push_back(state);
    return state;
}