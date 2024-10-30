#ifndef SYGUSGENERATER_H
#define SYGUSGENERATER_H
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "utils.h"
#include "Value.h"
class SyGuSGenerater
{
    public:
        SyGuSGenerater();
        // SyGuSGenerater(std::string);
        // SyGuSGenerater(std::vector<VCDSignal>);

        ~SyGuSGenerater();
        
        void setSignals(std::vector<Signal>);
        void setSignals(std::vector<Signal>*);
        //void setSignals(std::string);
        void addConstraints(std::vector<std::vector<Value*>>);
        void addFalseConstraints(std::vector<std::vector<Value*>>);
        //the signals and value should be in the same order
        //void addSignalsAndConstraints(std::vector<Signal*>,std::vector<std::vector<Value*>>);
        void printSysgusPath(std::string);
        void debugPrint();

    private:
        
        //this corelates the signal and its values
        std::vector<Signal> signals;
        std::vector<std::vector<Value*>> constraints;
        std::vector<std::vector<Value*>> falseConstraints;
        
        
        //This map contains the same signal type signals
        std::map<std::pair<SignalType,int>,std::vector<Signal>> sameTypeSignals;

        std::map<Signal,std::vector<Value*>> signalValuesMap;

        std::string makeSyntheisFunction(const std::vector<Signal>);
        std::string createFunctionHeader(const std::vector<Signal>);
        std::string createFunctionGrammar();
        std::string createBvGrammar(const std::vector<Signal>,int);
        std::string createBoolGrammar();
        std::string createConstraint(bool,int);

};
#endif // SyGuSGenerater_H