
#include <string>
#include <vector>
#include <map>
#include "VCDFile.hpp"
#include "VCDValue.hpp"
#include "Value.h"
enum TraceType{
    VCD,
    SMT,
    UNKNOWN
};

typedef struct{
    std::string name;
    SignalType type;
    int lindex;
    int rindex;
}Signal;

class Trace{
    public:
        Trace();
        Trace(TraceType type, std::string path);
        ~Trace();
        
        void printScope();
        
        //get the constraints from the trace
        std::vector<std::vector<Value*>> getConstraints(std::vector<Signal>);
        
        //get the signal values from the trace
        std::vector<Value*> getSignals(Signal);
    private:
        TraceType traceType;

        //if trace type is VCD
        VCDFile* vcdFile;
        std::string vcdPath;

        //if trace type is SMT
        std::string smtPath;

        std::map<Signal, std::vector<Value*>> signals_map;


};