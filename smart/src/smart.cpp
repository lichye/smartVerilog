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
#include "Module.h"
namespace fs = std::filesystem;

//global variables

std::vector<Signal>* signals;
SyGuSGenerater* sygus;
Module* module;
VerilogChecker* checker;
StateMaker* stateMaker;

std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";
std::string ebmcPath = "runtime/ebmc/formal.sv";
std::string ebmcReachable = "runtime/ebmc/reachable.sv";
std::string moduleName = "";
std::string verilogSrcPath = "";

std::string generateSMTResultPath();


int main(int argc, char* argv[]){
  
  if(argc!=3){
    print("Usage: ./smart <verilog_file_name> <module_name>\n");
    return -1;
  }
  else{
    verilogSrcPath = argv[1];
    moduleName = argv[2];
  }

  StateMaker::setSeed(42);
  SmtFunctionParser parser;
  module = new Module(moduleName);
  sygus = new SyGuSGenerater();
  checker = new VerilogChecker(verilogSrcPath,ebmcPath);
  
  module->addTracesfromDir(SIM,sim_path);
  module->addTracesfromDir(SMT,smt_path);

  signals = module->getAllSignals();

  sygus->setSignals(signals);

  stateMaker = new StateMaker(signals);

  std::vector<Constrains> constraints = module->getAllConstraints(signals);
  for(auto &constrain : constraints){
    sygus->addConstraints(constrain.constraints,constrain.isTrue);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+constrain.tracePath,constrain.isTrue);
  }

  printDebug("Adding random state",2);
  State* randomState = stateMaker->makeRandomState();
  printDebug("Get a random state: "+randomState->toString(),2);


  if(checker->checkStateReachability(randomState)){
    printDebug("The random state is unreachable\n",1);
    sygus->addConstraints(randomState,false);
  }

  sygus->printSysgusPath("sygus.sl");
  std::string Cvc5result = sygus->runCVC5Sygus("sygus.sl");
  SygusFunction* sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);
  
  std::string SMTVCDfilePath = generateSMTResultPath();
  bool verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
  
  int timeOut = 0;
  
  while(!verifiedResult){
    module->addTrace(SMT,SMTVCDfilePath);
    
    Constrains c = module->getConstrain(SMTVCDfilePath,signals);

    sygus->addConstraints(c.constraints,c.isTrue);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+c.tracePath,c.isTrue);
    
    randomState = stateMaker->makeRandomState();

    if(checker->checkStateReachability(randomState)){
      sygus->addConstraints(randomState,false);
    }

    sygus->printSysgusPath("sygus.sl");
    
    Cvc5result = sygus->runCVC5Sygus("sygus.sl");

    sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);

    SMTVCDfilePath = generateSMTResultPath();
    verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
    
    print("In looptime we get assertion:" + sygusfunc->getBodyVerilogExpr() + "\n");
    print("We have tried loop over "+std::to_string(timeOut)+" times\n");
    if(timeOut++>20){
      print("Time out\n");
      break;
    }
    if(verifiedResult)
      break;
  }
  return 0;
}

void runSmart(){

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
