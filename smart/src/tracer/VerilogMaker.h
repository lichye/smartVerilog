#ifndef VERILOG_MAKER_H
#define VERILOG_MAKER_H
#include <string>
#include <iostream>
#include <vector>
#include "State.h"
enum ExprType{
    UNREACHABLE_STATE,
    UNKOWN
};
class VerilogMaker {
    public:
        VerilogMaker();
        ~VerilogMaker();
        void addExprToVerilog(std::string,std::string);
        void storeExpr(State*,ExprType);
    private:
        std::vector<std::string> exprs;
        std::vector<ExprType> exprTypes;
};

#endif