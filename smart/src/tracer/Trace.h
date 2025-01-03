#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "VCDFile.hpp"
#include "VCDValue.hpp"
#include "Value.h"
#include "SignalGather.h"
#include "VCDFileParser.hpp"
#include "State.h"

#ifndef TRACE_H
#define TRACE_H
enum    TraceType{
        SIM,
        SMT,
        UNKNOWN
};

class   Trace{
    public:
        Trace();
        
        Trace(TraceType type, std::string path);
        
        ~Trace();
        
        std::vector<std::vector<Value*>>* getConstraints(std::vector<Signal>*);
        
        //get the signal values from the trace
        std::vector<Value*>* getSignalValue(Signal);

        std::vector<Signal>* getAllSignals();

        std::vector<Signal>* getAllSignals(std::string);

        std::string getPath();

        bool checkStateExist(State*);

    private:
        //variables:
        static std::mutex VCDFileParserMutex;

        TraceType traceType;

        std::string vcdPath;

        std::string smtPath;

        int valueLength;

        //major data structure to store the signal values
        std::map<Signal, std::vector<Value*>*> signals_map;
        
        //another data structure to store the signal values
        std::vector<State*> states;
        
        //private functions:
        void readVCDFile(VCDFile*);
        
        SignalType translateSignalType(VCDSignal*);

        Signal createSignal(VCDSignal*,std::string);

};

#endif // TRACE_H