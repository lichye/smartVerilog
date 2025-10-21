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
#include "setups.h"
#include <fcntl.h>      
#include <unistd.h>     
#include <sys/file.h>  
#include <sys/types.h>

//global variables
int latency = 0;
std::string sim_path    = "runtime/sim_results";
std::string smt_path    = "runtime/smt_results";
std::string config_path = "User/config.ini";
std::string ebmcPath = "runtime/formal/formal.sv";
std::string ebmcReachable = "runtime/formal/reachable.sv";
std::string moduleName = "";
std::string verilogSrcPath = "";
std::string resultFileDir = "";
std::string generateSMTResultPath();
std::string initVariables = "";
std::string currentDir = "";
std::string core_id = "0";
std::string Cvc5result;
std::vector<Signal>* signals;
SyGuSGenerater* sygus;
Module* module;
VerilogChecker* checker;
StateMaker* stateMaker;
Timer* timer;
SmtFunctionParser parser;

//Help function
void modifySignals(std::string,int mode);
void intersectionSignals(std::string);
void printCoreSignals(std::vector<Signal>*);
bool writeStringToFile(const std::string&, const std::string&,std::ios::openmode);
bool append_with_lock_posix(const std::string&, const std::string&);
SygusFunction* parseSygusFunction(const std::string&,SyGuSGenerater*,bool);
State* createNegativeState();

int main(int argc, char* argv[]){
  if(argc!=7){
    print("Usage: ./smart.out <currentDir> <topmodule> <result_file_dir> <core_id> <Latency>\n");
    return -1;
  }
  else{
    currentDir = argv[1];
    moduleName = argv[2];
    resultFileDir = argv[3];
    initVariables = argv[4];
    core_id = argv[5];
    latency = std::stoi(argv[6]);
  }

  bool isLTL = (latency > 0);
  verilogSrcPath = currentDir + "/runtime/verilog/"+moduleName+".sv";
  smt_path = currentDir + "/runtime/smt_results/"+core_id;
  std::string sygusPath = "runtime/smt_results/sygus"+core_id+".sl";
  fs::create_directory(smt_path);
  StateMaker::setSeed(42);

  module = new Module(moduleName);
  sygus = new SyGuSGenerater();
  checker = new VerilogChecker(verilogSrcPath,currentDir,BackEndSolver::EBMC);
  timer = new Timer();  

  checker->setTopModule(moduleName);
  checker->setTimer(timer);
  
  //Read from trace & Generate Signals
  module->addTracesfromDir(SIM,sim_path);
  module->addTracesfromDir(SMT,smt_path);
  signals = module->getAllSignals();
  modifySignals(initVariables,1);
  std::sort(signals->begin(),signals->end());
  sygus->setSignals(signals);
  checker->setSignals(signals);
  stateMaker = new StateMaker(signals);

  printCoreSignals(signals);
  print("In looptime 0:");

  //Add constrains into sygus
  std::vector<Constrains> constraints = module->getAllConstraints(signals);
  for(auto &constrain : constraints){
    sygus->addConstraints(constrain.constraints,constrain.isTrue);
    sygus->addConstraintComments("Getting constraints from the trace :\t"+constrain.tracePath,constrain.isTrue);
  }

  State* negativeState = createNegativeState();
  sygus->addConstraints(negativeState,false);
  if(latency == 0){
    sygus->printSysgusPath(sygusPath);
  }
  else{
    sygus->printLTLSygusPath(sygusPath,latency);
  }
  
  SygusFunction* sygusfunc = parseSygusFunction(sygusPath, sygus,isLTL);
  print("\twe get assertion:" + sygusfunc->getBodyVerilogExpr());

  std::string SMTVCDfilePath = generateSMTResultPath();
  bool verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
  print("\tIts correctness is: "+std::to_string(verifiedResult));
  printDebug("\tTrace goes to VCD file: "+SMTVCDfilePath,1);

  int timeOut = 0;
  int mostRun = signals->size();

  while(!verifiedResult){
    if(timeOut++>mostRun||verifiedResult){
      print("Time out\n");
      break;
    }
    print("In looptime "+std::to_string(timeOut)+":");
    module->addTrace(SMT,SMTVCDfilePath);
    Constrains c = module->getConstrain(SMTVCDfilePath,signals);
    Constrains new_c = checker->fixupConstrains(c);
    sygus->addConstraints(new_c.constraints,c.isTrue);
    sygus->addConstraintComments("Getting constraints from the trace :\t"+c.tracePath,c.isTrue);
    if(isLTL){
      sygus->printLTLSygusPath(sygusPath,latency);
    }
    else{
      sygus->printSysgusPath(sygusPath);
    }
    sygusfunc = parseSygusFunction(sygusPath, sygus,isLTL);    
    print("\tGet assertion:" + sygusfunc->getBodyVerilogExpr());

    SMTVCDfilePath = generateSMTResultPath();
    verifiedResult = checker->checkExprSafety(sygusfunc,SMTVCDfilePath);
    print("\tIts correctness is: "+std::to_string(verifiedResult));
    printDebug("\tTrace goes to VCD file: "+SMTVCDfilePath,1);

  }
  if(verifiedResult){
    writeStringToFile(resultFileDir,sygusfunc->getBodyVerilogExpr(),std::ios::out);
    writeStringToFile("runtime/SygusResult.sl",Cvc5result,std::ios::app);
    writeStringToFile("runtime/CompareResult.txt",sygusfunc->getBodyVerilogExpr()+":\n"+Cvc5result+"\n",std::ios::app);
    print("Last assertion "+sygusfunc->getBodyVerilogExpr()+" is verified and written to "+resultFileDir);
  }
  else{
    print("All assertion is not verified\n");
  }

  //append_with_lock_posix("log.txt","This loop's Result: "+std::to_string(verifiedResult)+"\n"+timer->printTime()+"\n");
  writeStringToFile("log.txt","This "+core_id+" 's Result: "+std::to_string(verifiedResult)+"\n"+timer->printTime()+"\n",std::ios::out|std::ios::app);
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

bool append_with_lock_posix(const std::string& path, const std::string& content) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0666);
    if (fd == -1) return false;
    if (::flock(fd, LOCK_EX) != 0) { ::close(fd); return false; }
    const char* p = content.data();
    size_t n = content.size();
    while (n > 0) {
        ssize_t w = ::write(fd, p, n);
        if (w < 0) { ::flock(fd, LOCK_UN); ::close(fd); return false; }
        p += w; n -= (size_t)w;
    }

    ::flock(fd, LOCK_UN);
    ::close(fd);
    return true;
}

