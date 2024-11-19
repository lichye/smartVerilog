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
    BOOL_TYPE = 7,
    INT_VALUE = 8,
    BOOL_VALUE = 9,
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

class SygusVariableType: public SygusExpr
{
    public:
        SygusVariableType();
        ~SygusVariableType();
        virtual std::string toString()=0;
};

class SygusBitsType: public SygusExpr
{
    public:
        SygusBitsType(int);
        ~SygusBitsType();
        std::string toString();

    private:
        int bitlength;
};

class SygusBoolType: public SygusExpr
{
    public:
        SygusBoolType();
        ~SygusBoolType();
        std::string toString();
};

class SygusVariableList: public SygusExpr
{
    public:
        SygusVariableList();
        ~SygusVariableList();
        std::string toString();
        void addVariable(SygusIdentifier* variable);
        void addType(SygusVariableType* type);
    private:
        std::vector<SygusIdentifier*> variables;
        std::vector<SygusVariableType*> types;
};

class SygusValue: public SygusExpr
{
    public:
        SygusValue();
        ~SygusValue();
        virtual std::string toString()=0;
};

class SygusIntValue: public SygusValue
{
    public:
        SygusIntValue(std::string);
        SygusIntValue(int);
        ~SygusIntValue();
        std::string toString();
        int getValue();
    private:
        int value;
};

class SygusBoolValue: public SygusValue
{
    public:
        SygusBoolValue(bool);
        ~SygusBoolValue();
        std::string toString();
        bool getValue();
    private:
        bool value;
};

#endif