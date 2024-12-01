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
        StateMaker(std::vector<Trace*>*,std::vector<Signal>*);
        ~StateMaker();
        State* makeRandomState();
    private:
        std::vector<Trace*> traces;
        std::vector<Signal>* signals;
        int seed;
        //this is random values gathered
        std::vector<Value*> randomValues;
        void checkState();
};
#endif // STATEMAKER_H