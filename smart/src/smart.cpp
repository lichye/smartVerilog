#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include <cassert>
#include <ctime>
#include <fstream>
#include <algorithm>
namespace fs = std::filesystem;

#include "Trace.h"
#include "SyGuSGenerater.h"
#include "State.h"
#include "StateMaker.h"
#include "VerilogChecker.h"
#include "SmtFunctionParser.h"
#include "Module.h"
#include "Timer.h"

//global variables

std::vector<Signal>* signals;
SyGuSGenerater* sygus;
Module* module;
VerilogChecker* checker;
StateMaker* stateMaker;
Timer* timer;


std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";
std::string ebmcPath = "runtime/formal/formal.sv";
std::string ebmcReachable = "runtime/formal/reachable.sv";
std::string moduleName = "";
std::string verilogSrcPath = "";
std::string resultFileDir = "";
std::string resultRemoveVariablesPath = "";
std::string generateSMTResultPath();
std::string initVariables = "";
std::string currentDir = "";
std::string core_id = "0";

bool writeStringToFile(const std::string&, const std::string&,std::ios::openmode);

void modifySignals(std::string,int mode);
void intersectionSignals(std::string);

int main(int argc, char* argv[]){
  // print("Smart Verilog 0.1");
  int timeOut = 0;

  if(argc!=6){
    print("Usage: ./smart.out <currentDir> <topmodule> <result_file_dir> <core_id>\n");
    return -1;
  }
  else{
    currentDir = argv[1];
    moduleName = argv[2];
    resultFileDir = argv[3];
    initVariables = argv[4];
    core_id = argv[5];
  }

  verilogSrcPath = currentDir + "/runtime/verilog/"+moduleName+".sv";
  resultRemoveVariablesPath = currentDir + "/runtime/variables/removeVariables.txt";
  smt_path = currentDir + "/runtime/smt_results/"+core_id;
  std::string sygusPath = "runtime/smt_results/sygus"+core_id+".sl";
  fs::create_directory(smt_path);
  // initVariables = currentDir + "/runtime/variables/initVariables.txt";

  StateMaker::setSeed(42);
  SmtFunctionParser parser;
  module = new Module(moduleName);
  sygus = new SyGuSGenerater();
  // checker = new VerilogChecker(verilogSrcPath,currentDir,BackEndSolver::SBY);
  checker = new VerilogChecker(verilogSrcPath,currentDir,BackEndSolver::EBMC);
  timer = new Timer();  

  checker->setTopModule(moduleName);
  checker->setTimer(timer);

  // this means we does not care about the initial state
  // checker->setModuleTime("##1");
  
  module->addTracesfromDir(SIM,sim_path);
  module->addTracesfromDir(SMT,smt_path);

  // we first get all the signals from the module
  signals = module->getAllSignals();

  modifySignals(initVariables,1);

  print("This Core Loop's Variables:");
  for(auto &signal : *signals){
    print("\t"+signal.name);
  }
  print("");

  std::sort(signals->begin(),signals->end());

  sygus->setSignals(signals);
  checker->setSignals(signals);

  stateMaker = new StateMaker(signals);

  std::vector<Constrains> constraints = module->getAllConstraints(signals);
  for(auto &constrain : constraints){
    //check will help
    // constrain = checker->fixupConstrains(constrain);
    sygus->addConstraints(constrain.constraints,constrain.isTrue);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+constrain.tracePath,constrain.isTrue);
  }

  print("In looptime "+std::to_string(timeOut)+":");

  State* randomState = stateMaker->makeRandomState();
  int loopTime = 0;

  while(checker->checkStateReachability(randomState)){
    sygus->addConstrainComments("Getting constraints from the random state",true);
    randomState = stateMaker->makeRandomState();
    if(loopTime++>3){
      break;
    }
  }
  sygus->addConstraints(randomState,false);


  print("\tFinish generating random state");

  sygus->printSysgusPath(sygusPath);
  print("\tFinish generating sygus file");

  timer->start(CVC5_Timer);
  std::string Cvc5result;
  SygusFunction* sygusfunc; 
  try{
    Cvc5result = sygus->runCVC5Sygus(sygusPath);
    timer->stop(CVC5_Timer);
    sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);
  }
  catch(const std::exception& e){
    timer->stop(CVC5_Timer);
    writeStringToFile("log.txt",timer->printTime(),std::ios::out|std::ios::app);
    fs::remove_all(smt_path);
    return -1;
  }
 
  print("\twe get assertion:" + sygusfunc->getBodyVerilogExpr());

  std::string SMTVCDfilePath = generateSMTResultPath();
  bool verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
  print("\tFinish checking the assertion: "+std::to_string(verifiedResult));
  print("\tTrace goes to VCD file: "+SMTVCDfilePath);

  int mostRun = signals->size();
  print("This smart will run "+std::to_string(mostRun)+" times");
  while(!verifiedResult){
    if(timeOut++>mostRun||verifiedResult){
      print("Time out\n");
      break;
    }
    print("In looptime "+std::to_string(timeOut)+":");
    print("\tGathering signals from the SMT traces");
    module->addTrace(SMT,SMTVCDfilePath);
    Constrains c = module->getConstrain(SMTVCDfilePath,signals);
    Constrains new_c = checker->fixupConstrains(c);
    sygus->addConstraints(new_c.constraints,c.isTrue);
    // print("\tFinish getting constraints from SMT trace: "+c.tracePath);
    sygus->addConstrainComments("Getting constraints from the trace :\t"+c.tracePath,c.isTrue);
    
    sygus->printSysgusPath(sygusPath);
    print("\tFinish generating sygus file");

    try{
      Cvc5result = sygus->runCVC5Sygus(sygusPath);
      timer->stop(CVC5_Timer);
      sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result);
    }
    catch(const std::exception& e){
      timer->stop(CVC5_Timer);
      writeStringToFile("log.txt",timer->printTime(),std::ios::out|std::ios::app);
      fs::remove_all(smt_path);
      return -1;
    }
    
    print("\twe get assertion:" + sygusfunc->getBodyVerilogExpr());

    SMTVCDfilePath = generateSMTResultPath();
    verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
    print("\tFinish checking the assertion: "+std::to_string(verifiedResult));
    print("\tTrace goes to VCD file: "+SMTVCDfilePath);

  }
  if(verifiedResult){
    print("Last assertion is verified\n");
    print("The property is "+sygusfunc->getBodyVerilogExpr());
    writeStringToFile(resultFileDir,sygusfunc->getBodyVerilogExpr(),std::ios::out);
    writeStringToFile("SygusResult.sl",Cvc5result,std::ios::app);
    
    print("The property is written to "+resultFileDir);
  }
  else{
    print("All assertion is not verified\n");
    fs::remove(smt_path);
    return -1;
  }
  writeStringToFile("log.txt",timer->printTime(),std::ios::out|std::ios::app);
  
  fs::remove_all(smt_path);
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

