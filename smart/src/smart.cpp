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
#include <fstream>
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
std::string resultFileDir = "";
std::string resultRemoveVariablesPath = "";
std::string generateSMTResultPath();

bool writeStringToFile(const std::string&, const std::string&);

void removeSignals(std::string);

int main(int argc, char* argv[]){
  int timeOut = 0;

  if(argc!=5){
    print("Usage: ./smart <verilog_file_name> <module_name> <result_file_dir>\n");
    return -1;
  }
  else{
    verilogSrcPath = argv[1];
    moduleName = argv[2];
    resultFileDir = argv[3];
    resultRemoveVariablesPath = argv[4];
  }

  StateMaker::setSeed(42);
  SmtFunctionParser parser;
  module = new Module(moduleName);
  sygus = new SyGuSGenerater();
  checker = new VerilogChecker(verilogSrcPath,ebmcPath);
  
  module->addTracesfromDir(SIM,sim_path);
  module->addTracesfromDir(SMT,smt_path);

  // we first get all the signals from the module
  signals = module->getAllSignals();
  removeSignals(resultRemoveVariablesPath);
  //TODO: add two functions
  //1. intersection of signals and variables
  

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

  //if the random state is unreachable, then we need to add the constraints
  if(!checker->checkStateReachability(randomState)){
    printDebug("The random state is unreachable\n",1);
    sygus->addConstraints(randomState,false);
  }

  sygus->printSysgusPath("sygus.sl");
  print("\tFinish generating sygus file");

  std::string Cvc5result = sygus->runCVC5Sygus("sygus.sl");
  SygusFunction* sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);
  
  
  print("\twe get assertion:" + sygusfunc->getBodyVerilogExpr());

  std::string SMTVCDfilePath = generateSMTResultPath();
  bool verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
  print("\tFinish checking the assertion: "+std::to_string(verifiedResult));
  print("\tTrace goes to VCD file: "+SMTVCDfilePath);

  while(!verifiedResult){
    if(timeOut++>20||verifiedResult){
      print("Time out\n");
      break;
    }
    print("In looptime "+std::to_string(timeOut)+":");

    module->addTrace(SMT,SMTVCDfilePath);
    
    Constrains c = module->getConstrain(SMTVCDfilePath,signals);

    sygus->addConstraints(c.constraints,c.isTrue);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+c.tracePath,c.isTrue);
    

    // randomState = stateMaker->makeRandomState();

    // if(checker->checkStateReachability(randomState)){
    //   sygus->addConstraints(randomState,false);
    // }
   
    sygus->printSysgusPath("sygus.sl");
    print("\tFinish generating sygus file");

    Cvc5result = sygus->runCVC5Sygus("sygus.sl");

    sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);
    print("\twe get assertion:" + sygusfunc->getBodyVerilogExpr());

    SMTVCDfilePath = generateSMTResultPath();
    verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
    print("\tFinish checking the assertion: "+std::to_string(verifiedResult));
    print("\tTrace goes to VCD file: "+SMTVCDfilePath);

  }
  if(verifiedResult){
    print("Last assertion is verified\n");
    print("The property is "+sygusfunc->getBodyVerilogExpr());
    writeStringToFile(resultFileDir,sygusfunc->getBodyVerilogExpr());
  }
  else{
    print("All assertion is not verified\n");
  }
  return 0;
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

bool writeStringToFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename); 
    if (!outfile.is_open()) {        
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return false;
    }
    outfile << content; 
    outfile.close();    
    return true;     
}

void removeSignals(std::string removeVariablesPath){
  // std::string removeVariablesPath = "/home/magna/Desktop/smartVerilog/smart/runtime/variables/removeVariables.txt";
  std::vector<std::string> lines;
  std::ifstream file(removeVariablesPath); // Open the file

  if (!file.is_open()) {
      std::cerr << "Error: Unable to open file " << removeVariablesPath << std::endl;
      // exit(1);
      // return lines; // Return an empty vector if the file cannot be opened
  }

  std::string line;
  while (std::getline(file, line)) { // Read the file line by line
      lines.push_back(line); // Add each line to the vector
  }

  for(auto &line : lines){
    printDebug("Removing signal: "+line,2);
  }
  for(auto &line : lines){
    for(auto it = signals->begin(); it != signals->end();){
      if(it->name == line){
        it = signals->erase(it);
      }
      else{
        ++it;
      }
    }
  }
}