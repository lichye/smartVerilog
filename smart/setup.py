import time
import sys
import os
import subprocess
import shutil
import time

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
        exit(-1)
        return

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

def preAnalysis(work_dir,file_path,top_module,output_file):
    cmd = ["python", "src/python/preAnalyzer.py",work_dir,file_path, top_module, output_file]
    print("Run cmd: ", cmd)
    subprocess.run(cmd)
    return 0

def writeLog(file,content):
    with open(file, "a") as f:
        f.write(content)
    f.close()

if __name__ == "__main__":
    # Set the parameters
    sim_loop = 3

    compile_cmd = 1    

    mutant_cmd = 1

    all_start_time = time.time()

    current_path = os.getcwd()
    
    user_path = current_path+"/user"
    
    cocotb_path = current_path+"/runtime/cocotb/"
    
    formal_path = current_path+"/runtime/formal/"
    
    mutant_path = current_path+"/benchmarks/"

    mverilog_path = current_path+"/runtime/verilog/"

    setup_path = current_path+"/src/python/setUp.py"
    
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
    subprocess.run(["python",setup_path]) # this will setup the directories and files needed for the smart compiler
    sv_prep(user_path,mverilog_path) # this will prepare the system verilog files for cocotb
    print("Finish Initial Setup")
    
    #before the simulation copy all the files from cocotb to runtime/formal
    copy_sv_files(mverilog_path, cocotb_path)
    copy_sv_files(mverilog_path, formal_path)

    #Start the simulation
    print("Start Simulation")
    
    sim_start_time = time.time()

    sim_target_dir = current_path+"/runtime/sim_results"

    if(len(os.listdir(sim_target_dir))==0):
        for i in range(sim_loop):
            sim(current_path, main_file_name) # this will run the simulation
    else:
        print("Simulation already done")
    
    sim_end_time = time.time()

    sim_time = sim_end_time - sim_start_time
    
    compile_start_time = time.time()
    
    # Start compile the smart compiler
    if(compile_cmd):
        ret = subprocess.run(["make", "compile"]) # this will compile the smart compiler
        if ret.returncode != 0:
            print("Error in compiling")
            exit(-1)

    compile_end_time = time.time()
    
    compile_time = compile_end_time - compile_start_time 

    #Setup the mutants
    # setupMutants(mverilog_path,main_file_name,mutant_path,main_module)

    all_end_time = time.time()
    print("Simulation time: ", sim_time)
    print("Compile time: ", compile_time)
    print("Setup time: ", all_end_time - all_start_time)

    logfile = current_path+"/log_"+main_module+".txt"
    writeLog(logfile, "Simulation time: "+str(sim_time)+"\n")
    writeLog(logfile, "Compile time: "+str(compile_time)+"\n")
    writeLog(logfile, "Setup time: "+str(all_end_time - all_start_time)+"\n")
    writeLog(logfile, "--------------------------------------\n")

        
    
