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
    else if(op=="bvneg"){
        return BVNEG;
    }
    else if(op=="bvnot"){
        return BVNOT;
    }
    else if(op=="bvand"){
        return BVAND;
    }
    else if(op=="bvor"){
        return BVOR;
    }
    else if(op=="bvxor"){
        return BVXOR;
    }
    else if(op=="bvmul"){
        return BVMUL;
    }
    else if (op=="bvudiv"){
        return BVDIV;
    }
    else if(op=="not"){
        return BOOLNOT;
    }
    else if(op=="or"){
        return BOOLOR;
    }
    else if(op=="and"){
        return BOOLAND;
    }
    else if(op=="bvult"){
        return BVULT;
    }
    else if(op=="bvule"){
        return BVULE;
    }
    else if(op=="bvugt"){
        return BVUGT;
    }
    else if(op=="bvuge"){
        return BVGUE;
    }
    else if(op=="bvslt"){
        return BVSLT;
    }
    else if(op=="bvsle"){
        return BVSLE;
    }
    else if(op=="bvsgt"){
        return BVSGT;
    }
    else if(op=="bvsge"){
        return BVSGE;
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

//SygusIdentifier
SygusIdentifier::SygusIdentifier(std::string name)
{
    this->name = name;
}

SygusIdentifier::~SygusIdentifier()
{
}

std::string SygusIdentifier::toString()
{
    return name;
}

//SygusOperator
SygusOperator::SygusOperator(SygusOperatorType type)
{
    op = type;
}

SygusOperator::~SygusOperator()
{
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
        case BVNEG:
            return "-";
        case BVNOT:
            return "!";
        case BVOR:
            return "|";
        case BVXOR:
            return "^";
        case BVAND:
            return "&";
        case BVMUL:
            return "*";
        case BVDIV:
            return "/";
        case BOOLNOT:
            return "!";
        case BOOLOR:
            return "||";
        case BOOLAND:
            return "&&";
        case BVULT:
            return "<";
        case BVULE:
            return "<=";
        case BVUGT:
            return ">";
        case BVGUE:
            return ">=";
        case BVSLT:
            return "s<";
        case BVSLE:
            return "s<=";
        case BVSGT:
            return "s>";
        case BVSGE:
            return "s>=";
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
        case BVNEG:
            return 1;
        case BVNOT:
            return 1;
        case BVOR:
            return 2;
        case BVXOR:
            return 2;
        case BVMUL:
            return 2;
        case BVAND:
            return 2;
        case BVDIV:
            return 2;
        case BOOLNOT:
            return 1;
        case BOOLOR:
            return 2;
        case BOOLAND:
            return 2;
        case BVULT:
            return 2;
        case BVULE:
            return 2;
        case BVUGT:
            return 2;
        case BVGUE:
            return 2;
        case BVSLT:
            return 2;
        case BVSLE:
            return 2;
        case BVSGT:
            return 2;
        case BVSGE:
            return 2;
        default:
            throw std::invalid_argument("Unknown operator type: " + std::to_string(op));
    }
}

//SygusComplexExpr 
SygusComplexExpr::SygusComplexExpr(SygusOperator* op)
{
    this->op = op;
    this->operands = operands;
}

SygusComplexExpr::~SygusComplexExpr()
{
    delete op;
    for(auto operand : operands){
        delete operand;
    }
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
        if(op->getOperandsNumber() == 1){
            result += op->toString() + " " + operands[0]->toString();
        }
        else if(op->getOperandsNumber() == 2){
            result += operands[0]->toString() + " " + op->toString() + " " + operands[1]->toString();
        }
        else{
            throw std::invalid_argument("Unknown operator type: " + std::to_string(op->getOperandsNumber()));
        }
    }
    
    result += ")";
    return result;
}

//SygusVariableType

SygusVariableType::SygusVariableType()
{
}

SygusVariableType::~SygusVariableType()
{
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

SygusBoolType::~SygusBoolType()
{
}

std::string SygusBoolType::toString()
{
    return "Bool";
}

//SygusVariableList
SygusVariableList::SygusVariableList()
{
}

SygusVariableList::~SygusVariableList()
{
    for(auto variable : variables){
        delete variable;
    }
    for(auto type : types){
        delete type;
    }
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

SygusIntValue::~SygusIntValue()
{
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

//SygusFunction
SygusFunction::SygusFunction(SygusIdentifier* name, SygusVariableList* parameters, SygusExpr* body)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
}

SygusFunction::~SygusFunction()
{
    delete name;
    delete parameters;
    delete body;
}

std::string SygusFunction::toString()
{
    std::string result;
    result+="function id : " + name->toString() + "\n";
    result+="parameters : " + parameters->toString() + "\n";
    result+="body : " + body->toString() + "\n";
    return result;
}

SygusExpr* SygusFunction::getBody()
{
    return body;
}

std::string SygusFunction::getBodyVerilogExpr()
{
    return body->toString();
}
