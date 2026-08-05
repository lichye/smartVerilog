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
        
        // hierarchyRoot names the VCD scope whose whole subtree becomes
        // candidate material. Empty (the default) keeps the historical
        // behaviour: a signal belongs to the scope that declares it and
        // nothing below the mined scope is visible.
        Trace(TraceType type, std::string path, std::string hierarchyRoot = "");

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

        // "" disables hierarchical naming; see the constructor.
        std::string hierarchyRoot;

        int valueLength;

        //major data structure to store the signal values
        std::map<Signal, std::vector<Value*>*> signals_map;
        
        //another data structure to store the signal values
        std::vector<State*> states;
        
        //private functions:
        void readVCDFile(VCDFile*);
        
        SignalType translateSignalType(VCDSignal*);

        // instancePath is the dotted path from the hierarchy root down to the
        // signal's own scope, exclusive of both ends ("" for a signal in the
        // root scope, "U0" one level down). It becomes the signal's name
        // prefix, which is exactly the Verilog hierarchical reference the
        // assertions are written with.
        Signal createSignal(VCDSignal*,const std::string& moduleName,
                            const std::string& instancePath);

        // moduleName/instancePath for every scope in the file, resolved once
        // per trace because the state loop revisits every scope per timestamp.
        std::map<const VCDScope*,std::pair<std::string,std::string>>
            scopeNaming(VCDFile*) const;

};

#endif // TRACE_H