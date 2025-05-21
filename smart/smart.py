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

def smart(current_path, top_module,result_file,init_variables,core_id):
    # print("calling : ./smart.out",current_path, top_module, result_file, init_variables,core_id)
    cmd = ["timeout","100","./smart.out",current_path,top_module,result_file,init_variables,core_id]
    # print("Run cmd: ", cmd)
    # result = subprocess.run(cmd, capture_output=True, text=True, cwd=current_path)
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
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

def merge_txt_files(folder_path, output_file='invariants.txt'):
    with open(output_file, 'w', encoding='utf-8') as out_file:
        for filename in os.listdir(folder_path):
            if filename.endswith('.txt'):
                file_path = os.path.join(folder_path, filename)
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    out_file.write(content + '\n')
    print(f"All .txt files in '{folder_path}' merged into '{output_file}'.")


if __name__ == "__main__":
    all_start_time = time.time()
    
    # parameters
    assertion = 10000000
    removeVariables = False

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
    #Pre analysis of the code
    pre_start_time = time.time()
    
    preAnalysis(current_path,mverilog_path+main_file_name,main_module,runtimeVariablesDir)
    
    pre_end_time = time.time()  
    # Smart
    smart_sucess = 0

    assertion_founded = 0

    loop_count = 0
    if(assertion == 0):
        exit(0)
    
    smart_start_time = time.time()
    cnt = 0

    all_work = len(os.listdir(runtimeVariablesDir))
    verified_assertion = []
    result_files = []
    futures = []


    # start new process from the pool
    with ProcessPoolExecutor() as executor:
        # all works
        smart_loop = 0

        # send works to different executors
        for filename in sorted(os.listdir(runtimeVariablesDir)):
            smart_loop += 1
            result_file = os.path.join(resultDir, "result_"+str(smart_loop)+".txt")
            future = executor.submit(smart, current_path, main_module, result_file, os.path.join(runtimeVariablesDir, filename), str(smart_loop))
            futures.append(future)
            result_files.append(result_file)
        
        done_set = set()
        while len(done_set) < len(futures):
            time.sleep(1)
            done_now = [f for f in futures if f.done() and f not in done_set]
            done_set.update(done_now)
            print(f"[{time.strftime('%X')}] Completed {len(done_set)}/{len(futures)} tasks")


    # After all result back, delete the files
    for i, future in enumerate(futures):
        try:
            result = future.result()
            # print("Result: ", result)
            if result != 0:
                file_to_delete = result_files[i]
                if os.path.exists(file_to_delete):
                    os.remove(file_to_delete)
                    print(f"Deleted {file_to_delete} (result={result})")
            # else:
                # print(f"Kept {result_files[i]} (result={result})")
        except Exception as e:
            file_to_delete = result_files[i]
            if os.path.exists(file_to_delete):
                os.remove(file_to_delete)
                print(f"Deleted {file_to_delete} due to exception")

    smart_end_time = time.time()
    smart_time = smart_end_time - smart_start_time

    all_end_time = time.time()
    all_time = all_end_time - all_start_time

    print("Smart Time: ", smart_time)
    print("Overall Time: ", all_time)
    print("all Assertion: ", verified_assertion)
    with open(resultfile,"a") as f:
        f.write("\n")
        f.write("Smart Time: "+str(smart_time)+"\n")
        f.write("Overall Time: "+str(all_time)+"\n")
        f.write("Pre analysis Time: "+str(pre_end_time-pre_start_time)+"\n")
        f.write("We found "+str(len(verified_assertion))+" assertions\n")
    merge_txt_files(resultDir, "assertions.txt")
    print("Finish all the work")
    
    
    
