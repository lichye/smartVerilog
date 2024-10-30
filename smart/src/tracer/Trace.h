
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "VCDFile.hpp"
#include "VCDValue.hpp"
#include "Value.h"
#include "SignalGather.h"
#include "VCDFileParser.hpp"

#ifndef TRACE_H
#define TRACE_H
enum TraceType{
    VCD,
    SMT,
    UNKNOWN
};

class Trace{
    public:
        Trace();
        Trace(TraceType type, std::string path);
        ~Trace();
        
        
        //get the constraints from the trace
        std::vector<std::vector<Value*>>* getConstraints(std::vector<Signal>);
        std::vector<std::vector<Value*>>* getConstraints(std::vector<Signal>*);
        
        //get the signal values from the trace
        std::vector<Value*>* getSignalValue(Signal);

        std::vector<Signal>* getAllSignals();

        void printDebug();
    private:

        static std::mutex VCDFileParserMutex;

        TraceType traceType;

        

        std::string vcdPath;

        //if trace type is SMT
        std::string smtPath;

        std::map<Signal, std::vector<Value*>*> signals_map;
        
        //use this function to set the signals_map
        void readVCDFile(VCDFile*);
        SignalType translateSignalType(VCDSignal*);

};

#endif // TRACE_H