bool writeStringToFile(const std::string& filename, const std::string& content, std::ios::openmode mode) {
    // std::ios::openmode mode = std::ios::out | std::ios::app;
    std::ofstream outfile(filename, mode);

    // std::ofstream outfile(filename); 
    if (!outfile.is_open()) {        
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return false;
    }
    outfile << content; 
    outfile.close();    
    return true;     
}

void modifySignals(std::string removeVariablesPath,int mode){
  // std::string removeVariablesPath = "/home/magna/Desktop/smartVerilog/smart/runtime/variables/removeVariables.txt";
  std::vector<std::string> lines;
  std::ifstream file(removeVariablesPath); // Open the file

  if (!file.is_open()) {
      std::cerr << "Error: Unable to open file " << removeVariablesPath << std::endl;
      return;
      // exit(1);
      // return lines; // Return an empty vector if the file cannot be opened
  }

  std::string line;
  while (std::getline(file, line)) { // Read the file line by line
      lines.push_back(line); // Add each line to the vector
  }
  if(mode == 0){
    for(auto &line : lines){
      //if in mode 0, we remove the signal with the same name in file
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

  if(mode == 1){
    // we will keep the signals inside the file
    std::vector<Signal> tempSignals;
    for(auto &line : lines){
      for(auto &signal : *signals){
        if(signal.name == line){
          tempSignals.push_back(signal);
        }
      }
    }
    signals->clear();
    for(auto &signal : tempSignals){
      signals->push_back(signal);
    }
  }


}
