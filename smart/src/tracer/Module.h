#ifndef MODULE_H
#define MODULE_H
#include "string"
#include <vector>
#include "Trace.h"
#include "setups.h"
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
    // hierarchical: take the whole subtree below the mined scope as candidate
    // material, naming each signal by its instance path. Off keeps the mined
    // scope's own signals only, which is what --module relies on.
    Module(std::string, bool hierarchical = false);
    void addTrace(TraceType,std::string);
    void addTracesfromDir(TraceType,std::string);
    std::vector<Signal>* getAllSignals();
    std::vector<Constrains> getAllConstraints(std::vector<Signal>*);
    Constrains getConstrain(std::string,std::vector<Signal>*);
    private:
    std::string moduleName;

    bool hierarchical;

    std::vector<Trace*> traces;
    std::vector<std::string> tracePaths;

    
};
#endif