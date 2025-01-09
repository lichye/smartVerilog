import time
import sys
import os
import subprocess
import shutil
import time

def smart(current_path, top_module,result_file,init_variables):
    cmd = ["./smart.out",current_path,top_module,result_file,init_variables]
    # print("Run cmd: ", cmd)
    result = subprocess.run(cmd)
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

if __name__ == "__main__":
    all_start_time = time.time()

    # parameters
    assertion = 5 
    removeVariables = True

    # Set up the file path
    current_path = os.getcwd()
    mverilog_path = current_path+"/runtime/verilog/"
    resultDir = current_path+"/result"
    runtimeVariablesDir = current_path+"/runtime/variables/"
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

    #Pre analysis of the code
    preAnalysis(current_path,mverilog_path+main_file_name,main_module,runtimeVariablesDir)
    
    # Smart
    smart_sucess = 0

    assertion_founded = 0

    loop_count = 0
    if(assertion == 0):
        exit(0)
    
    smart_start_time = time.time()
    
    verified_assertion = []

    # Each init variable file is a different smart loop
    for filename in sorted(os.listdir(runtimeVariablesDir)):
        #clean the removeVariables pipe
        subprocess.run(["rm","-rf",runtimeRemoveVariables])
        subprocess.run(["touch",runtimeRemoveVariables])

        writeLog("log.txt","Run with variables: "+filename+"\n")
        writeLog("log.txt","Run Smart Loop "+str(loop_count)+"\n")
        loop_count += 1

        result_file = resultDir+"/result"+str(assertion_founded)+".txt"
        init_variables = os.path.join(runtimeVariablesDir, filename)
        result = smart(current_path, main_module,result_file,init_variables)

        subprocess.run(["rm","-rf","runtime/smt_results/*"])

        if result == 0:
            writeLog("log.txt", "Smart Loop " + str(loop_count)+" is successful\n\n")
            smart_sucess += 1
            with open(result_file, "r") as f:
                new_assertion = f.read()

            if new_assertion not in verified_assertion:
                verified_assertion.append(new_assertion)
                assertion_founded += 1
        
            if assertion_founded == assertion:
                writeLog("log.txt","Finish all the assertion\n")
                break
        else:
            writeLog("log.txt","Smart Loop " + str(loop_count)+" is failure\n\n")
            continue
        
            
        if removeVariables:
            result_file = resultDir+"/result"+str(assertion_founded)+".txt"
            resultAnalysis(resultDir,runtimeRemoveVariables)
            result = smart(current_path, main_module,result_file,init_variables)

            if result == 0:
                writeLog("log.txt", "Remover Smart Loop is successful\n\n")
                smart_sucess += 1
                with open(result_file, "r") as f:
                    new_assertion = f.read()
                
                if new_assertion not in verified_assertion:
                    verified_assertion.append(new_assertion)
                    assertion_founded += 1
            
                if assertion_founded == assertion:
                    writeLog("log.txt","Finish all the assertion\n")
                    break
            else:
                writeLog("log.txt","Remover Smart Loop  is failure\n\n")
            
            

           

        

    smart_end_time = time.time()
    smart_time = smart_end_time - smart_start_time

    all_end_time = time.time()
    all_time = all_end_time - all_start_time

    print("Smart Time: ", smart_time)
    print("Overall Time: ", all_time)
    print("all Assertion: ", verified_assertion)
    with open("result.txt","w") as f:
        f.write("Smart Time: "+str(smart_time)+"\n")
        f.write("Overall Time: "+str(all_time)+"\n")
        for assertion in verified_assertion:
            f.write(str(assertion)+"\n")
        
    
