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
    printDebug("Signals are not null",2);
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
