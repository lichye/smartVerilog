import os
import subprocess

## This function is used to setup the runtime directory
def setup(current_path):
    if not os.path.exists(current_path+"/runtime"):
        os.makedirs(current_path+"/runtime")

    runtime_ebmc_path = current_path+"/runtime/ebmc/"

    runtime_verilog_path = current_path+"/runtime/verilog/"

    runtime_cocotb_path = current_path+"/runtime/cocotb/"

    sim_build_path = current_path+"/sim_build"

    user_verilog_path = current_path+"/User"
    
    sim_target_dir = current_path+"/runtime/sim_results"

    smt_target_dir = current_path+"/runtime/smt_results"

    # subprocess.run(["cp", current_path+"/src/runtime/Makefile",current_path+"/runtime/Makefile"])
    if not os.path.exists(sim_target_dir):
        os.makedirs(sim_target_dir)
    if not os.path.exists(smt_target_dir):
        os.makedirs(smt_target_dir)
    if not os.path.exists(runtime_verilog_path):
        os.makedirs(runtime_verilog_path)
    if not os.path.exists(runtime_ebmc_path):
        os.makedirs(runtime_ebmc_path)
    if not os.path.exists(sim_build_path):
        os.makedirs(sim_build_path)
    if not os.path.exists(runtime_cocotb_path):
        os.makedirs(runtime_cocotb_path)

    subprocess.run(["cp", user_verilog_path+"/sim.py",runtime_cocotb_path+"sim.py"])

def get_sv_file_paths(directory):
    sv_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".sv"):
                sv_files.append(os.path.join(root, file))
            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)
    return sv_files

def move_User_verilog(current_path):
    runtime_ebmc_path = current_path+"/runtime/ebmc/"
    runtime_verilog_path = current_path+"/runtime/verilog/"
    user_verilog_path = current_path+"/User"
    sim_build_path = current_path+"/sim_build"

    #get all the sv files in the user directory
    sv_files = get_sv_file_paths(user_verilog_path)

    for file in sv_files:
        subprocess.run(["cp", file, runtime_verilog_path])
        subprocess.run(["cp", file, runtime_ebmc_path])
        subprocess.run(["cp", file, sim_build_path])

if __name__ == "__main__":
    file_name = ""
    current_path = os.getcwd()
    setup(current_path)
    move_User_verilog(current_path)