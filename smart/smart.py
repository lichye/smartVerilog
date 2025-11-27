import time
import os
import sys
import json
import subprocess
import shutil
import time
import glob
import math
import subprocess
import threading
import random
from concurrent.futures import ProcessPoolExecutor

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "src", "python")))
from minimal_satisfiable_assignment import get_mus
from minimise_assertions import run_minimisation

def smart(current_path, top_module,result_file,init_variables,core_id,latency):
    # print("calling : ./smart.out",current_path, top_module, result_file, init_variables,core_id)
    cmd = ["timeout","100","./smart.out",current_path,top_module,result_file,init_variables,core_id,latency,Config]
    # print("Run cmd: ", str(cmd))
    # result = subprocess.run(cmd, capture_output=True, text=True, cwd=current_path)
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # print(" ".join(cmd))
    writeLog("smartCore.txt", " ".join(cmd)+"\n\n")
    # writeLog("smartCore.txt", " ".str(result)+ "\n")
    # print("Result: ", result)
    return result.returncode

def preAnalysis(work_dir,file_path,top_module,output_file,Config):
    cmd = ["python", "src/python/preAnalyzer.py",work_dir,file_path, top_module, output_file,Config]
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
    # Read Config
    cpu_cores = os.cpu_count()
    workers = min(max_threads, all_work, cpu_cores) # do not exceed the number of works
    
    
    with ProcessPoolExecutor(max_workers=workers) as executor:
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
    # subprocess.run("rm -rf ./runtime/variables/*", shell=True)
    print("Finish running Smart Block, found "+str(assertion_founded)+" new assertions")
    return assertion_founded
    
