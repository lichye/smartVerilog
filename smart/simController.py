import time
import sys
import os
import subprocess
import shutil

def compile():
    global compiled
    print("Compiling")
    subprocess.run(["make", "compile"])
    
    print("Finish Compiling")
    compiled = True

def sim(current_path,file_name):
    print("Verilog Simulating")
    
    simPython = current_path+"/runtime/sim.py"

    verilogDesign = current_path+"/runtime/verilog/"+file_name

    subprocess.run(["python", simPython, verilogDesign])

    sourceVcdFile = current_path+"/sim_build/dump.vcd"
    
    sim_target_dir = current_path+"/runtime/sim_results"

    if not os.path.exists(sim_target_dir):
        os.makedirs(sim_target_dir)
    
    file_index = 1

    while True:
        target_file = os.path.join(sim_target_dir, f"sim{file_index}.vcd")
        if not os.path.exists(target_file):
            break
        file_index += 1
    
    shutil.move(sourceVcdFile, target_file)
    
    print("Finish Verilog simulation")

def clean():
    global compiled
    global prep
    print("Cleaning")
    subprocess.run(["make", "clean"])
    subprocess.run(["make", "clean_runtime"])
    compiled = False
    prep = False
    print("Finish Cleaning")

def VerilogPrep(current_path, file_name):
    user_verilog_file = current_path+"/User/"+file_name
    runtime_verilog_file = current_path+"/runtime/verilog/"+file_name
    print("Running Verilog Code Preprocessing")
    print("Run command: python src/Analyzer.py", user_verilog_file, runtime_verilog_file)
    result = subprocess.run(["python", "src/Analyzer.py", user_verilog_file, runtime_verilog_file],
    capture_output=True,
    text=True)

    if(result.returncode != 0):
        print("Error in Verilog Code Preprocessing")
        print(result.stderr)
        exit(1)
    
    print("Finished Verilog Code Preprocessing")

def smart():
    subprocess.run(["./smart.out"])

def setup(current_path):
    if not os.path.exists(current_path+"/runtime"):
        os.makedirs(current_path+"/runtime")

    runtime_ebmc_path = current_path+"/runtime/ebmc/"

    runtime_verilog_path = current_path+"/runtime/verilog/"

    user_verilog_path = current_path+"/User"
    
    sim_target_dir = current_path+"/runtime/sim_results"

    smt_target_dir = current_path+"/runtime/smt_results"

    subprocess.run(["cp", user_verilog_path+"/sim.py",current_path+"/runtime/sim.py"])
    # subprocess.run(["cp", current_path+"/src/runtime/Makefile",current_path+"/runtime/Makefile"])
    if not os.path.exists(sim_target_dir):
        os.makedirs(sim_target_dir)
    if not os.path.exists(smt_target_dir):
        os.makedirs(smt_target_dir)
    if not os.path.exists(runtime_verilog_path):
        os.makedirs(runtime_verilog_path)
    if not os.path.exists(runtime_ebmc_path):
        os.makedirs(runtime_ebmc_path)
    
   
if __name__ == "__main__":
    file_name = ""

    current_path = os.getcwd()
    
    if(len(sys.argv) !=2):
        print("Should give verilog design file name")
        exit(1)
    else:
        file_name = sys.argv[1]
    setup(current_path)
    VerilogPrep(current_path, file_name)
    sim(current_path,file_name)