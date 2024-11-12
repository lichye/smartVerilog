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


std::vector<Trace*> traces;

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
void makeSyGusFile(std::string configPath, std::string resultPath){
  
  SignalGather sg(configPath);

  SyGuSGenerater sygus;

  std::vector<Signal>* signals = sg.getAllSignals();
  
  sygus.setSignals(signals);

  for(auto &trace : traces){
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    sygus.addConstrainComments("Getting constraints from the trace :\t"+trace->getPath(),true);
    sygus.addConstraints(*constraints,true);
  }

  sygus.printSysgusPath(resultPath);
  print("SyGuS file is stored in "+resultPath+"\n");
}
void getUnreachableState(std::string configPath,std::string verilogPath,std::string resultPath){
  print("Getting the unreachable state of the module\n");
  SignalGather sg(configPath);
  std::vector<Signal>* signals = sg.getAllSignals();
  StateMaker sm(&traces,signals);
  State* state = sm.makeRandomState();
  VerilogMaker vm;
  vm.addProperty(state,PropertyType::REACHABILITY_PROPERTY);
  vm.writeVerilogFile(verilogPath,resultPath);
  print("Try below states\n");
  print(state->toString());

  //then we should the ebmc to verify the state
  
}

int main(int argc, char* argv[]){
  bool runSygus = false;
  bool runGetUnreachableState = false;

  std::string sim_path = "runtime/sim_results";
  std::string smt_path = "runtime/smt_results";
  std::string config_path = "User/config.ini";
  std::string verilogPath = "/home/magna/smartVerilog/smart/runtime/verilog/addsub.sv";
  std::string resultPath = "/home/magna/smartVerilog/smart/runtime/ebmc/addsub.sv";

  if(argc==1){
    std::cout<<"No arguments provided, using default paths\n";
    runSygus = true; 
  }
  //when there is two arguments
  else if(argc>=2){
    //requires of help
    if(std::string(argv[1])=="-h"){
      print("Usage: ./smart sygus/state [sim_path] [smt_path] [config_path] [result_path]");
      print("Default paths are used if no arguments are provided");
      return 0;
    }
    if(std::string(argv[1])=="--sygus"||std::string(argv[1])=="-s"){
      runSygus = true;
    }
    //we hope the usage of smart --unreachable [config_path] [verilogPath] [resultPath]
    if(std::string(argv[1])=="--unreachable"||std::string(argv[1])=="-u"){
      runSygus = false;
      runGetUnreachableState = true;
      if(argc>=3){
        config_path = std::string(argv[2]);
      }
      if(argc>=4){
        verilogPath = std::string(argv[3]);
      }
      if(argc>=5){
        resultPath = std::string(argv[4]);
      }
      print("Getting the unreachbale state of module\n");
    }
  }

  readSimVcdFiles(sim_path);

  readSmtVcdFiles(smt_path);  

  

  if(runSygus)
    makeSyGusFile(config_path,"sygus.sl");
  
  if(runGetUnreachableState)
    getUnreachableState(config_path,verilogPath,resultPath);

  for(auto &trace : traces){
    delete trace;
  }
  //std::cout<<"\nThe default paths are used\n"; 

  return 0;  
}