#ifndef SYGUSGENERATER_H
#define SYGUSGENERATER_H
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "utils.h"
#include "Value.h"
#include "State.h"
class SyGuSGenerater
{
    public:
        SyGuSGenerater();
        ~SyGuSGenerater();
        
        void setSignals(std::vector<Signal>*);
        void addConstraints(std::vector<std::vector<Value*>>,bool);
        void addConstraints(State*,bool);
        void addConstraintComments(std::string,bool);

        void printSysgusPath(std::string);
        void printLTLSygusPath(std::string,int);
        void debugPrint();
        std::string runCVC5Sygus(std::string);

    private:
        //this corelates the signal and its values
        std::vector<Signal> signals;
        std::vector<std::vector<Value*>> constraints; 
        std::vector<std::vector<Value*>> falseConstraints;

        std::map<int,std::string> comments;
        std::map<int,std::string> falseComments;

        //This map contains the same signal type signals
        std::map<std::pair<SignalType,int>,std::vector<Signal>> sameTypeSignals;
        std::map<Signal,std::vector<Value*>> signalValuesMap;
        std::map<std::string, std::string> bitSignalGrammerMap; 

        std::string createSynthesisFunction(const std::vector<Signal>,int);
        std::string createFunctionHeader(const std::vector<Signal>,int);
        std::string createFunctionGrammar(int);
        std::string createAtomGrammar();
        std::string createAtomXGrammar(int);
        std::string createExprGrammar();
        std::string createExprXGrammar();
        std::string createSingleBvGrammar(Signal);
        std::string createMixBvGrammar(const std::vector<Signal>,int);
        std::string createKeyGrammar(int);
        std::string createConstraint(bool,int);
        bool checkConstraintsDefined(int,bool);

        

};
#endif // SyGuSGenerater_H