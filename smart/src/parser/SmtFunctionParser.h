#ifndef SMT_FUNCTION_PARSER_H
#define SMT_FUNCTION_PARSER_H
#include <vector>
#include <algorithm>
#include "Utils.h"
#include "setups.h"
#include "SygusExpr.h"
class SmtFunctionParser {
    public:
    SmtFunctionParser();
    ~SmtFunctionParser();
    SygusExpr* parseSmtFunction(std::string,bool);

    private:
    std::vector<Signal> signals;
    SygusExpr* parseExpression(std::istringstream& stream,bool);
    std::string parseToken(std::istringstream& stream);
    void skipWhitespace(std::istringstream& stream);
    std::string cleanLatency(std::string);
    int latency;
};

#endif