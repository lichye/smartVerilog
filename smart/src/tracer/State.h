#ifndef STATE_H
#define STATE_H

#include <vector>
#include <string>
#include <utility>
#include "utils.h"
#include "Value.h"  


class State
{
    public:
        State();
        State(bool);
        ~State();
        std::string toString();
        std::string toVerilogExpr();
        //std::string toSyGusString();
        //std::string toVerilogExpr();
        void addValue(Value*);
        void addValue(Signal,Value*);
        

        bool isUndefined();
        void setSignals(std::vector<Signal>*);
    private:
        bool signalAllExist;
        bool reachable;//if the state is reachable from model
        std::vector<Signal>* signals;
        std::vector<Value*> values;
};
#endif // STATEMAKER_H