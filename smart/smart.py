import time
import sys
import os
import subprocess
import shutil

def copy_sv_files(original_path, target_path):
    for root, dirs, files in os.walk(original_path):
        for file in files:
            if file.endswith(".sv"):
                shutil.copy(os.path.join(root, file), target_path)
            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)

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

def sv_prep(sv_path,rst_path):
    sv_file_names = []
    sv_file_paths = []
    
    for root, dirs, files in os.walk(sv_path):
        for file in files:
            if file.endswith(".sv"):
                sv_file_names.append(file)
                sv_file_paths.append(os.path.join(root, file))
                
                sv_file_prep_path = rst_path + file
                cmd = ["python", "src/python/Prep.py", os.path.join(root, file), sv_file_prep_path]
                print("Run cmd: ", cmd)
                subprocess.run(cmd)
                # subprocess.run(["python," "src/python/prep.py", os.path.join(root, file), sv_file_prep_path])

            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)

def smart(current_path, file_name):
    src_file = current_path+"/runtime/verilog/"+file_name
    module_name = file_name.split(".")[0]
    print("module name: ", module_name)
    subprocess.run(["./smart.out",src_file,module_name])

if __name__ == "__main__":

    current_path = os.getcwd()
    user_path = current_path+"/User"
    cocotb_path = current_path+"/runtime/cocotb/"
    ebmc_path = current_path+"/runtime/ebmc/"
    mverilog_path = current_path+"/runtime/verilog/"

    dir_path = os.path.dirname(os.path.realpath(__file__))
    
    # Check if the User give the correct input
    if(len(sys.argv)==2):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
    else:
        print("Should give the main module name")
        exit(1)

    files = os.listdir(dir_path)
    if len(files) == 0:
        print("No files in User directory")
        print("Stop running")
        exit(1)

    #Start the preporcessing
    subprocess.run(["make", "setup"]) # this will setup the directories and files needed for the smart compiler
    sv_prep(user_path,mverilog_path) # this will prepare the system verilog files for cocotb
    print("Finish Initial Setup")
    
    #before the simulation copy all the files from cocotb to runtime/ebmc
    copy_sv_files(mverilog_path, cocotb_path)
    copy_sv_files(mverilog_path, ebmc_path)

    #Start the simulation
    print("Start Simulation")
    sim(current_path, main_file_name) # this will run the simulation

    #Start compile the smart compiler
    subprocess.run(["make", "compile"]) # this will compile the smart compiler

    #Start to run smart main module
    main_module_path = mverilog_path+main_file_name
   
    subprocess.run(["./smart.out",main_module_path,main_module])