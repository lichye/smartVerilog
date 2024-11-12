#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include "Trace.h"
#include "SyGuSGenerater.h"
#include "State.h"
#include "StateMaker.h"
#include "VerilogMaker.h"
namespace fs = std::filesystem;

//globak variables
std::vector<Trace*> traces;
std::vector<Signal>* signals;

SyGuSGenerater sygus;

std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";


State* getUnreachableState(std::string,std::string);
void readSimVcdFiles(std::string);
void readSmtVcdFiles(std::string);
void addConstraintsfromTrace();

bool runEbmc(std::string);

int main(int argc, char* argv[]){
  bool useTraces = false;
  bool testGetUnreachable = false;

  SignalGather sg(config_path);
  signals = sg.getAllSignals();
  sygus.setSignals(signals);
  

  if(argc==1){
    print("No arguments provided, run default paths\n");
    useTraces = true; 
  }
  //when there is two arguments
  else if(argc>=2){
    if(std::string(argv[1])=="--trace"||std::string(argv[1])=="-t")
      useTraces = true;
    //we hope the usage of smart --unreachable [config_path] [verilogSrcPath] [resultPath]
    if(std::string(argv[1])=="--unreachable"||std::string(argv[1])=="-u"){
      testGetUnreachable = true;
      print("Try to find a unreachable state\n");
    }
  }

  readSimVcdFiles(sim_path);
  readSmtVcdFiles(smt_path);

  if(useTraces){
    addConstraintsfromTrace();
  }
  
  if(testGetUnreachable){
    std::string verilogSrcPath = "runtime/verilog/addsub.sv";
    std::string resultPath  = "runtime/ebmc/addsub.sv";

    State* state = getUnreachableState(verilogSrcPath,resultPath);
    if(state!=nullptr){
      print("Find a unreachable state\n");
      print(state->toString());
    }
    else{
      print("Find a reachable state\n");
      print(state->toString());
    }
    return 0;
  }

      
  sygus.printSysgusPath("sygus.sl");
  for(auto &trace : traces){
    delete trace;
  }

  return 0;  
}

//input: verilog code to be verified
//output: true : the state is unreachable
//        false: the state is reachable
bool runEbmc(std::string verilogSrcPath){
  std::string command = "ebmc "+verilogSrcPath+" --bound 10 > /dev/null 2>&1";
  printDebug("Running EBMC with command: "+command+"\n",1);
  int status = system(command.c_str());
  return status==0;
}

State* getUnreachableState(std::string verilogSrcPath,std::string resultPath){
  StateMaker sm(&traces,signals);
  VerilogMaker vm;

  
  State* state = sm.makeRandomState();
  
  vm.addProperty(state,PropertyType::REACHABILITY_PROPERTY);
  vm.writeVerilogFile(verilogSrcPath,resultPath);
  
  
  printDebug("Try below states",3);
  printDebug(state->toString(),3);

  //then we should the ebmc to verify the state
  if(runEbmc(resultPath)){
    printDebug("The state is unreachable",3);
    return state;
  }
  else{
    printDebug("The random state is reachable",3);
    return nullptr;
  }
}

void readSimVcdFiles(std::string sim_path){
  int vcdFileCount = 0;
  std::vector<std::string> simVcdFiles;
  try {
        for (const auto& entry : fs::directory_iterator(sim_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vcd") {
                ++vcdFileCount;
                simVcdFiles.push_back(entry.path().string());
            }
        }
        
        std::cout << "Number of Simulation VCD files: " << vcdFileCount << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  if(vcdFileCount > 0){
    print("List of Simulation VCD files: \n");
  }

  for(auto &vcdFile : simVcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(TraceType::SIM, vcdFile);
    traces.push_back(trace);
  }
}

void readSmtVcdFiles(std::string smt_path){
  int vcdFileCount = 0;
  std::vector<std::string> smtVcdFiles;
  try {
        for (const auto& entry : fs::directory_iterator(smt_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vcd") {
                ++vcdFileCount;
                smtVcdFiles.push_back(entry.path().string());
            }
        }
        print("SMT's Number of VCD files: " + std::to_string(vcdFileCount)+"\n");
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  if(vcdFileCount > 0){
    print("Number of VCD files: " + std::to_string(vcdFileCount)+"\n");
  }

  for(auto &vcdFile : smtVcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(TraceType::SMT, vcdFile);
    traces.push_back(trace);
    std::cout<<"SMT trace object inserted into the traces vector"<<std::endl;
  }
}

void addConstraintsfromTrace(){
  for(auto &trace : traces){
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    sygus.addConstrainComments("Getting constraints from the trace :\t"+trace->getPath(),true);
    sygus.addConstraints(*constraints,true);
  }

}


