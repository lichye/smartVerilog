#ifndef SYGUSGENERATER_H
#define SYGUSGENERATER_H
#include <string>
#include <vector>
#include "utils.h"
#include "Value.h"
class SyGuSGenerater
{
    public:
        SyGuSGenerater();
        // SyGuSGenerater(std::string);
        // SyGuSGenerater(std::vector<VCDSignal>);

        ~SyGuSGenerater();
        
        //void parseFile(const char* filename);
        void setSignals(const std::vector<Signal*>);
        void setConstraints(std::vector<std::vector<Value*>>);
        void printSysgusPath(std::string);

        bool isSetSignals;
        bool isSetConstraints;
        bool isPrintSysgus;
    private:
        std::vector<Signal*> signals;
        std::vector<std::vector<Value*>> constraints;

        std::string translateSignal2synthFun(const std::vector<Signal*>);
        std::string createFunctionHeader(const std::vector<Signal*>);
        std::string createBvGrammar(const std::vector<Signal*>,int);
        std::string createBoolGrammar();
        std::string createConstraint(bool,int);

};
#endif // SyGuSGenerater_H