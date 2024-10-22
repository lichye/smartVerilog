#if !defined(SignalGather_H)
#define SignalGather_H
#include <iostream>
#include <string>
#include "VCDTypes.hpp"
#include "utils.h"

class SignalGather
{
    public:
        SignalGather();
        SignalGather(std::string);
        SignalGather(std::vector<VCDSignal>);

        ~SignalGather();
        //void parseFile(const char* filename);
        const std::vector<Signal*>* getAllSignals();
        const Signal getOneSignal(int);
    private:
        std::vector<Signal*> signals;

};

#endif // SignalGather_H
