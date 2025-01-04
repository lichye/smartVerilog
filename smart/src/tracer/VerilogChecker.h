#ifndef VERILOG_MAKER_H
#define VERILOG_MAKER_H
#include <string>
#include <iostream>
#include <vector>
#include "State.h"
#include "SygusExpr.h"
#include "Module.h"
enum PropertyType{
    
    SAFT_PROPERTY,
    //  Ensures that the system always meets certain safety conditions and does not enter illegal states. 
    //  @(posedge clk) enable |-> (output_signal == expected_value);

    
    LIVENESS_PROPERTY,
    // When a signal is asserted, the system must respond within a specific number of cycles.
    // @(posedge clk) request ##[1:5] acknowledge;
    
    MUTUAL_EXCLUSION_PROPERTY,
    //  Ensures that two signals are never asserted at the same time.
    //  !(signal_A && signal_B);
    
    REACHABILITY_PROPERTY
    //  Ensures that a certain state is reachable from the initial state.
    //  (a == 0 && b == 1)
};

enum BackEndSolver{
    EBMC,
    SBY
};

class VerilogChecker {
    public:
        VerilogChecker();
        VerilogChecker(std::string, BackEndSolver);
        VerilogChecker(std::string,std::string,BackEndSolver);
        ~VerilogChecker();
        
        void setVerilogSrcPath(std::string);
        void setformalFilePath(std::string);
        void setTracePath(std::string);

        void addProperty(State*,PropertyType);
        void addProperty(SygusFunction*,PropertyType);
        void cleanProperties();
        
        //The check function will return true if the state is reachable
        bool checkStateReachability(State*);
        bool checkExprSafety(SygusFunction*,std::string);
        void setBound(int);
        void setTopModule(std::string);
        void setModuleTime(std::string);
        void setHomePath(std::string);
        Constrains fixupConstrains(Constrains);
        void setSignals(std::vector<Signal>*);
    private:
        int bound;
        //BackEndSolver
        BackEndSolver solver;

        //parameters of ebmc
        std::string verilogSrcPath;
        std::string homePath;

        
        std::string formalFilePath;
        std::string tracePath;
        std::string topModule;
        std::string moduleTime;

        std::vector<std::string> properties;
        std::vector<PropertyType> propertyTypes;
        // std::vector<std::string> formalFilePaths;
        std::vector<std::string> relatedFilePaths;
        std::vector<Signal>* signals;

        void writeVerilogFile();
        bool runEBMC();
        bool runEBMC(std::string);
        bool runSby();

        std::string generateFormalFilePath(PropertyType);
        State* makeReachableState(std::vector<Value*>);
};

#endif