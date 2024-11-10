#ifndef VERILOGEXPR_H
#define VERILOGEXPR_H
#include <string>
#include <vector>
#include <map>
#include <set>

enum VerilogExprType
{ 
    UNKNOWNTYPE = 1,
    IDENTIFIER = 2,
    BITS = 3,
    COMPLEXEXPR = 4,
    OPERANDS = 5
};

enum VerilogOperandType
{
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    MOD = 5,
    AND = 6,
    OR = 7,
    XOR = 8,
    NOT = 9,
    SHL = 10,
    SHR = 11,
    LSHR = 12,
    EQ = 13,
    NEQ = 14,
    LT = 15,
    GT = 16,
    LTE = 17,
    GTE = 18,
    LAND = 19,
    LOR = 20,
    LNOT = 21,
    TERNARY = 22
};

class VerilogExpr
{
    public:
        VerilogExpr();
        ~VerilogExpr();
        std::string toString();
        VerilogExprType getType();
    private:
        VerilogExprType type;
};

class VerilogIdentifier: public VerilogExpr
{
    public:
        VerilogIdentifier(std::string name);
        ~VerilogIdentifier();
        std::string toString();
        VerilogExprType getType();
    private:
        std::string name;
};

class VerilogComplexExpr: public VerilogExpr
{
    public:
        VerilogComplexExpr(std::string op, std::vector<VerilogExpr*> operands);
        ~VerilogComplexExpr();
        std::string toString();
        VerilogExprType getType();
    private:
        VerilogOperandType op;
        std::vector<VerilogExpr*> operands;
};

class VerilogBool: public VerilogExpr
{
    public:
        VerilogBool(bool value);
        ~VerilogBool();
        std::string toString();
        VerilogExprType getType();
    private:
        bool value;
};

class VerilogBits: public VerilogExpr
{
    public:
        VerilogBits(int width);
        ~VerilogBits();
        std::string toString();
        VerilogExprType getType();
    private:
        int width;
        int value;
};



#endif // VERILOGEXPR_H