def GenerateNewBlocks():
    # clean the "runtime/variables/" folder
    shutil.rmtree(runtimeVariablesDir, ignore_errors=True)
    os.makedirs(runtimeVariablesDir, exist_ok=True)
    if(Blockified_settings["MSA"]== True and Blockified_settings["Random"]== False):
        underspecified,model = get_mus("runtime/variables.txt", "runtime/SygusResult.sl", timeout=300)
        print(f"Underspecified variables size: {len(underspecified)}")
        underspecified = list(underspecified)

        num_cores = os.cpu_count()

        V = len(underspecified)
        k = round(2.7+k_size*math.log(V,10))
        n = max(int(Block_size*V**0.9), num_cores)

        for i in range(int(n)):
            variable_set = random.sample(underspecified, k)
        #    print(f"variable_set_{i}: {variable_set}")
            with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
                f.write('\n'.join(variable_set))
   
    if(Blockified_settings["Random"]== True and Blockified_settings["MSA"]== False):
        with open("runtime/variables.txt") as f:
            variables = f.read().splitlines()
        variables = list(set(variables))
        V = len(variables)
        k = round(2.7+k_size*math.log(V,10))
        n = max(int(Block_size*V**0.9), os.cpu_count())
        print("Underspecified variable size is "+str(V))
        print("Subset size k is "+str(k))
        print("Number of threads n is "+str(n))

        underspecified = list(variables)

        for i in range(int(n)):
            variable_set = random.sample(underspecified, k)
        #    print(f"variable_set_{i}: {variable_set}")
            with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
                f.write('\n'.join(variable_set))

    if(Blockified_settings["Random"]== True and Blockified_settings["MSA"]== True):
        # Do 50% MSA
        underspecified,model = get_mus("runtime/variables.txt", "runtime/SygusResult.sl", timeout=300)
        print(f"Underspecified variables size from MSA: {len(underspecified)}")
        underspecified = list(underspecified)
        V = len(underspecified)
        k = round(2.7+k_size*math.log(V,10))
        n = max(int(Block_size*V**0.9), os.cpu_count())
        msa_n = int(n/2)
        for i in range(int(msa_n)):
            variable_set = random.sample(underspecified, k)
            with open(f"runtime/variables/thread_{i}.txt", 'w') as f:
                f.write('\n'.join(variable_set))
      
      # Do 50% Random
        with open("runtime/variables.txt") as f:
            variables = f.read().splitlines()
        variables = list(set(variables))
        V = len(variables)
        k = round(2.7+k_size*math.log(V,10))
        random_n = n - msa_n
        print("Underspecified variable size from Random is "+str(V))
        print("Subset size k is "+str(k))
        print("Number of threads n is "+str(random_n))
        underspecified = list(variables)
        for i in range(int(random_n)):
            variable_set = random.sample(underspecified, k)
            with open(f"runtime/variables/thread_{i+msa_n}.txt", 'w') as f:
                f.write('\n'.join(variable_set))
    
    return len(underspecified)

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
        parent_dir = os.path.dirname(current_path)
        Config = parent_dir+"/Config/basic.json"
    elif(len(sys.argv)==3):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        Config = sys.argv[2]
    elif(len(sys.argv)==4):
        main_module = sys.argv[1]
        main_file_name = main_module+".sv"
        Config = sys.argv[2]
        latency = int(sys.argv[3])
    else:
        print("Should give the main module name")
        exit(1)

    files = os.listdir(dir_path)
    if len(files) == 0:
        print("No files in User directory")
        print("Stop running")
        exit(1)

    with open(Config) as f:
        config = json.load(f)
        Workflow = config.get("Workflow")
        Blockified_settings = config.get("Blockified_settings")
        Threadhold = Blockified_settings.get("Threadhold")
        Parallel_settings = config.get("Parallel_settings")
        max_threads = Parallel_settings.get("max_threads")
        Minizer_settings = config.get("Minimizer_settings")
        Block_minimizer = Minizer_settings.get("Block_minimizer")
        MSA_stable_end = Blockified_settings.get("MSA_stable_end")
        Log = Workflow.get("Log")
        Block_Minimizer_timeout = Minizer_settings.get("Block_Minimizer_timeout")
        Save_temp_assertions = Blockified_settings.get("Save_temp_assertions")
        Block_size = Blockified_settings.get("Block_size",0.5)
        k_size = Blockified_settings.get("k_size",1.6)

    resultfile = current_path+"/result_"+main_module+".txt"
    detail_logfile = current_path+"/log_"+main_module+".txt"
    
    # Pre analysis of the code
    pre_start_time = time.time()
    preAnalysis(current_path,mverilog_path+main_file_name,main_module,runtimeVariablesDir,Config)
    pre_end_time = time.time()  
    
    # SMART CORE EXECUTION
    smart_start_time = time.time()
    total_smart_time = 0
    total_msa_time = 0
    total_minizer_time = 0
    
    # Run one iteration of Smart core
    new_result = runBlockSmart()

    smart_end_time = time.time()
    total_smart_time = smart_end_time - smart_start_time + total_smart_time
    if Log == True:
        with open(detail_logfile,"a") as f:
            f.write("Block 1:\n")
            f.write("\tFound "+str(new_result)+" assertions\n")
            f.write("\tSmart time taken: "+str(smart_end_time-smart_start_time)+" seconds\n")
    
    if Workflow["Blockified"] == False:
        print("Non-blockified mode, stop after first iteration")
        new_result = 0
    
    last_size = math.inf
    loop_count = 1

    while new_result > Threadhold:
        if Save_temp_assertions == True:
            # Save the found assertions to a temp file
            temp_assertion_file = current_path+"/temp_assertions_block_"+str(loop_count)+".txt"
            with open(temp_assertion_file,"w") as f:
                for assertion in verified_assertion:
                    f.write(assertion+"\n")
        
        print("Generate New SMART blocks based on new found assertions")
        if Block_minimizer == True:
            # Minimize the found assertions to reduce the variable set
            min_start_time = time.time()
            run_minimisation("runtime/SygusResult.sl","runtime/SygusResult.sl", Block_Minimizer_timeout)
            min_end_time = time.time()
            total_minizer_time = total_minizer_time + (min_end_time - min_start_time)
            if Log == True:
                with open(detail_logfile,"a") as f:
                    f.write("\tMinimizer time taken: "+str(min_end_time - min_start_time)+" seconds\n")
        

        msa_start_time = time.time()
        msa_size = GenerateNewBlocks()
        msa_end_time = time.time()

        total_msa_time = total_msa_time + (msa_end_time - msa_start_time)
        if Log == True:
            with open(detail_logfile,"a") as f:
                f.write("\tMSA time taken: "+str(msa_end_time - msa_start_time)+" seconds\n")
                f.write("\tMSA set size: "+str(msa_size)+"\n\n")
                f.write("--------------------------------\n")
        
        if MSA_stable_end == True and msa_size >= last_size:
            break
        else:
            last_size = msa_size
        
        smart_start_time = time.time()
        new_result = runBlockSmart()
        smart_end_time = time.time()
        total_smart_time = smart_end_time - smart_start_time + total_smart_time
        if Log == True:
            with open(detail_logfile,"a") as f:
                f.write("Block "+str(loop_count+1)+":\n")
                f.write("\tFound "+str(new_result)+" assertions\n")
                f.write("\tSmart time taken: "+str(smart_end_time-smart_start_time)+" seconds\n")
        loop_count = loop_count + 1

    # Count the time

    all_end_time = time.time()
    all_time = all_end_time - all_start_time

    print("Total runtime: "+str(all_time)+" seconds")
    print("Pre analysis Time: "+str(pre_end_time-pre_start_time)+" seconds")
    print("Total Smart Time: "+str(total_smart_time)+" seconds")
    print("Total MSA Time: "+str(total_msa_time)+" seconds")
    print("Total Minimizer Time: "+str(total_minizer_time)+" seconds")
    
    if Log == True:
        with open(resultfile,"a") as f:
            f.write("\n")
            f.write("Total runtime: "+str(all_time)+"\n")
            f.write("Pre analysis Time: "+str(pre_end_time-pre_start_time)+"\n")
            f.write("Total Smart Time: "+str(total_smart_time)+"\n")
            f.write("Total MSA Time: "+str(total_msa_time)+"\n")
            f.write("Total Minimizer Time: "+str(total_minizer_time)+"\n")
            f.write("We found "+str(len(verified_assertion))+" assertions\n\n")

    with open("assertions.txt","a") as f:
        for assertion in verified_assertion:
            f.write(assertion+"\n")
    print("Finish running smart core")
    print("-------------------------------")
    
    
    
