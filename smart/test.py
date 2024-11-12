import time
import sys
import os
import subprocess
import shutil

compiled = False

prep = False

current_path = os.getcwd()

user_config_path = current_path+"/User/config.ini"

user_verilog_path = current_path+"/User/addsub.sv"

runtime_ebmc_path = current_path+"/runtime/ebmc/addsub.sv"

runtime_verilog_path = current_path+"/runtime/verilog/addsub.sv"

sim_target_dir = current_path+"/runtime/sim_results"

smt_target_dir = current_path+"/runtime/smt_results"


def compile():
    global compiled
    print("Compiling")
    subprocess.run(["make", "compile"])
    
    print("Finish Compiling")
    compiled = True

def sim():
    print("Verilog Simulating")
    global prep

    if(not prep):
        print("Has not run Verilog Preprocessing, run VerilogPrep first")
        VerilogPrep()
        prep = True

    simPython = current_path+"/runtime/sim.py"
    verilogDesign = current_path+"/runtime/verilog/addsub.sv"
    subprocess.run(["python", simPython, verilogDesign])

    sourceVcdFile = current_path+"/sim_build/dump.vcd"
    

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
    compiled = False
    prep = False
    print("Finish Cleaning")


def sygusFileGen():
    global compiled
    if(not compiled):
        print("Has not compiled, run compile first")
        compile()
    print("Running Sygus File Generation")
    subprocess.run(["./smart.out","--sygus"])

def unreachableGen():
    global compiled
    global prep
    if(not compiled):
        print("Has not compiled, run compile first")
        compile()
    if(not prep):
        print("Has not run Verilog Preprocessing, run VerilogPrep first")
        VerilogPrep()

    print("Running Unreachable Generation")
    subprocess.run(["./smart.out", "--unreachable",user_config_path, runtime_verilog_path, runtime_ebmc_path])

def VerilogPrep():
    print("Running Verilog Code Preprocessing")
    result = subprocess.run(["python", "src/VerilogPrep.py", user_verilog_path, runtime_verilog_path],
    capture_output=True,
    text=True)

    print("Finished Verilog Code Preprocessing")

def ebmc():
    # call EBMC and get results from CMDLINE?
    print("Running EBMC")
    result = subprocess.run(["ebmc", runtime_ebmc_path,"--bound", "10"],
    capture_output=True,
    text=True)

    if result.returncode == 0:
        print("EBMC ran successfully.")
        print("Output:", result.stdout)
    else:
        print("EBMC encountered an error.")
        print("Error:", result.stderr)

def runner():
    print("This file path is "+current_path)
    while True:
        cmd = input("")
        if cmd == "exit" or cmd == "quit" or cmd == "q" or cmd == "e": 
            return
        elif cmd == "compile":
            compile()
        elif cmd == "clear":
            subprocess.run(["clear"])
        elif cmd == "clean" or cmd == "c":
            clean()
        elif cmd == "sim":
            sim()
        elif cmd == "smt":
            print("Have not implemented the smt command")
        elif cmd == "sygus":
            sygusFileGen()
        elif cmd == "unreachable" or cmd == "u":
            unreachableGen()
        elif cmd == "prep":
            VerilogPrep()
        elif cmd == "ebmc":
            ebmc()
        elif cmd == "all":
            VerilogPrep()
            compile()
            sim()
        else:
            print("Invalid Command")
    
if __name__ == "__main__":
    compiled = False
    prep = False
    # compile()
    # modify()
    # sim()
    runner()