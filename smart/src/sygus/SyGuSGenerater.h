#ifndef SYGUSGENERATER_H
#define SYGUSGENERATER_H
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "Utils.h"
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

        // Solve the SyGuS problem in `path` and return cvc5's answer verbatim
        // (the `(define-fun inv ...)` block the caller parses).
        std::string runCVC5Sygus(std::string);

        // Per-call time limit, in milliseconds. Used to be a hardcoded
        // `timeout 5` on the command line.
        void setSygusTimeoutMs(int);
        // Shell out to the cvc5 BINARY instead of solving in-process. Kept as
        // an A/B switch: the two paths pose the same problem but do not always
        // pick the same solution out of the many that satisfy it.
        void setUseSubprocess(bool);
        // Keep the generated .sl file after solving (--keep-work).
        void setKeepTempFiles(bool);

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

        int sygusTimeoutMs = 5000;
        bool useSubprocess = false;
        bool keepTempFiles = false;

        std::string runCVC5SygusInProcess(const std::string&);
        std::string runCVC5SygusSubprocess(const std::string&);
        void removeTempFile(const std::string&);

        std::string createSynthesisFunction(const std::vector<Signal>,int);
        std::string createFunctionHeader(const std::vector<Signal>,int);
        std::string createFunctionGrammar(int);
        std::string createAtomGrammar(bool);
        std::string createAtomXGrammar(int);
        std::string createExprGrammar();
        std::string createExprXGrammar();
        std::string createSingleBvGrammar(Signal);
        std::string createMixBvGrammar(const std::vector<Signal>,int);
        std::string createKeyGrammar(int);
        std::string createConstraint(bool,int);
        std::string createLTLConstraint(bool,int,int);
        bool checkConstraintsDefined(int,bool);

        

};
#endif // SyGuSGenerater_H