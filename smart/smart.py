import time
import sys
import os
import subprocess
import shutil
import time
import glob
import subprocess
import threading
from concurrent.futures import ProcessPoolExecutor

def copy_sv_files(original_path, target_path):
    for root, dirs, files in os.walk(original_path):
        for file in files:
            if file.endswith(".sv"):
                shutil.copy(os.path.join(root, file), target_path)
            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)

def smart(current_path, top_module,result_file,init_variables,core_id,latency):
    # print("calling : ./smart.out",current_path, top_module, result_file, init_variables,core_id)
    cmd = ["timeout","100","./smart.out",current_path,top_module,result_file,init_variables,core_id,latency]
    # print("Run cmd: ", str(cmd))
    # result = subprocess.run(cmd, capture_output=True, text=True, cwd=current_path)
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # print(" ".join(cmd))
    writeLog("smartCore.txt", " ".join(cmd)+"\n\n")
    # writeLog("smartCore.txt", " ".str(result)+ "\n")
    # print("Result: ", result)
    return result.returncode

def preAnalysis(work_dir,file_path,top_module,output_file):
    cmd = ["python", "src/python/preAnalyzer.py",work_dir,file_path, top_module, output_file]
    print("Run cmd: ", cmd)
    subprocess.run(cmd)
    return 0

def writeLog(file,content):
    with open(file, "a") as f:
        f.write(content)
    f.close()

def runBlockSmart():
    cnt = 0
    all_work = len(os.listdir(runtimeVariablesDir))
    
    result_files = []
    futures = []

    print("-------------------------------")
    print("Start running Smart Block of "+str(all_work)+" threads")

    assertion_founded = 0
     # start new process from the pool
    with ProcessPoolExecutor() as executor:
        # all works
        smart_loop = 0

        # send works to different executors
        for filename in sorted(os.listdir(runtimeVariablesDir)):
            smart_loop += 1
            result_file = os.path.join(resultDir, "result_"+str(smart_loop)+".txt")
            future = executor.submit(smart, current_path, main_module, result_file, os.path.join(runtimeVariablesDir, filename), str(smart_loop), str(latency))
            futures.append(future)
            result_files.append(result_file)
        
        done_set = set()
        while len(done_set) < len(futures):
            time.sleep(10)
            done_now = [f for f in futures if f.done() and f not in done_set]
            done_set.update(done_now)
            print(f"[{time.strftime('%X')}] Completed {len(done_set)}/{len(futures)} Smart Cores")


    # After all result back, delete the files
    for i, future in enumerate(futures):
        try:
            result = future.result()
            # print("Result: ", result)
            if result != 0:
                file_to_delete = result_files[i]
                if os.path.exists(file_to_delete):
                    os.remove(file_to_delete)
                    # print(f"Deleted {file_to_delete} (result={result})")
            else:
                with open(result_files[i], "r") as f:
                    new_assertion = f.read().strip()

                if new_assertion not in verified_assertion: 
                    verified_assertion.add(new_assertion)
                    assertion_founded += 1
                # print(f"Kept {result_files[i]} (result={result})")
        except Exception as e:
            # print(f"Exception occurred: {e}")
            file_to_delete = result_files[i]
            if os.path.exists(file_to_delete):
                os.remove(file_to_delete)
                # print(f"Deleted {file_to_delete} due to exception")
    subprocess.run("rm -rf ./runtime/variables/*", shell=True)
    print("Finish running Smart Block, found "+str(assertion_founded)+" new assertions")
    
def GenerateNewBlocks():
    cmd = ["python", "src/python/generate_variable_subsets.py"]
    subprocess.run(cmd)
    return

if __name__ == "__main__":
    # Count the overall time
    all_start_time = time.time()
    latency = 0

    # Result Stored here
    verified_assertion = set()
    
    # Set up the file path
    current_path = os.getcwd()
    mverilog_path = current_path+"/runtime/verilog/"
    resultDir = current_path+"/result"
    runtimeVariablesDir = current_path+"/runtime/variables/"
    runtimeFormalDir = current_path+"/runtime/formal/"
    runtimeVerilogDir = current_path+"/runtime/verilog/"
    runtimeSmtDir = current_path+"/runtime/smt_results/"
    initVariables = current_path+"/runtime/variables/initVariables.txt"
    dir_path = os.path.dirname(os.path.realpath(__file__))
    

    # Check if the User give the correct input
    if(len(sys.argv)==2):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        latency = 0
    elif(len(sys.argv)==3):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        latency = int(sys.argv[2])
    else:
        print("Should give the main module name")
        exit(1)

    files = os.listdir(dir_path)
    if len(files) == 0:
        print("No files in User directory")
        print("Stop running")
        exit(1)


    logfile = current_path+"/log_"+main_module+".txt"
    resultfile = current_path+"/result_"+main_module+".txt"
    copy_sv_files(mverilog_path, runtimeFormalDir)
    
    # Pre analysis of the code
    pre_start_time = time.time()
    preAnalysis(current_path,mverilog_path+main_file_name,main_module,runtimeVariablesDir)
    pre_end_time = time.time()  
    
    # SMART CORE EXECUTION
    smart_start_time = time.time()

    runBlockSmart()
    GenerateNewBlocks()
    runBlockSmart()


    # Count the time
    smart_end_time = time.time()
    smart_time = smart_end_time - smart_start_time
    all_end_time = time.time()
    all_time = all_end_time - all_start_time

    print("Smart Time: ", smart_time)
    print("Overall Time: ", all_time)
    print("all Assertion: ", len(verified_assertion))
    with open(resultfile,"a") as f:
        f.write("\n")
        f.write("Smart Time: "+str(smart_time)+"\n")
        f.write("Overall Time: "+str(all_time)+"\n")
        f.write("Pre analysis Time: "+str(pre_end_time-pre_start_time)+"\n")
        f.write("We found "+str(len(verified_assertion))+" assertions\n")
    with open("assertions.txt","a") as f:
        for assertion in verified_assertion:
            f.write(assertion+"\n")
    print("Finish running smart core")
    print("-------------------------------")
    
    
    
