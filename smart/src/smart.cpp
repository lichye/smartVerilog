#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include "Trace.h"
#include "SyGuSGenerater.h"
#include "State.h"
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
        
        std::cout << "Number of VCD files: " << vcdFileCount << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  //Loop over the trace file system to get the traces from VCD files
  std::cout<<"List of the VCD files input:\n";

  for(auto &vcdFile : simVcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(TraceType::SIM, vcdFile);
    std::cout<<"Created new trace object"<<std::endl;
    traces.push_back(trace);
    std::cout<<"SIM trace object inserted into the traces vector"<<std::endl;
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
        
        std::cout << "Number of VCD files: " << vcdFileCount << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  //Loop over the trace file system to get the traces from VCD files
  std::cout<<"List of the VCD files input:\n";

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
  
  delete signals;
}

void getUnreachableState(std::string configPath){
  SignalGather sg(configPath);
  std::vector<Signal>* signals = sg.getAllSignals();
  StateMaker sm(&traces,signals);
  State* state = sm.makeRandomState();
  
  print(state->toString());
  
  delete signals;
}

int main(int argc, char* argv[]){
  bool runSygus = false;
  bool runGetUnreachableState = false;

  std::string sim_path = "runtime/sim_results";
  std::string smt_path = "runtime/smt_results";
  std::string config_path = "User/config.ini";

  if(argc==1){
    std::cout<<"No arguments provided, using default paths\n";
    runSygus = true; 
  }
  //when there is two arguments
  else if(argc==2){
    //requires of help
    if(std::string(argv[1])=="-h"){
      print("Usage: ./smart sygus/state [sim_path] [smt_path] [config_path] [result_path]");
      print("Default paths are used if no arguments are provided");
      return 0;
    }
    if(std::string(argv[1])=="--sygus"||std::string(argv[1])=="-s"){
      runSygus = true;
    }
    if(std::string(argv[1])=="--unreachable"||std::string(argv[1])=="-u"){
      runSygus = false;
      runGetUnreachableState = true;
      print("Running get unreachable state");
    }
  }
  
  readSimVcdFiles(sim_path);

  readSmtVcdFiles(smt_path);  

  if(runSygus)
    makeSyGusFile(config_path,"sygus.sl");
  
  if(runGetUnreachableState)
    getUnreachableState(config_path);

  for(auto &trace : traces){
    delete trace;
  }
  //std::cout<<"\nThe default paths are used\n"; 

  return 0;  
}