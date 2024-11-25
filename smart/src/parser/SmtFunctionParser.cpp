#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include "SygusExpr.h"
#include "SmtFunctionParser.h"

SmtFunctionParser::SmtFunctionParser()
{
}

SmtFunctionParser::~SmtFunctionParser()
{
}

SygusExpr* SmtFunctionParser::parseSmtFunction(std::string function) {
    if (function.find("(define-fun") <0) {
        throw std::invalid_argument("Input is not a valid SMT-LIB function definition.");
    }

    size_t bodyStart = function.find('(', function.find("(define-fun"));
    size_t bodyEnd = function.rfind(')');
    std::string body = function.substr(bodyStart, bodyEnd - bodyStart);

    // Parse the function body
    printDebug("We parse the function: " + body + "\n",2);
    std::istringstream stream(body);
    return parseExpression(stream);
}

SygusExpr* SmtFunctionParser::parseExpression(std::istringstream& stream) {
    skipWhitespace(stream);
    //print("stream after skipping whitespace: " + stream.str() + "\n");
    char ch = stream.peek();

    std::string parsedChar(1, ch);

    if (ch == '(') {
        stream.get();
        std::string opToken = parseToken(stream);
        SygusExprType exprType = getExprType(opToken);

        if (exprType == FUNCTION) {
            SygusIdentifier* id = (SygusIdentifier*)parseExpression(stream);
            SygusVariableList* parameter_list = (SygusVariableList*)parseExpression(stream);
            SygusExpr* return_type = parseExpression(stream);
            SygusExpr* body = parseExpression(stream);
            SygusFunction* function = new SygusFunction(id, parameter_list, body);
            stream.get(); // Consume ')'
            printDebug("We parse a function : \n" + function->toString() + "\n",1);
            return function;
        }
        else if(exprType == LIST){
            printDebug("We meet a list expression\n",3);
            SygusVariableList* list = new SygusVariableList();
            while(stream.peek() != ')'){
                SygusExpr* id = parseExpression(stream);
                list->addVariable((SygusIdentifier*)id);
                SygusExpr* type = parseExpression(stream);
                list->addType((SygusVariableType*)type);
                stream.get(); // Consume ')' of the identifier declaration      
            }
            stream.get(); // Consume ')'
            printDebug("We parsed a list expression: "+list->toString()+"\n",2);
            return list;
        }
        else if(exprType == IDENTIFIER){
            printDebug("We meet an identifier declaration: "+opToken+"\n",3);
            return new SygusIdentifier(opToken);
        }    
        else if(exprType == BITS_TYPE){
            printDebug("We meet a bits type\n",3);

            SygusExpr* bitvec = parseExpression(stream);
            SygusIntValue* bitlength = (SygusIntValue*)parseExpression(stream);

            int bitlengthValue = bitlength->getValue();
            SygusBitsType* bitsType = new SygusBitsType(bitlengthValue);

            
            stream.get(); // Consume ')' of the bits type
            return bitsType;
        }
        else if(exprType == COMPLEXEXPR){
            printDebug("We meet a complex expression\n",3);
            SygusOperatorType opType = getOperatorType(opToken);

            SygusOperator* op = new SygusOperator(opType);
            SygusComplexExpr* complexExpr = new SygusComplexExpr(op);
            int operands = op->getOperandsNumber();
            if(operands==1){
                SygusExpr* operand = parseExpression(stream);
                complexExpr->addOperand(operand);
            }
            else if(operands==2){
                SygusExpr* operand1 = parseExpression(stream);
                complexExpr->addOperand(operand1);
                SygusExpr* operand2 = parseExpression(stream);
                complexExpr->addOperand(operand2);
            }
            else{
                throw std::invalid_argument("Unknown operator type: " + opToken);
            }
            stream.get(); // Consume ')'
            return complexExpr;
        }
        else{
            throw std::invalid_argument("Unknown Mixed type: " + opToken);
        }
        
    }
    else {
        // Identifier or literal (e.g., x, y, z)
        std::string token = parseToken(stream);
        SygusExprType exprType = getExprType(token);
        if(exprType == IDENTIFIER){
            printDebug("the token has type of Identifier: "+token+"\n",3);
            return new SygusIdentifier(token);
        } 
        else if(exprType == BOOL_TYPE){
            printDebug("the token has type of BoolType: "+token+"\n",3);
            SygusBoolType* boolType = new SygusBoolType;
            return boolType;
        }
        else if(exprType == BOOL_VALUE){
            printDebug("the token has type of BoolValue: "+token+"\n",3);
            bool tokenBool;
            if(token=="true"){
                tokenBool = true;
            }
            else if(token=="false")
            {
                tokenBool = false;
            }
            else{
                throw std::invalid_argument("Unknown boolean value: " + token);
            }
            SygusBoolValue* value = new SygusBoolValue(tokenBool);
            return value;
        }    
        else if (exprType == INT_VALUE){
            printDebug("the token has type of IntValue: "+token+"\n",3);
            int value = std::stoi(token);
            SygusIntValue* intValue = new SygusIntValue(value);
            return intValue;
        }
        else{
            throw std::invalid_argument("Unknown single type: " + token);
        }
    }
    return nullptr;
}

std::string SmtFunctionParser::parseToken(std::istringstream& stream) {
    skipWhitespace(stream);
    std::string token;
    char ch;
    while (stream.get(ch)) {
        if (std::isspace(ch) || ch == '(' || ch == ')') {
            stream.putback(ch);
            break;
        }
        token += ch;
    }
    printDebug("we parsed token: " + token + "\n",3);
    return token;
}

void SmtFunctionParser::skipWhitespace(std::istringstream& stream) {
    while (std::isspace(stream.peek())) {
        stream.get();
    }
}