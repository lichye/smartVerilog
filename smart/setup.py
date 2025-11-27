import time
import sys
import os
import subprocess
import shutil
import time
import json

def copy_sv_files(original_path, target_path):
    for root, dirs, files in os.walk(original_path):
        for file in files:
            if file.endswith(".sv") or file.endswith(".svh") or file.endswith(".vh"):
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
    
    if main_module and (main_module.endswith("_a") or main_module.endswith("_assume")):
        if main_module.endswith("_a"):
            base_module = main_module[:-2]
        if main_module.endswith("_assume"):
            base_module = main_module[:-7]
        with open(target_file,"r") as f:
            content = f.read()
        new_content = content.replace(base_module, main_module)
        with open(target_file,"w") as f:
            f.write(new_content)
        f.close()
    print("Finish Verilog simulation")

def sv_prep(sv_path,rst_path):
    sv_file_names = []
    sv_file_paths = []
    
    for root, dirs, files in os.walk(sv_path):
        for file in files:
            if file.endswith(".sv") or file.endswith(".svh") or file.endswith(".vh"):
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

def setupDir(current_path):
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
    
    dir_path = os.path.dirname(os.path.realpath(__file__))
    
    # Check if the User give the correct input
    if(len(sys.argv)==2):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        parent_dir = os.path.dirname(current_path)
        Config = parent_dir+"/Config/basic.json"

    elif(len(sys.argv)==3):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        Config = sys.argv[2]
    
    else:
        print("Should give the main module name")
        exit(1)
    
    with open(Config) as f:
        config_data = json.load(f)
        Number_Trace = config_data["PreAnalysis_settings"]["Number_Trace"]
        sim_loop = Number_Trace

    files = os.listdir(dir_path)
    if len(files) == 0:
        print("No files in User directory")
        print("Stop running")
        exit(1)

    #Start the preporcessing
    setupDir(current_path) # this will setup the runtime directory structure
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

        
    
