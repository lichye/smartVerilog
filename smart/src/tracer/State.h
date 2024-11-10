#ifndef STATEMAKER_H
#define STATEMAKER_H

#include <vector>
#include <string>
#include <utility>
#include "Trace.h"
#include "utils.h"


class State
{
    public:
        State();
        State(bool);
        ~State();
        std::string toString();
        //std::string toSyGusString();
        //std::string toVerilogExpr();
        void addValue(Signal,Value*);
        void addValue(Value*);

        bool isUndefined();
        void setSignals(std::vector<Signal>*);
    private:
        bool signalAllExist;
        bool reachable;//if the state is reachable from model
        std::vector<Signal>* signals;
        std::vector<Value*> values;
};

class StateMaker
{
    public:
        StateMaker();
        StateMaker(std::vector<Signal>*);
        StateMaker(std::vector<Trace*>*,std::vector<Signal>*);
        ~StateMaker();
        State* makeRandomState();
        
    private:
        std::vector<Trace*> traces;
        std::vector<Signal>* signals;
        std::vector<State*> states;

        //this is random values gathered
        std::vector<Value*> randomValues;
        void checkState();
};



#endif // STATEMAKER_H