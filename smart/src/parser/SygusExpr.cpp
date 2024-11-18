#include "SygusExpr.h"
#include "utils.h"
#include <string>
#include <iostream>

SygusOperatorType getOperatorType(std::string op)
{
   
    throw std::invalid_argument("Unknown operator type: " + op);
    //return UNKNOWN;
}

SygusExprType getExprType(std::string expr)
{
    if (expr == "define-fun")
    {
        return FUNCTION; 
    }
    else if (expr == "")
    {
        return LIST;
    }
    else if (expr == "_")
    {
        return BITS_TYPE;
    }
    else
    {
        return IDENTIFIER;
    }
}

SygusExpr::SygusExpr()
{
}

SygusExpr::~SygusExpr()
{
}

SygusIdentifier::SygusIdentifier(std::string name)
{
    this->name = name;
}

std::string SygusIdentifier::toString()
{
    return name;
}

SygusOperator::SygusOperator(SygusOperatorType type)
{
    op = type;
}

std::string SygusOperator::toString()
{
    switch (this->op)
    {
    case DEFINE_FUN:
        return "define-fun";
    default:
        return "Unknown Operator";
    }
}

//TODO:
//this might need a deep copy?
SygusComplexExpr::SygusComplexExpr(SygusOperator* op, std::vector<SygusExpr*> operands)
{
    this->op = op;
    this->operands = operands;
}

void SygusComplexExpr::addOperand(SygusExpr *operand)
{
    this->operands.push_back(operand);
}

std::string SygusComplexExpr::toString()
{
    std::string result = "(";
    
    int index = 0;
    if(operands.size() == 0){
        return "Empty Complex Expression";
    }
    else{
        for (int i =0;i<operands.size();i++){
            result += operands[i]->toString();
            if(i == 1){
                result += " " + op->toString() + " ";
            }
        }
    }
    
    result += ")";
    return result;
}

SygusFunction::SygusFunction(SygusIdentifier* name, std::vector<SygusExpr*> parameters, SygusExpr* body)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
}

std::string SygusFunction::toString()
{
    std::string result = "(define-fun " + name->toString() + " (";
    for (int i = 0; i < parameters.size(); i++)
    {
        result += parameters[i]->toString();
        if (i != parameters.size() - 1)
        {
            result += " ";
        }
    }
    result += ") " + body->toString() + ")";
    return result;
}