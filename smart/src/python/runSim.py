import os
import subprocess
import shutil

def sim(current_path):
    print("Verilog Simulating")
    
    simPython = current_path+"/runtime/cocotb/sim.py"

    subprocess.run(["python", simPython])

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


if __name__ == "__main__":
    current_path = os.getcwd()
    sim(current_path)