#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
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
        
        std::cout << "Number of .vcd files: " << vcdFileCount << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
  }

  //Loop over the trace file system to get the traces from VCD files
  for(auto &vcdFile : vcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(TraceType::VCD, vcdFile);
    traces.push_back(trace);
  }

  //The signalGather will pick good signals
  SignalGather sg;

  Signal s1 =sg.getOneSignal(0);
  Signal s2 =sg.getOneSignal(1);

  //get the constrains from the traces
  std::vector<std::vector<Value*>>* constraints = traces[0]->getConstraints({s1,s2});
  

  //when we put the signals and the constraints into the sygus generator
  SyGuSGenerater sygus;
  sygus.addSignalsAndConstraints(*sg.getAllSignals(),*constraints);
  sygus.printSysgusPath("sygus.sl");
  
  //delete the traces
  for(auto &trace : traces){
    delete trace;
  } 

  return 0;  
}