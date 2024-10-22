
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

struct Signal{
    std::string name;
    std::string moduleName;
    SignalType type;
    int lindex;
    int rindex;
    bool operator<(const Signal& rhs) const
    {   
        return name != rhs.name ? name < rhs.name :
               moduleName != rhs.moduleName ? moduleName < rhs.moduleName :
               type != rhs.type ? type < rhs.type :
               lindex != rhs.lindex ? lindex < rhs.lindex :
               rindex < rhs.rindex;
    }
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