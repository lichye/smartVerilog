import time
import sys
import os
import subprocess
import shutil

compiled = False

prep = False

current_path = os.getcwd()

user_config_path = current_path+"/User/config.ini"

user_verilog_path = current_path+"/User"

user_verilog_file = user_verilog_path+"/addsub.sv"

runtime_ebmc_path = current_path+"/runtime/ebmc/"

runtime_verilog_path = current_path+"/runtime/verilog"

runtime_verilog_file = runtime_verilog_path+"/addsub.sv"

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
    subprocess.run(["make", "clean_runtime"])
    compiled = False
    prep = False
    print("Finish Cleaning")

def trace():
    global compiled
    if(not compiled):
        print("Has not compiled, run compile first")
        compile()
    print("Running Sygus File Generation")
    subprocess.run(["./smart.out","--trace"])

def VerilogPrep():
    print("Running Verilog Code Preprocessing")
    result = subprocess.run(["python", "src/VerilogPrep.py", user_verilog_file, runtime_verilog_file],
    capture_output=True,
    text=True)

    print("Finished Verilog Code Preprocessing")

def smart():
    VerilogPrep()
    sim()
    compile()
    subprocess.run(["./smart.out"])
    subprocess.run(["cvc5","--sygus","sygus.sl"])

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
        elif cmd == "smart":
            smart()
        elif cmd == "trace" or cmd == "t":
            trace()
        elif cmd == "prep":
            VerilogPrep()
        elif cmd == "all":
            VerilogPrep()
            compile()
            sim()
        else:
            print("Invalid Command")

def setup():
    if not os.path.exists(current_path+"/runtime"):
        os.makedirs(current_path+"/runtime")
    subprocess.run(["cp", current_path+"/src/runtime/sim.py",current_path+"/runtime/sim.py"])
    subprocess.run(["cp", current_path+"/src/runtime/Makefile",current_path+"/runtime/Makefile"])
    if not os.path.exists(sim_target_dir):
        os.makedirs(sim_target_dir)
    if not os.path.exists(smt_target_dir):
        os.makedirs(smt_target_dir)
    if not os.path.exists(runtime_verilog_path):
        os.makedirs(runtime_verilog_path)
    if not os.path.exists(runtime_ebmc_path):
        os.makedirs(runtime_ebmc_path)
    VerilogPrep()

   
if __name__ == "__main__":
    setup()
    compiled = False
    prep = False
    # compile()
    # modify()
    # sim()
    runner()