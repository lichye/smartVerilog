import time
import sys
import os
import subprocess
import shutil
import time
import glob

def copy_sv_files(original_path, target_path):
    for root, dirs, files in os.walk(original_path):
        for file in files:
            if file.endswith(".sv"):
                shutil.copy(os.path.join(root, file), target_path)
            elif file.endswith(".v"):
                print("Please convert the verilog files to system verilog")
                exit(1)

def smart(current_path, top_module,result_file,init_variables):
    cmd = ["timeout","100","./smart.out",current_path,top_module,result_file,init_variables]
    # print("Run cmd: ", cmd)
    # result = subprocess.run(cmd)
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return result.returncode

def resultAnalysis(resultDir,runtimeRemoveVariables):
    print("Result Analysis")
    cmd = ["python", "src/python/resultAnalyzer.py", resultDir, runtimeRemoveVariables]
    print("Run cmd: ", cmd)
    subprocess.run(cmd)

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
    runtimeRemoveVariables = current_path+"/runtime/variables/removeVariables.txt"
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
    
    verified_assertion = []

    cnt = 0
    all_work = len(os.listdir(runtimeVariablesDir))
    # Each init variable file is a different smart loop
    for filename in sorted(os.listdir(runtimeVariablesDir)):
        now_time = time.time()
        all_runtime = now_time - all_start_time

        if(all_runtime > 24*3600):
            print("Time out")
            break

        time_start = time.time()
        #clean the removeVariables pipe
        subprocess.run(["rm","-rf",runtimeRemoveVariables])
        subprocess.run(["touch",runtimeRemoveVariables])

        # /home/magna/smartVerilog/smart/runtime/smt_results
        smt_results = os.path.join(os.getcwd(), "runtime/smt_results", "*.vcd")
        for file in glob.glob(smt_results):
            os.remove(file)
        
        writeLog(logfile,"Run with variables: "+filename+"\n")
        writeLog(logfile,"Run Smart Loop "+str(loop_count)+"\n")
        loop_count += 1

        result_file = resultDir+"/result"+str(assertion_founded)+".txt"
        init_variables = os.path.join(runtimeVariablesDir, filename)
        result = smart(current_path, main_module,result_file,init_variables)

        if result == 0:
            writeLog(logfile, "Smart Loop " + str(loop_count)+" is successful\n\n")
            smart_sucess += 1
            with open(result_file, "r") as f:
                new_assertion = f.read()

            if new_assertion not in verified_assertion:
                verified_assertion.append(new_assertion)
                assertion_founded += 1
        
            if assertion_founded == assertion:
                writeLog(logfile,"Finish all the assertion\n")
                break
        else:
            writeLog(logfile,"Smart Loop " + str(loop_count)+" is failure\n\n")
        
        if removeVariables and result == 0:
            result_file = resultDir+"/result"+str(assertion_founded)+".txt"
            resultAnalysis(resultDir,runtimeRemoveVariables)
            result = smart(current_path, main_module,result_file,init_variables)

            if result == 0:
                writeLog(logfile, "Remover Smart Loop is successful\n\n")
                smart_sucess += 1
                with open(result_file, "r") as f:
                    new_assertion = f.read()
                
                if new_assertion not in verified_assertion:
                    verified_assertion.append(new_assertion)
                    assertion_founded += 1
            
                if assertion_founded == assertion:
                    writeLog(logfile,"Finish all the assertion\n")
                    break
            else:
                writeLog(logfile,"Remover Smart Loop  is failure\n\n")

        
        time_end = time.time()
        cnt += 1
        print("Finish "+str(cnt)+"/"+str(all_work)+" smart run in "+str(time_end-time_start)+" seconds")
        print("The result is "+str(result))


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
        for assertion in verified_assertion:
            f.write(str(assertion)+"\n")
        f.write("\n")
    merge_txt_files(resultDir, "invariants.txt")
    print("Finish all the work")
    
    
    
