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
    printDebug("Find an unreachable state\n",1);
    sygus.addConstraints(state,false);
  }
  else{
    printDebug("The state is reachable\n",1);
    sygus.addConstraints(state,true);
  }

  sygus.printSysgusPath("sygus.sl");
  
  printDebug("Sygus file generated to "+std::string("sygus.sl")+"\n",1);
  
  std::string sygusResult = runCVC5Sygus("sygus.sl");
  printDebug("the cvc5 result is: \n"+sygusResult,1);

  SmtFunctionParser smtParser;
  SygusFunction* func = (SygusFunction*)smtParser.parseSmtFunction(sygusResult);

  print("The assertion is : \n");
  print(func->getBodyVerilogExpr());

  bool safetyResult = vc.checkExprSafety(func);
  if(safetyResult){
    print("The assertion is safe\n");
  }
  else{
    print("The assertion is not safe\n");
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
    printDebug("Number of VCD files: " + std::to_string(vcdFileCount)+"\n",1);
  }
  catch(const std::filesystem::filesystem_error& e){
    std::cerr << "Error: " << e.what() << std::endl;
  }
  if(vcdFileCount > 0){
    printDebug("List of VCD files: \n",1);
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
  print("Start to run cvc5\n");
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