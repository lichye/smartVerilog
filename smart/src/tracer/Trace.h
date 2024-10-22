
#include <string>
#include <vector>
#include <map>
#include "VCDFile.hpp"
#include "VCDValue.hpp"
#include "Value.h"
#include "SignalGather.h"
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
        
        void printScope();
        
        //get the constraints from the trace
        std::vector<std::vector<Value*>>* getConstraints(std::vector<Signal>);
        
        //get the signal values from the trace
        std::vector<Value*>* getSignalValue(Signal);
    private:
        TraceType traceType;

        //if trace type is VCD
        VCDFile* vcdFile;
        std::string vcdPath;

        //if trace type is SMT
        std::string smtPath;

        std::map<Signal, std::vector<Value*>*> signals_map;


};