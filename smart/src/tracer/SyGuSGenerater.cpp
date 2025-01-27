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
    printDebug("SyGuSGenerater Destructor called",2);
    //std::cout<<"SyGuSGenerater Destructor called"<<std::endl;
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

void SyGuSGenerater::addConstraints(std::vector<std::vector<Value*>> inputConstraints,bool trueConstrains){
    printDebug("Run addConstraints",2);
    if(inputConstraints.size()!=this->signals.size()){
        std::cout<<"The constraints size is not the same length with the signals"<<std::endl;
        std::cout<<"The constraints size is "<<inputConstraints.size()<<" and the signals size is "<<this->signals.size()<<std::endl;
    }
    assert(inputConstraints.size() == this->signals.size());
    //if the constraints are empty then we add the inputConstraints one by one
    if(trueConstrains){
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
    else{
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
    printDebug("Finish SyGuSGenerater::addConstraints",2);
}

void SyGuSGenerater::addConstrainComments(std::string comment,bool isTrueConstrains){
    if(isTrueConstrains){
        if(constraints.size() == 0){
            comments[0] = comment;
        }
        else{
            comments[constraints[0].size()] = comment;
        }
    }
    else{
        if(falseConstraints.size() == 0){
            falseComments[0] = comment;
        }
        else{
            falseComments[falseConstraints[0].size()] = comment;
        }
    }
}

void SyGuSGenerater::addConstraints(State* state,bool trueConstrains)
{   
    printDebug("Adding constraints from the state",2);
    printDebug("The state is "+state->toString(),2);
    assert(state->signalSize() == this->signals.size());
    std::vector<Value*> values = state->getValues();
    assert(values.size() == this->signals.size()); 
    //we suppose the signal order is correct
    // addConstrainComments("This constrainsts are added from the state",trueConstrains);
    if(trueConstrains){
       if(constraints.size()==0){
            for(auto value : values){
                std::vector<Value*> constraint;
                constraint.push_back(value);
                constraints.push_back(constraint);
            }
       }
       else{
            int size = values.size();
            for(int i=0;i<size;i++){
                constraints[i].push_back(values[i]);
            }
       }
    }
    else{
        printDebug("Adding false constraints\n",3);
        if(falseConstraints.size()==0){
            printDebug("FalseConstraints does not exists\n",3);
            for(auto value : values){
                std::vector<Value*> constraint;
                constraint.push_back(value);
                falseConstraints.push_back(constraint);
            }
        }
        else{
            int size = values.size();
            for(int i=0;i<size;i++){
                falseConstraints[i].push_back(values[i]);
            }
        }
    }
}

void SyGuSGenerater::printSysgusPath(std::string path)
{
    printDebug("Printing the SyGuS file to the path: "+path,2);
    std::ofstream file;
    file.open(path);
    if(file.is_open()){
        file<<"(set-logic BV)\n";
        
        //print out the synthesis function, only if signals exist
        if(signals.size()>0){
            file<<createSyntheisFunction(signals);
        }
        
        //print out  the constraints, only if constraints exist
        if(constraints.size() >0){
            //we believe that the constraints are the same length with the signals
            for(int i =0;i<constraints[0].size();i++){
                if(comments.find(i) != comments.end()){
                    file<<"; "<<comments[i]<<std::endl;
                }
                file<<createConstraint(true,i);
            }
        }

        //print out the false constraints, only if false constraints exist
        file<<"; "<<"False Constraints below"<<std::endl;
        if(falseConstraints.size() > 0){
            for(int i =0;i<falseConstraints[0].size();i++){
                if(falseComments.find(i) != falseComments.end()){
                    file<<"; "<<falseComments[i]<<std::endl;
                }
                file<<createConstraint(false,i);
            }
        }
       
        file<<"(check-synth)"<<std::endl;
        file.close();
    }
    else{
        std::cout<<"Error opening file"<<std::endl;
    }
    printDebug("Printing the SyGuS file to the path: "+path,2);
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
            std::cout<<value->toSyGusString()<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"The false constraints are: "<<std::endl;
    for(auto &constraint : falseConstraints){
        for(auto &value : constraint){
            std::cout<<value->toSyGusString()<<std::endl;
        }
    }
}

std::string SyGuSGenerater::runCVC5Sygus(std::string sygusPath){
  std::string command = "cvc5 --lang=sygus2 "+sygusPath;
  std::string result;
  char buffer[128];
  FILE* pipe = popen(command.c_str(), "r");
  if(!pipe){
    print("Error: popen failed\n");
    return "";
  }
  try{
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
    }
  }
  catch(...){
    pclose(pipe);
    throw;
  }
  int exitCode = pclose(pipe);
  if(exitCode!=0){
    print("Error: cvc5 failed\n");
    return "";
  }
  return result;
}

std::string SyGuSGenerater::createSyntheisFunction(const std::vector<Signal> signals)
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

    functionGrammarDetail += createBoolGrammar();

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
    std::string("\t(= Start Start) \n")+
    std::string("\t(not Start) \n")+
    std::string("\t(and Start Start) \n")+
    std::string("\t(or Start Start)\n");
    
    //add the important checker for True return
    boolGra += createKeyGrammar();

    //wait for add bv compare grammar
    boolGra += std::string("\t)\n")+ std::string(")\n");
    return boolGra;
}

std::string SyGuSGenerater::createConstraint(bool constraintType,int index)
{
    assert(index < constraints[0].size());
    std::string constraintLine;
    if(!checkConstraintsDefined(index,constraintType)){
        constraintLine+="; ";
    }
    
    constraintLine += "(constraint (=(inv ";

    if(constraintType){
        for(auto constraint : constraints){
            constraintLine += constraint[index]->toSyGusString() + " ";
        }
    }
    else{
        for(auto constraint : falseConstraints){
            constraintLine += constraint[index]->toSyGusString() + " ";
        }
    }
    
    constraintLine += ") ";
    constraintLine += constraintType ? "true" : "false";
    constraintLine += "))\n";
    return constraintLine;
}

std::string SyGuSGenerater::createKeyGrammar()
{   
    std::string trueGrammar = "";
    //add true checker for each signal
    for(auto typeSignals : sameTypeSignals){
        
        SignalType signalType = typeSignals.first.first;
        int signalWidth = typeSignals.first.second;


        if(signalType == SignalType::BOOLEAN){
            for(auto signal : typeSignals.second){
                trueGrammar += "\t" + signal.toSygusName() + "\n";
            }
        }
        else if(signalType == SignalType::BITS){
            // trueGrammar += "\t(not (= MixBv"+std::to_string(signalWidth)+" (_ bv0 "+std::to_string(signalWidth)+")))\n";
            trueGrammar += "\t(= MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvule  MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvuge MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvult MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvugt MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvslt MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvsgt MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvsle MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
            trueGrammar += "\t(bvsge MixBv"+std::to_string(signalWidth)+" MixBv"+std::to_string(signalWidth)+" )\n";
        }    
    }

    return trueGrammar;
}

bool SyGuSGenerater::checkConstraintsDefined(int index,bool trueConstrains)
{   
    if(trueConstrains){
        for(auto &constraint : constraints){
            if(constraint[index]->isUndefined()){
                return false;
            }
        }
        return true;
    }
    else{
        for(auto &constraint : falseConstraints){
            if(constraint[index]->isUndefined()){
                return false;
            }
        }
        return true;
    }

}

