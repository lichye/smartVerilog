#include "StateMaker.h"

// StateMaker class function definitions below
StateMaker::StateMaker(){
    this->signals = nullptr;
    printDebug("StateMaker with no arguments Constructor called",10);
}

StateMaker::StateMaker(std::vector<Signal>* signals){
    this->signals = signals;
    printDebug("StateMaker Constructor called",10);
}


StateMaker::~StateMaker(){
    printDebug("StateMaker Destructor called",10);
}

State* StateMaker::makeRandomState(){
    //check there is signals to make the state
    printDebug("Call StateMaker::makeRandomState",2);
    assert(signals != nullptr);
    // printDebug("Signals are not null",2);
    if(signals->size() == 0){
        printDebug("No signals to make state",1);
        exit(1);
    }

    State* state = new State();
    state->setSignals(signals);

    for(auto signal : *signals){
        Value *value = Value::makeRandomValue(signal.type,signal.lindex - signal.rindex + 1);
        
        state->addValue(value);
        delete value;
    }
    return state;
}

void StateMaker::setSeed(int seed){
    Value::setSeed(seed);
}

State* StateMaker::fixUpState(std::vector<Value*> values){
    assert(values.size() == signals->size());
    State* ret_state = new State();
    ret_state->setSignals(signals);
    for(int i = 0; i < values.size(); i++){
        if(values[i]->isUndefined()){
            Value *value = Value::makeRandomValue((*signals)[i].type,(*signals)[i].lindex - (*signals)[i].rindex + 1);
            ret_state->addValue(value);
            delete value;
        }
        else{
            ret_state->addValue(values[i]);
        }
    }
    return ret_state;
}

State* StateMaker::fixUpState(State* state,std::vector<Value*> values){
    assert(values.size() == signals->size());
    State* ret_state = new State();
    ret_state->setSignals(signals);
    for(int i = 0; i < values.size(); i++){
        if(values[i]->isUndefined()){
            Value* nowValue = state->getValues()[i];
            Value *newValue = Value::makeNewValue(nowValue,(*signals)[i].type,(*signals)[i].lindex - (*signals)[i].rindex + 1);
            ret_state->addValue(newValue);
        }
        else{
            ret_state->addValue(values[i]);
        }
    }
    return ret_state;
}