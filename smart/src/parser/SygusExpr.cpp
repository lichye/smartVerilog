#include "SygusExpr.h"
#include "utils.h"
#include <string>
#include <cassert>
#include <iostream>
#include <cctype>

SygusOperatorType getOperatorType(std::string op)
{
    if(op=="="){
        return EQUAL;
    }
    else if(op=="bvadd"){
        return BVADD;
    }
    else if(op=="bvsub"){
        return BVSUB;
    }
    else
        return UNKNOWOPERATOR;
}

SygusExprType getExprType(std::string expr)
{
    if (expr == "define-fun")
    {
        return FUNCTION; 
    }
    else if (getOperatorType(expr)!=-1)
    {
        return COMPLEXEXPR;
    }
    else if (expr == "")
    {
        return LIST;
    }
    else if (expr == "_")
    {
        return BITS_TYPE;
    }
    else if (expr == "Bool")
    {
        return BOOL_TYPE;
    }
    else if (expr == "true" || expr == "false")
    {
        return BOOL_VALUE;
    }
    else if(expr.size() > 0 && std::isdigit(expr[0])){
            return INT_VALUE;
    }
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
        case EQUAL:
            return "==";
        case BVADD:
            return "+";
        case BVSUB:
            return "-";
        default:
            return "Unknown Operator";
    }
}

int SygusOperator::getOperandsNumber()
{
    switch (this->op)
    {
        case EQUAL:
            return 2;
        case BVADD:
            return 2;
        case BVSUB:
            return 2;
        
        default:
            throw std::invalid_argument("Unknown operator type: " + std::to_string(op));
    }
}

SygusComplexExpr::SygusComplexExpr(SygusOperator* op)
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
            if(i == 0){
                result += " " + op->toString() + " ";
            }
        }
    }
    
    result += ")";
    return result;
}

SygusFunction::SygusFunction(SygusIdentifier* name, SygusVariableList* parameters, SygusExpr* body)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
}

std::string SygusFunction::toString()
{
    std::string result;
    result+="function id : " + name->toString() + "\n";
    result+="parameters : " + parameters->toString() + "\n";
    result+="body : " + body->toString() + "\n";
    return result;
}

//SygusBitsVariables
SygusBitsType::SygusBitsType(int bitlength)
{
    this->bitlength = bitlength;
}

std::string SygusBitsType::toString()
{
    return " Bits " + std::to_string(bitlength);
}

SygusBitsType::~SygusBitsType()
{
}

//SygusBoolType

SygusBoolType::SygusBoolType()
{
}

std::string SygusBoolType::toString()
{
    return "Bool";
}

SygusBoolType::~SygusBoolType()
{
}

//SygusVariableList

SygusVariableList::SygusVariableList()
{
}

SygusVariableList::~SygusVariableList()
{
}

std::string SygusVariableList::toString()
{   
    std::string result;
    assert(variables.size()==types.size());
    for(int i=0;i<variables.size();i++){
        result += "(" + variables[i]->toString() + " " + types[i]->toString() + ")";
    }
    return result;
}

void SygusVariableList::addVariable(SygusIdentifier *variable)
{
    this->variables.push_back(variable);
}

void SygusVariableList::addType(SygusVariableType *type)
{
    this->types.push_back(type);
}

//SygusValue
SygusValue::SygusValue(){

}

SygusValue::~SygusValue()
{
}

//SygusIntValue
SygusIntValue::SygusIntValue(std::string value)
{
    this->value = std::stoi(value);
}

SygusIntValue::SygusIntValue(int value)
{
    this->value = value;
}

std::string SygusIntValue::toString()
{
    return std::to_string(value);
}

int SygusIntValue::getValue()
{
    return value;
}

//SygusBoolValue

SygusBoolValue::SygusBoolValue(bool inputValue)
{
    this->value = inputValue;
}

SygusBoolValue::~SygusBoolValue()
{
}

std::string SygusBoolValue::toString()
{
    return value ? "true" : "false";
}

bool SygusBoolValue::getValue()
{
    return value;
}