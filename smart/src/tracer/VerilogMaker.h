#ifndef VERILOG_MAKER_H
#define VERILOG_MAKER_H
#include <string>
#include <iostream>
#include <vector>
#include "State.h"
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
class VerilogMaker {
    public:
        VerilogMaker();
        ~VerilogMaker();
        void writeVerilogFile(std::string,std::string);
        void addProperty(State*,PropertyType);
    private:
        std::vector<std::string> properties;
        std::vector<PropertyType> propertyTypes;
};

#endif