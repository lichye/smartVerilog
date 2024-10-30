#include "SyGuSGenerater.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

SyGuSGenerater::SyGuSGenerater()
{

}

SyGuSGenerater::~SyGuSGenerater()
{
    std::cout<<"SyGuSGenerater Destructor called"<<std::endl;
}

void SyGuSGenerater::setSignals(std::vector<Signal> signals)
{
    for(auto signal : signals){
        SignalType signalType = signal.type;
        int signalWidth = signal.lindex - signal.rindex + 1;
        std::pair<SignalType,int> signalKey = std::make_pair(signalType,signalWidth);
        
        //add the Signal to the sameTypeSignals map
        //then we have the relation ship between the signal type and the signal
        if(sameTypeSignals.find(signalKey) == sameTypeSignals.end()){
            sameTypeSignals[signalKey] = std::vector<Signal>();
            sameTypeSignals[signalKey].push_back(signal);
        }
        else{
            sameTypeSignals[signalKey].push_back(signal);
        }
        this->signals.push_back(signal);
    }

}

void SyGuSGenerater::setSignals(std::vector<Signal>* signals)
{
    for(auto signal : *signals){
        SignalType signalType = signal.type;
        int signalWidth = signal.lindex - signal.rindex + 1;
        std::pair<SignalType,int> signalKey = std::make_pair(signalType,signalWidth);
        
        //add the Signal to the sameTypeSignals map
        //then we have the relation ship between the signal type and the signal
        if(sameTypeSignals.find(signalKey) == sameTypeSignals.end()){
            sameTypeSignals[signalKey] = std::vector<Signal>();
            sameTypeSignals[signalKey].push_back(signal);
        }
        else{
            sameTypeSignals[signalKey].push_back(signal);
        }
        this->signals.push_back(signal);
    }
}

void SyGuSGenerater::addConstraints(std::vector<std::vector<Value*>> inputConstraints)
{   
    assert(inputConstraints.size() == this->signals.size());
    //if the constraints are empty then we add the inputConstraints one by one
    if(constraints.size() == 0){
        for(auto &constraint : inputConstraints){
            this->constraints.push_back(constraint);
        }
    }
    else{
        if(inputConstraints.size() == this->constraints.size()){
            for(int i = 0;i<constraints.size();i++){
                this->constraints[i].insert(this->constraints[i].end(),inputConstraints[i].begin(),inputConstraints[i].end());
            }
        }
        else{
            std::cout<<"The constraints size is not the same"<<std::endl;
        }
    }
    
}

void SyGuSGenerater::addFalseConstraints(std::vector<std::vector<Value*>> inputConstraints)
{
    assert(inputConstraints.size() == this->signals.size());

    if(falseConstraints.size() == 0){
        for(auto &constraint : inputConstraints){
            this->falseConstraints.push_back(constraint);
        }
    }
    else{
        if(inputConstraints.size() == this->falseConstraints.size()){
            for(int i = 0;i<falseConstraints.size();i++){
                this->falseConstraints[i].insert(this->falseConstraints[i].end(),inputConstraints[i].begin(),inputConstraints[i].end());
            }
        }
        else{
            std::cout<<"The false constraints size is not the same length with the signals"<<std::endl;
        }
    }
}

void SyGuSGenerater::printSysgusPath(std::string path)
{
    std::ofstream file;
    file.open(path);
    if(file.is_open()){
        file<<"(set-logic BV)\n";
        
        //print out the synthesis function, only if signals exist
        if(signals.size()>0){
            file<<makeSyntheisFunction(signals);
        }
        
        //print out  the constraints, only if constraints exist
        if(constraints.size() >0){
            for(int i =0;i<constraints[0].size();i++){
                file<<createConstraint(true,i);
            }
        }

        //print out the false constraints, only if false constraints exist
        if(falseConstraints.size() > 0){
            for(int i =0;i<falseConstraints[0].size();i++){
                file<<createConstraint(false,i);
            }
        }
       
        file<<"(check-synth)"<<std::endl;
        file.close();
    }
    else{
        std::cout<<"Error opening file"<<std::endl;
    }
}

std::string SyGuSGenerater::makeSyntheisFunction(const std::vector<Signal> signals)
{   
    std::string synthFun="";
    std::string functionheader = createFunctionHeader(signals);
    std::string functionGrammar = createFunctionGrammar();
    synthFun = "( "+ functionheader + functionGrammar+")\n";
    return synthFun;
}

std::string SyGuSGenerater::createFunctionHeader(const std::vector<Signal> signals)
{
    std::string functionHeader = "synth-fun ";
    functionHeader += "inv ";
    functionHeader += "(";
    
    //add the signals to the function header
    for(auto signal : signals){
        if(signal.type == SignalType::BITS){
            functionHeader += "("+ signal.toSygusName() + " ";
            functionHeader += "(_ BitVec " + std::to_string(signal.lindex - signal.rindex + 1) + ")";
            functionHeader += ") ";
        }
        if(signal.type == SignalType::BOOLEAN){
            functionHeader += "("+signal.toSygusName() + " ";
            functionHeader += "Bool";
            functionHeader += ") ";
        }
    }

    //end of the function header
    functionHeader += ") Bool\n";
    return functionHeader;
}

