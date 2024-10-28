#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include "Trace.h"
#include "SyGuSGenerater.h"
namespace fs = std::filesystem;

int main(){
  //make the path to the vcd files containing directory
  std::string vcd_path = "tests/vcd_files";
  std::string config_path = "tests/config.ini";

  std::vector<std::string> vcdFiles;
  std::vector<Trace*> traces;

  int vcdFileCount = 0;
  try {
        for (const auto& entry : fs::directory_iterator(vcd_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vcd") {
                ++vcdFileCount;
                vcdFiles.push_back(entry.path().string());
            }
        }
        
        std::cout << "Number of VCD files: " << vcdFileCount << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  //assert(false&&"finish count the vcd files");

  //Loop over the trace file system to get the traces from VCD files
  std::cout<<"List of the VCD files input:\n";

  for(auto &vcdFile : vcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(TraceType::VCD, vcdFile);
    std::cout<<"Created new trace object"<<std::endl;
    traces.push_back(trace);
    std::cout<<"trace object inserted into the traces vector"<<std::endl;
  }

  //defualt sg will read from config.ini file
  SignalGather sg;

  Signal s1 =sg.getOneSignal(0);
  Signal s2 =sg.getOneSignal(1);

  //get the constrains from the traces
  //traces[0]->printDebug();

  std::vector<Signal> signals = {s1,s2};

  SyGuSGenerater sygus;

  sygus.setSignals(signals);

  for(auto &trace : traces){
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    sygus.addConstraints(*constraints);
  }

  //sygus.debugPrint();
  sygus.printSysgusPath("sygus.sl");

  //read back from the sygus files
    
  //delete the traces
  for(auto &trace : traces){
    delete trace;
  } 

  return 0;  
}