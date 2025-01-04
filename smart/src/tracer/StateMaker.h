#ifndef STATEMAKER_H
#define STATEMAKER_H

#include "Trace.h"
#include "utils.h"
#include "State.h"
class StateMaker
{
    public:
        StateMaker();
        StateMaker(std::vector<Signal>*);
        ~StateMaker();
        State* makeRandomState();
        State* fixUpState(std::vector<Value*>);
        State* fixUpState(State*,std::vector<Value*>);
        static void setSeed(int);

    private:
        std::vector<Signal>* signals;
        //this is random values gathered
        std::vector<Value*> randomValues;
        void checkState();
        void readState();
};
#endif // STATEMAKER_H