import time
import sys
import os
import subprocess
import shutil

compiled = False

current_path = os.getcwd()

def compile():
    print("Compiling")
    subprocess.run(["make", "compile"])
    
    print("Finish Compiling")
    compiled = True

def sim():
    print("Verilog Simulating")
    simPython = current_path+"/runtime/sim.py"
    verilogDesign = current_path+"/runtime/verilog/addsub.sv"
    subprocess.run(["python", simPython, verilogDesign])

    sourceVcdFile = current_path+"/sim_build/dump.vcd"
    target_dir = "runtime/sim_results"

    if not os.path.exists(target_dir):
        os.makedirs(target_dir)
    
    file_index = 1

    while True:
        target_file = os.path.join(target_dir, f"sim{file_index}.vcd")
        if not os.path.exists(target_file):
            break
        file_index += 1
    
    shutil.move(sourceVcdFile, target_file)
    
    print("Finish Verilog simulation")

def clean():
    print("Cleaning")
    subprocess.run(["make", "clean"])
    compiled = False

def smart():
    subprocess.run(["./smart.out"])

def runner():
    print("This file path is "+current_path)
    while True:
        cmd = input("")
        if cmd == "exit":
            return
        elif cmd == "compile":
            compile()
        elif cmd == "clear":
            subprocess.run(["clear"])
        elif cmd == "clean":
            clean()
        elif cmd == "sim":
            sim()
        elif cmd == "smart":
            smart()
        else:
            print("Invalid Command")
    
if __name__ == "__main__":
    compile()
    sim()
    runner()