void modifySignals(std::string removeVariablesPath,int mode){
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

SygusFunction* parseSygusFunction(const std::string& sygusPath,SyGuSGenerater* sygus,bool isLTL){
  SygusFunction* sygusfunc = nullptr;
  try{
      timer->start(CVC5_Timer);
      Cvc5result = sygus->runCVC5Sygus(sygusPath);
      timer->stop(CVC5_Timer);
      sygusfunc = (SygusFunction*) parser.parseSmtFunction(Cvc5result, isLTL);
    }
    catch(const std::exception& e){
      timer->stop(CVC5_Timer);
      writeStringToFile("log.txt","This "+core_id+" 's Result: -1(Sygus infeasible)\n"+timer->printTime()+"\n",std::ios::out|std::ios::app);
      fs::remove_all(smt_path);
      exit(-1);
  }
  return sygusfunc;
}

void printCoreSignals(std::vector<Signal>* signals){
  std::string coreSignals="Core Signals:";
  for(auto &signal : *signals){
    coreSignals += "\t" + signal.name;
  }
  print(coreSignals);
}

State* createNegativeState(){
  State* negativeState = stateMaker->makeRandomState();
  int loopTime = 0;
  while(checker->checkStateReachability(negativeState)){
    sygus->addConstraintComments("Getting constraints from the random state",true);
    negativeState = stateMaker->makeRandomState();
    if(loopTime++>3){
      break;
    }
  }
  return negativeState;
}