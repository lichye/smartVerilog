import time
import sys
import os
import subprocess
import shutil

def sim(current_path,file_name):
    print("Verilog Simulating")
    
    simPython = current_path+"/runtime/cocotb/sim.py"

    verilogDesign = current_path+"/runtime/verilog/"+file_name
    
    result = subprocess.run(["python", simPython])

    if result.returncode != 0:
        print("Error in simulation")
        exit(1)

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

def smart(current_path, file_name):
    src_file = current_path+"/runtime/verilog/"+file_name
    module_name = file_name.split(".")[0]
    print("module name: ", module_name)
    subprocess.run(["./smart.out",src_file,module_name])

def has_files(dir_path):
    files = os.listdir(dir_path)
    if len(files) == 0:
        return False
    return True

def sv_prep(sv_path):
    sv_file_names = []
    sv_file_paths = []
    
    for root, dirs, files in os.walk(sv_path):
        for file in files:
            if file.endswith(".sv"):
                sv_file_names.append(file)
                sv_file_paths.append(os.path.join(root, file))
                
                sv_file_prep_path = "runtime/cocotb/"+file
                cmd = ["python", "src/python/Prep.py", os.path.join(root, file), sv_file_prep_path]
                print("Run cmd: ", cmd)
                subprocess.run(cmd)
                # subprocess.run(["python," "src/python/prep.py", os.path.join(root, file), sv_file_prep_path])

            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)

if __name__ == "__main__":
    
    if(len(sys.argv)==2):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
    else:
        main_module = input("Should give the main module name")



    current_path = os.getcwd()
    
    user_path = current_path+"/User"
    
    runtime_verilog_path = current_path+"/runtime/verilog"


    # Check if there is a file in the User directory
    if not has_files(user_path):
        print("No files in User directory")
        print("Stop running")
        exit(1)

    # If there is a file in the User directory
    # Run the smart compiler and setup
    subprocess.run(["make", "setup"]) # this will setup the directories and files needed for the smart compiler
    

    sv_prep(user_path) # this will prepare the system verilog files for cocotb

    print("Finish Initial Setup")
    
    print("Start Simulation")
    sim(current_path, main_file_name) # this will run the simulation


    # subprocess.run(["make", "compile"]) # this will compile the smart compiler