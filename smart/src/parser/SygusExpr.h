#ifndef SygusExpr_H
#define SygusExpr_H
#include <string> 
#include <vector>

enum SygusOperatorType
{   
    UNKNOWN_SygusOperatorType = 0,
    DEFINE_FUN = 1,
    PARAMETER_LIST_OP = 2,
    SYGUS_IDENTIFIER = 3,
};

enum SygusExprType
{
    IDENTIFIER = 1,
    OPERATOR = 2,
    COMPLEXEXPR = 3,
    FUNCTION = 4,
    LIST =5,
    BITS_TYPE = 6,
};

SygusOperatorType getOperatorType(std::string);
SygusExprType getExprType(std::string);

class SygusExpr{
    public:
        SygusExpr();
        ~SygusExpr();
        virtual std::string toString() = 0;
};

class SygusIdentifier: public SygusExpr
{
    public:
        SygusIdentifier(std::string name);
        ~SygusIdentifier();
        std::string toString();
    private:
        std::string name;
};

class SygusOperator: public SygusExpr
{
    public:
        SygusOperator(SygusOperatorType);
        ~SygusOperator();
        std::string toString();
    
    private:
        SygusOperatorType op;

};

class SygusComplexExpr: public SygusExpr
{
    public:
        SygusComplexExpr(SygusOperator*,std::vector<SygusExpr*>);
        ~SygusComplexExpr();
        std::string toString();
        void addOperand(SygusExpr* operand);

    private:
        SygusOperator *op;
        std::vector<SygusExpr*> operands;
};

class SygusFunction: public SygusExpr
{
    public:
        SygusFunction(SygusIdentifier*,std::vector<SygusExpr*>,SygusExpr*);
        ~SygusFunction();
        std::string toString();
        void addParameter(SygusExpr* parameter);

    private:
        SygusIdentifier *name;
        std::vector<SygusExpr*> parameters;
        SygusExpr *body;
};


#endif