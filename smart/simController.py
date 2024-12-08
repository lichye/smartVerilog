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

def smart(current_path, file_name):
    src_file = current_path+"/runtime/verilog/"+file_name
    module_name = file_name.split(".")[0]
    print("module name: ", module_name)
    subprocess.run(["./smart.out",src_file,module_name])
    
def runner(current_path, file_name):
    while True:
        cmd = input("Enter Command: ")
        if cmd == "compile":
            compile()
        elif cmd == "sim":
            sim(current_path,file_name)
        elif cmd == "clean":
            clean()
        elif cmd == "exit" or cmd == "quit" or cmd == "q" or cmd == "e": 
            break
        elif cmd == "smart" or cmd == "s":
            smart(current_path, file_name)
        elif cmd == "prep":
            VerilogPrep(current_path, file_name)
        else:
            print("Invalid Command")

if __name__ == "__main__":
    file_name = ""
    current_path = os.getcwd()
    
    print("finish Verilog Prep and Setup")
    runner(current_path, file_name)