#ifndef SMT_FUNCTION_PARSER_H
#define SMT_FUNCTION_PARSER_H
#include <vector>
#include "utils.h"
#include "setups.h"
#include "SygusExpr.h"
class SmtFunctionParser {
    public:
    SmtFunctionParser();
    ~SmtFunctionParser();
    SygusExpr* parseSmtFunction(std::string);

    private:
    std::vector<Signal> signals;
    SygusExpr* parseExpression(std::istringstream& stream);
    std::string parseToken(std::istringstream& stream);
    void skipWhitespace(std::istringstream& stream);
};

#endif