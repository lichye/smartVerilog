#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include <ctime>
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

SyGuSGenerater* sygus;

std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";
std::string verilogSrcPath = "runtime/verilog/addsub.sv";
std::string ebmcPath = "runtime/ebmc/addsub.sv";


void generateTrace(std::string Path,TraceType type); 
void addConstraintsfromTrace();
std::string runCVC5Sygus(std::string);
std::string generateSMTResultPath();
void setUpSignal();
int RunSmart(int);

int main(int argc, char* argv[]){
  
  int looptime = 1;
  while (looptime < 10)
  { 
    sygus = new SyGuSGenerater();
    setUpSignal();
    
    int result = RunSmart(looptime);
    if(result == 0)
      break;
    looptime++;
    delete sygus;
  }

  print("Stop after loop for "+std::to_string(looptime)+" times\n");
  delete signals;
  return 0;
}

int RunSmart(int loopTime){ 
  assert(traces.empty());
  assert(signals != nullptr);

  sygus->setSignals(signals);

  VerilogChecker vc(verilogSrcPath,ebmcPath);

  generateTrace(sim_path,TraceType::SIM);
  generateTrace(smt_path,TraceType::SMT);

  //add the constraints from the traces to the sygus file
  addConstraintsfromTrace();

  if(runRandomState){
    print("Try to get an unreachable state\n");
    StateMaker sm(&traces,signals);
    State* state = sm.makeRandomState();
    bool checkResult = false;
    while(!checkResult){
      print("Try to get an unreachable state\n");
      print(state->toString());

      checkResult = vc.checkStateReachability(state);
      if(checkResult){
        printDebug("Find an unreachable state\n",1);
        sygus->addConstraints(state,false);
      }
      else{
        printDebug("The state is reachable\n",1);
        // sygus->addConstraints(state,true);
      }
    }
  }

  sygus->printSysgusPath("sygus.sl");
  
  printDebug("Sygus file generated to "+std::string("sygus.sl")+"\n",1);
  std::string sygusResult = runCVC5Sygus("sygus.sl");

  printDebug("the cvc5 result is: \n"+sygusResult,1);

  SmtFunctionParser smtParser;
  SygusFunction* func = (SygusFunction*)smtParser.parseSmtFunction(sygusResult);

  print("Found an assertion in "+std::to_string(loopTime)+" times: \n");
  print(func->getBodyVerilogExpr());

  bool safetyResult = vc.checkExprSafety(func,generateSMTResultPath());

  //clean up
  for(auto &trace : traces){
    delete trace;
  }
  traces.clear();

  if(safetyResult){
    print("The assertion is safe\n");
    return 0;
  }
  else{
    print("The assertion is not safe\n");
    return 1;
  }
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
  assert(signals != nullptr && signals->size()!=0);

  printDebug("Adding constraints from the traces\n",1);
  printDebug("Number of traces: "+std::to_string(traces.size())+"\n",1);
  for(auto &trace : traces){
    printDebug("Adding constraints from the trace: "+trace->getPath()+"\n",1);
    std::vector<std::vector<Value*>>* constraints = trace->getConstraints(signals);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+trace->getPath(),true);
    assert(constraints->size() !=0);
    sygus->addConstraints(*constraints,true);
  }
}

std::string runCVC5Sygus(std::string sygusPath){
  print("Running cvc5\n");
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

std::string generateSMTResultPath(){
  auto now = std::chrono::system_clock::now();

  std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

  auto duration = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&timeNow), "%Y%m%d_%H%M%S") 
      << "_" << std::setw(3) << std::setfill('0') << millis;

  std::string filename="";
  filename +=smt_path;
  filename +="/ebmc_result_";
  filename += oss.str();
  filename +=".vcd";
  return filename;
}

void setUpSignal(){
  printDebug("Run setUp\n",1);
  SignalGather sg(config_path);
  signals = sg.getAllSignals();
  printDebug("Number of signals: "+std::to_string(signals->size())+"\n",1);
}