std::string SyGuSGenerater::createFunctionGrammar()
{
    std::string functionGrammar = "(\n";
    
    functionGrammar += "(";
    functionGrammar += "Start ";
    functionGrammar += " Bool";
    functionGrammar += ")\n";

    std::string functionGrammarDetail = "";

    // for(auto signal:signals){
    //     if(signal.type == SignalType::BITS){
    //         functionGrammar += "(";
    //         functionGrammar += "Single_"+signal.toSygusName()+" ";
    //         functionGrammar += " (_ ";
    //         functionGrammar += "BitVec ";
    //         functionGrammar += std::to_string(signal.lindex - signal.rindex + 1);
    //         functionGrammar += "))\n";

    //         functionGrammarDetail += createSingleBvGrammar(signal);
    //     }
    // }

    functionGrammarDetail += createBoolGrammar();

    for(auto signals : sameTypeSignals){
        if(signals.first.first == SignalType::BITS){
            //assert(signals.second.size() > 1);
            functionGrammarDetail += createMixBvGrammar(signals.second);
            functionGrammar += "(";
            functionGrammar += "MixBv"+std::to_string(signals.first.second)+" ";
            functionGrammar += " (_ ";
            functionGrammar += "BitVec ";
            functionGrammar += std::to_string(signals.first.second);
            functionGrammar += "))\n";
        }   
    }
    //add the details for each grammar in the functionGrammar
    functionGrammar += ")\n";
    functionGrammar += "(\n";
    functionGrammar += functionGrammarDetail;
    functionGrammar += ")\n";

    return functionGrammar;
}

std::string SyGuSGenerater::createSingleBvGrammar(Signal signal)
{   
    std::string startName = "Single_"+signal.toSygusName();

    std::string bvGrammar=std::string("("+startName+" (_ BitVec " + std::to_string(signal.getWidth()) + ") \n") +std::string("    ( \n");
    bvGrammar += std::string("\t")+signal.toSygusName()+"\n";
    bvGrammar += std::string("\t(bvneg ")+ startName + ")\n";
    bvGrammar += std::string("\t(bvnot ")+ startName + ")\n";
    bvGrammar += std::string("\t(bvand ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t(bvor ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t(bvxor ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t(bvadd ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t(bvsub ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t(bvmul ")+ startName + " " + startName + ")\n";
    // bvGrammar += std::string("\t(bvudiv ")+ startName + " " + startName + ")\n";
    // bvGrammar += std::string("\t(bvlshr ")+ startName + " " + startName + ")\n";
    // bvGrammar += std::string("\t(bvshl ")+ startName + " " + startName + ")\n";
    bvGrammar += std::string("\t)\n")+
    std::string(")\n");
    return bvGrammar;
}

std::string SyGuSGenerater::createMixBvGrammar(std::vector<Signal> signals){
    std::string mixBvGrammar = "";
    std::string startName = "MixBv"+std::to_string(signals[0].getWidth());
    mixBvGrammar += std::string("("+startName+" (_ BitVec " + std::to_string(signals[0].getWidth()) + ") \n") +std::string("    ( \n");
    for(auto signal : signals){
        mixBvGrammar += std::string("\t")+signal.toSygusName()+"\n";
    }
    mixBvGrammar += std::string("\t(bvneg ")+ startName + ")\n";
    mixBvGrammar += std::string("\t(bvnot ")+ startName + ")\n";
    mixBvGrammar += std::string("\t(bvand ")+ startName + " " + startName + ")\n";
    mixBvGrammar += std::string("\t(bvor ")+ startName + " " + startName + ")\n";
    //mixBvGrammar += std::string("\t(bvxor ")+ startName + " " + startName + ")\n";
    mixBvGrammar += std::string("\t(bvadd ")+ startName + " " + startName + ")\n";
    mixBvGrammar += std::string("\t(bvsub ")+ startName + " " + startName + ")\n";
    mixBvGrammar += std::string("\t(bvmul ")+ startName + " " + startName + ")\n";
    // mixBvGrammar += std::string("\t(bvudiv ")+ startName + " " + startName + ")\n";
    // mixBvGrammar += std::string("\t(bvlshr ")+ startName + " " + startName + ")\n";
    // mixBvGrammar += std::string("\t(bvshl ")+ startName + " " + startName + ")\n";
    mixBvGrammar += std::string("\t)\n")+
    std::string(")\n");
    return mixBvGrammar;
}

std::string SyGuSGenerater::createBoolGrammar()
{   
    std::string boolGra =
    std::string("(Start Bool \n")+
    std::string("\t(\n")+
    //std::string("; true false\n")+
    std::string("\t(not Start) \n")+
    std::string("\t(and Start Start) \n")+
    std::string("\t(or Start Start)\n");
    
    //wait for add bv compare grammar
    boolGra += std::string("\t)\n")+ std::string(")\n");
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

void SyGuSGenerater::debugPrint()
{
    std::cout<<"The signals are: "<<std::endl;
    for(auto &signal : signals){
        std::cout<<signal.name<<std::endl;
    }
    std::cout<<"The constraints are: "<<std::endl;
    for(auto &constraint : constraints){
        for(auto &value : constraint){
            std::cout<<value->toString()<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"The false constraints are: "<<std::endl;
    for(auto &constraint : falseConstraints){
        for(auto &value : constraint){
            std::cout<<value->toString()<<std::endl;
        }
    }
}