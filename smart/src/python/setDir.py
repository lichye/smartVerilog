import os
import subprocess

## This function is used to setup the runtime directory
def setup(current_path):
    if not os.path.exists(current_path+"/runtime"):
        os.makedirs(current_path+"/runtime")

    runtime_formal_path = current_path+"/runtime/formal/"

    runtime_verilog_path = current_path+"/runtime/verilog/"

    runtime_cocotb_path = current_path+"/runtime/cocotb/"

    sim_build_path = current_path+"/sim_build"

    user_verilog_path = current_path+"/user"
    
    sim_target_dir = current_path+"/runtime/sim_results"

    smt_target_dir = current_path+"/runtime/smt_results"

    variables_dir = current_path+"/runtime/variables"

    result_dir = current_path+"/result"

    mutant_path = current_path+"/benchmarks/"

    SygusResultDir = current_path+"/runtime/SygusResult.sl"

    # subprocess.run(["cp", current_path+"/src/runtime/Makefile",current_path+"/runtime/Makefile"])
    if not os.path.exists(sim_target_dir):
        os.makedirs(sim_target_dir)
    if not os.path.exists(smt_target_dir):
        os.makedirs(smt_target_dir)
    if not os.path.exists(runtime_verilog_path):
        os.makedirs(runtime_verilog_path)
    if not os.path.exists(runtime_formal_path):
        os.makedirs(runtime_formal_path)
    if not os.path.exists(sim_build_path):
        os.makedirs(sim_build_path)
    if not os.path.exists(runtime_cocotb_path):
        os.makedirs(runtime_cocotb_path)
    if not os.path.exists(result_dir):
        os.makedirs(result_dir)
    if not os.path.exists(variables_dir):
        os.makedirs(variables_dir)
    if not os.path.exists(mutant_path):
        os.makedirs(mutant_path)
    if not os.path.exists(SygusResultDir):
        open(SygusResultDir, 'a').close()

    subprocess.run(["cp", user_verilog_path+"/sim.py",runtime_cocotb_path+"sim.py"])

if __name__ == "__main__":
    file_name = ""
    current_path = os.getcwd()
    setup(current_path)