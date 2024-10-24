#include "SyGuSGenerater.h"
#include <iostream>
#include <fstream>
#include <vector>

SyGuSGenerater::SyGuSGenerater()
{

}

SyGuSGenerater::~SyGuSGenerater()
{
    std::cout<<"SyGuSGenerater Destructor called"<<std::endl;
}

void SyGuSGenerater::addSignalsAndConstraints(std::vector<Signal*> signals,std::vector<std::vector<Value*>> constraints)
{
    for(auto &signal : signals){
        this->signals.push_back(signal);
    }

    for(auto &constraint : constraints){
        this->constraints.push_back(constraint);
    }
}

void SyGuSGenerater::printSysgusPath(std::string path)
{
    std::ofstream file;
    file.open(path);
    if(file.is_open()){
        file<<"(set-logic BV)\n";
        file<<makeSyntheisFunction(signals);
        //print out  the constraints
        for(int i =0;i<constraints[0].size();i++){
            file<<createConstraint(true,i);
        }
        file<<"(check-synth)"<<std::endl;
        file.close();
    }
    else{
        std::cout<<"Error opening file"<<std::endl;
    }
}

//TODO: add support for other signal types
std::string SyGuSGenerater::makeSyntheisFunction(const std::vector<Signal*> signals)
{   
    std::string synthFun="";

    //create the function header
    std::string functionheader = createFunctionHeader(signals);

    //start the grammar parts
    std::string functionGrammar = "((Start Bool) (StartBv (_ BitVec 8)))\n";
    functionGrammar += "(" + createBoolGrammar() + createBvGrammar(signals, 8) + ")\n";

    synthFun = "( "+ functionheader + functionGrammar+")\n";
    return synthFun;
}

std::string SyGuSGenerater::createFunctionHeader(const std::vector<Signal*> signals)
{
    std::string functionHeader = "synth-fun ";

    //set the function name
    functionHeader += "inv ";
    functionHeader += "(";
    
    //add the signals to the function header
    for(auto &signal : signals){
        if(signal->type == SignalType::BITS){
            functionHeader += "("+signal->name + " ";
            functionHeader += "(_ BitVec " + std::to_string(signal->lindex - signal->rindex + 1) + ")";
            functionHeader += ") ";
        }
        if(signal->type == SignalType::BOOLEAN){
            functionHeader += "("+signal->name + " ";
            functionHeader += "Bool";
            functionHeader += ") ";
        }
    }

    //end of the function header
    functionHeader += ") Bool\n";
    return functionHeader;
}

std::string SyGuSGenerater::createBvGrammar(const std::vector<Signal*> signals, int bitWidth)
{   

    std::string signalsName="";
    for(auto &signal : signals){
        if(signal->type == SignalType::BITS)
            signalsName += signal->name + " ";
    }
    signalsName +="\n";

    std::string bvGrammar=std::string("(StartBv (_ BitVec " + std::to_string(bitWidth) + ") \n") +std::string("    ( \n");
    bvGrammar += signalsName;
    bvGrammar +=
    std::string("(bvneg StartBv)\n ")+
    std::string("(bvnot StartBv)\n")+
    std::string("(bvand StartBv StartBv) \n")+
    std::string("(bvor StartBv StartBv) \n")+
    std::string("(bvxor StartBv StartBv) \n")+
    std::string("(bvadd StartBv StartBv) \n")+
    std::string("(bvsub StartBv StartBv) \n")+
    std::string("(bvmul StartBv StartBv) \n")+
    std::string("(bvudiv StartBv StartBv) \n")+
    std::string("(bvlshr StartBv StartBv) \n")+
    std::string("(bvshl StartBv StartBv) \n")+
    std::string("   )\n")+
    std::string(")\n");
    return bvGrammar;
}

std::string SyGuSGenerater::createBoolGrammar()
{   
    std::string boolGra =
    std::string("(Start Bool \n")+
    std::string("(")+
    std::string("; true false\n")+
    std::string("(not Start) \n")+
    std::string("(and Start Start) \n")+
    std::string("(or Start Start)\n")+
    std::string("(bvult StartBv StartBv)\n")+
    std::string("(bvslt StartBv StartBv)\n")+
    std::string("(bvuge StartBv StartBv)\n")+
    std::string("(bvsge StartBv StartBv)\n")+
    //std::string("(not (= Start (_ bv0 8)))\n")+
    std::string(")\n")+
    std::string(")");
    return boolGra;
}

std::string SyGuSGenerater::createConstraint(bool constraintValue,int index)
{
    std::string constraintLine = "(constraint (=(inv ";
    for(auto constraint : constraints){
        constraintLine += constraint[index]->toString() + " ";
    }
    constraintLine += ") ";
    constraintLine += constraintValue ? "true" : "false";
    constraintLine += "))\n";
    return constraintLine;
}