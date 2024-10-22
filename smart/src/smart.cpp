#include <iostream>
#include <filesystem>
#include <vector>
#include "Trace.h"
namespace fs = std::filesystem;

int main(){
  //make the path to the vcd files containing directory
  std::string path = "tests/vcd_files";
  std::vector<std::string> vcdFiles;
  std::vector<Trace*> traces;
  int vcdFileCount = 0;
  try {
        for (const auto& entry : fs::directory_iterator(path)) {
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
    //t.printScope();
  }

  Signal signal;
  signal.moduleName = "counter";
  signal.name = "out";
  signal.type = SignalType::BITS;
  signal.lindex = 7;
  signal.rindex = 0;

  //get the vector of a signal's values
  std::vector<Value*>* values=traces[0]->getSignalValue(signal);
  
  
  for(auto it = values->begin();it != values->end();++it){
    Value* val = *it;
    std::cout<<val->toString()<<std::endl;
  }

  for(auto &trace : traces){
    delete trace;
  } 

  return 0;  
}