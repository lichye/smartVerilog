#ifndef MODULE_H
#define MODULE_H
#include "string"
#include <vector>
#include "Trace.h"
#include <filesystem>
#include <set>
#include <map>
struct Constrains{
    std::vector<std::vector<Value*>> constraints;
    std::string tracePath;
    bool isTrue;
};

class Module{
    public:
    Module(std::string);
    void addTrace(TraceType,std::string);
    void addTracesfromDir(TraceType,std::string);
    std::vector<Signal>* getAllSignals();
    std::vector<Constrains> getAllConstraints(std::vector<Signal>*);
    Constrains getConstrain(std::string,std::vector<Signal>*);
    private:
    std::string moduleName;

    std::vector<Trace*> traces;
    std::vector<std::string> tracePaths;

    
};
#endif