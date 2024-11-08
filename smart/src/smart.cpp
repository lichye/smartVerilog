#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include "Trace.h"
#include "SyGuSGenerater.h"
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
int main(){
  //make the path to the vcd files containing directory
  std::string sim_path = "runtime/sim_files";
  std::string smt_path = "runtime/smt_files";
  std::string config_path = "User/config.ini";

  
  readSimVcdFiles(sim_path);

  readSmtVcdFiles(smt_path);  

  //defualt sg will read from config.ini file
  

  SignalGather sg(config_path);
  SyGuSGenerater sygus;


  //read from SignalGather
  std::vector<Signal>* signals = sg.getAllSignals();

  //read from the tracer
  //std::vector<Signal>* signals = traces[0]->getAllSignals("addsub");

  sygus.setSignals(signals);

  std::cout<<"Signals set"<<std::endl;
  
  for(auto &trace : traces){
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    //std::cout<<"Constraints from trace: "<<trace->getPath()<<" has size of "<<constraints.size()<<std::endl;
    sygus.addConstrainComments("Getting constraints from the trace :\t"+trace->getPath(),true);
    //sygus.addComments("test of the coments",true);
    sygus.addConstraints(*constraints,true);
  }

  sygus.printSysgusPath("sygus.sl");


  for(auto &trace : traces){
    delete trace;
  } 

  return 0;  
}