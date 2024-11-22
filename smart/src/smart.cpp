#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include "Trace.h"
#include "SyGuSGenerater.h"
#include "State.h"
#include "StateMaker.h"
#include "VerilogChecker.h"
#include "SmtFunctionParser.h"
namespace fs = std::filesystem;

//globak variables
std::vector<Trace*> traces;
std::vector<Signal>* signals;

SyGuSGenerater sygus;

std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";
std::string verilogSrcPath = "runtime/verilog/addsub.sv";
std::string ebmcPath = "runtime/ebmc/addsub.sv";


void generateTrace(std::string Path,TraceType type); 
void addConstraintsfromTrace();

std::string runCVC5Sygus(std::string);

int main(int argc, char* argv[]){
  
  bool useTraces = false;
  bool testGetUnreachable = false;

  SignalGather sg(config_path);
  VerilogChecker vc(verilogSrcPath,ebmcPath);

  signals = sg.getAllSignals();
  sygus.setSignals(signals);
  
  generateTrace(sim_path,TraceType::SIM);
  generateTrace(smt_path,TraceType::SMT);

  //add the constraints from the traces to the sygus file
  addConstraintsfromTrace();


  StateMaker sm(&traces,signals);

  State* state = sm.makeRandomState();

  bool checkResult = vc.checkStateReachability(state);

  if(!checkResult){
    print("Find an unreachable state\n");
    sygus.addConstraints(state,false);
  }
  else{
    print("The state is reachable\n");
    sygus.addConstraints(state,true);
  }

  sygus.printSysgusPath("sygus.sl");
  
  print("Sygus file generated to "+std::string("sygus.sl")+"\n");
  
  std::string sygusResult = runCVC5Sygus("sygus.sl");
  print("the cvc5 result is: \n"+sygusResult);

  SmtFunctionParser smtParser;
  SygusFunction* func = (SygusFunction*)smtParser.parseSmtFunction(sygusResult);

  print("The relation is : \n");
  print(func->getBodyVerilogExpr());

  bool safetyResult = vc.checkExprSafety(func);
  if(safetyResult){
    print("The relation is safe\n");
  }
  else{
    print("The relation is not safe\n");
  }

  //clean up
  for(auto &trace : traces){
    delete trace;
  }

  return 0;  
}


void generateTrace(std::string Path,TraceType type){
  int vcdFileCount = 0;
  std::vector<std::string> vcdFiles;
  try{
    for(const auto& entry : fs::directory_iterator(Path)){
      if(entry.is_regular_file() && entry.path().extension() == ".vcd"){
        ++vcdFileCount;
        vcdFiles.push_back(entry.path().string());
      }
    }
    print("Number of VCD files: " + std::to_string(vcdFileCount)+"\n");
  }
  catch(const std::filesystem::filesystem_error& e){
    std::cerr << "Error: " << e.what() << std::endl;
  }
  if(vcdFileCount > 0){
    print("List of VCD files: \n");
  }
  for(auto &vcdFile : vcdFiles){
    std::cout<<vcdFile<<std::endl;
    Trace* trace = new Trace(type, vcdFile);
    traces.push_back(trace);
  }
}

void addConstraintsfromTrace(){
  for(auto &trace : traces){
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    sygus.addConstrainComments("Getting constraints from the trace :\t"+trace->getPath(),true);
    sygus.addConstraints(*constraints,true);
  }
}



std::string runCVC5Sygus(std::string sygusPath){
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