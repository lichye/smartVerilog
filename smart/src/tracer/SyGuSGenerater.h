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
        
        void setSignals(std::vector<Signal>);
        void setSignals(std::vector<Signal>*);
        //void setSignals(std::string);

        void addConstraints(std::vector<std::vector<Value*>>,bool);
        void addConstraints(State*,bool);
        void addConstrainComments(std::string,bool);

        void printSysgusPath(std::string);
        void debugPrint();

    private:
        
        //this corelates the signal and its values
        std::vector<Signal> signals;


        std::map<int,std::string> comments;
        std::map<int,std::string> falseComments;

        std::vector<std::vector<Value*>> constraints;
        std::vector<std::vector<Value*>> falseConstraints;
        
        
        //This map contains the same signal type signals
        std::map<std::pair<SignalType,int>,std::vector<Signal>> sameTypeSignals;
        std::map<Signal,std::vector<Value*>> signalValuesMap;
        std::map<std::string, std::string> bitSignalGrammerMap; 

        std::string createSyntheisFunction(const std::vector<Signal>);
        std::string createFunctionHeader(const std::vector<Signal>);
        std::string createFunctionGrammar();
        std::string createBoolGrammar();
        std::string createSingleBvGrammar(Signal);
        std::string createMixBvGrammar(const std::vector<Signal>);
        std::string createKeyGrammar();
        std::string createConstraint(bool,int);

        bool checkConstraintsDefined(int,bool);

        

};
#endif // SyGuSGenerater_H