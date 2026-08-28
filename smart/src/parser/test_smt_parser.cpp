#include <iostream>
#include <string>

#include "SmtFunctionParser.h"

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (condition) {
        std::cout << "ok  " << what << "\n";
    } else {
        std::cout << "FAIL " << what << "\n";
        ++failures;
    }
}

void testValidFunction() {
    SmtFunctionParser parser;
    SygusExpr* expression = parser.parseSmtFunction(
        "(define-fun inv ((a Bool) (b (_ BitVec 4))) Bool "
        "(and a (= b b)))",
        false);
    auto* function = dynamic_cast<SygusFunction*>(expression);
    check(function != nullptr, "define-fun parses as a function");
    if (function != nullptr)
        check(function->getBodyVerilogExpr() == "(a && (b == b))",
              "boolean and bit-vector parameter types are preserved");
    delete expression;
}

void testInvalidFunction() {
    SmtFunctionParser parser;
    bool threw = false;
    try {
        delete parser.parseSmtFunction("sat", false);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "non-function solver output is rejected explicitly");
}

void testUnsignedComparisonsStayUnsignedInSystemVerilog() {
    SmtFunctionParser parser;
    SygusExpr* expression = parser.parseSmtFunction(
        "(define-fun inv ((a (_ BitVec 4)) (b (_ BitVec 4))) Bool "
        "(and (bvult a b) (bvule a b)))",
        false);
    auto* function = dynamic_cast<SygusFunction*>(expression);
    check(function != nullptr, "unsigned BitVec comparisons parse");
    if (function != nullptr)
        check(function->getBodyVerilogExpr() ==
                  "(($unsigned(a) < $unsigned(b)) && "
                  "($unsigned(a) <= $unsigned(b)))",
              "bvult and bvule keep unsigned SystemVerilog semantics");
    delete expression;
}

void testLtlLatencyIsResetBetweenFunctions() {
    SmtFunctionParser parser;
    SygusExpr* first = parser.parseSmtFunction(
        "(define-fun inv ((a Bool) (a_3 Bool)) Bool (=> a a_3))", true);
    auto* firstFunction = dynamic_cast<SygusFunction*>(first);
    check(firstFunction != nullptr &&
              firstFunction->getBodyVerilogExpr() == "(a |-> ##3  a)",
          "numeric LTL suffix becomes the property latency");
    delete first;

    SygusExpr* second = parser.parseSmtFunction(
        "(define-fun inv ((a Bool) (a_3 Bool)) Bool (=> a a))", true);
    auto* secondFunction = dynamic_cast<SygusFunction*>(second);
    check(secondFunction != nullptr &&
              secondFunction->getBodyVerilogExpr() == "(a |-> a)",
          "an unused next-state parameter neither sets nor inherits latency");
    delete second;
}

}  // namespace

int main() {
    testValidFunction();
    testInvalidFunction();
    testUnsignedComparisonsStayUnsignedInSystemVerilog();
    testLtlLatencyIsResetBetweenFunctions();
    if (failures != 0) {
        std::cout << failures << " SMT parser test(s) failed\n";
        return 1;
    }
    std::cout << "all SMT parser tests passed\n";
    return 0;
}
