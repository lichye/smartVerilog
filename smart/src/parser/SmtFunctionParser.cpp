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
    std::istringstream stream(body);
    return parseExpression(stream);
}

SygusExpr* SmtFunctionParser::parseExpression(std::istringstream& stream) {
    skipWhitespace(stream);
    //print("stream after skipping whitespace: " + stream.str() + "\n");
    char ch = stream.peek();

    std::string parsedChar(1, ch);

    print("ParseExpression is called");
    print("we parsed to ch: " + parsedChar + "\n");

    if (ch == '(') {
        // Complex expression (e.g., (= x (bvsub y z)))
        stream.get(); // Consume '('
        std::string opToken = parseToken(stream);
        SygusExprType exprType = getExprType(opToken);

        if (exprType == FUNCTION) {
            SygusExpr* id = parseExpression(stream);
            print("In the function, we get id as: " + id->toString() + "\n");
            SygusExpr* parameter_list = parseExpression(stream);
            exit(0);
            stream.get(); // Consume ')'
        }
        else if(exprType == LIST){
            print("We meet a list expression\n");
            while(stream.peek() != ')'){
                SygusExpr* expr = parseExpression(stream);
                print("We get an expression: "+expr->toString()+"\n");
            }
            stream.get(); // Consume ')'

            std::string nextToken = parseToken(stream);
            print("Next token is: "+nextToken+"\n"); 
            
            exit(0);
        }
        else if(exprType == IDENTIFIER){
            print("We meet an identifier declaration: "+opToken+"\n");
            
            SygusExpr* idType = parseExpression(stream);

            stream.get(); // Consume ')' of the identifier declaration
            return new SygusIdentifier(opToken);
        }
        //TODO:
        //Make correct Types
        else if(exprType == BITS_TYPE){
            print("We meet a bits type declaration: "+opToken+"\n");
            SygusExpr* bitvec = parseExpression(stream);
            SygusExpr* bitlength = parseExpression(stream);
            stream.get(); // Consume ')' of the bits type
            return nullptr;
        }
        else{
            throw std::invalid_argument("Unknown Mixed type: " + opToken);
        }
        
    }
    else if(ch == ')'){
        stream.get(); // Consume ')'
        return nullptr;
    } 
    else {
        // Identifier or literal (e.g., x, y, z)
        std::string token = parseToken(stream);
        SygusExprType exprType = getExprType(token);
        if(exprType == IDENTIFIER){
            print("We meet an identifier: "+token+"\n");
            return new SygusIdentifier(token);
        } 
            
        else{
            throw std::invalid_argument("Unknown single type: " + token);
        }
    }
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
    print("we parsed token: " + token + "\n");
    return token;
}

void SmtFunctionParser::skipWhitespace(std::istringstream& stream) {
    while (std::isspace(stream.peek())) {
        stream.get();
    }
}