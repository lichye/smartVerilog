
import os
import sys
import re
import time
import subprocess
#this is the list that ebmc does not support

ebmc_unsupport_list = ["c432","c499","c880","c1355","c2670","c3540","c6288","c7552","s298","s344","s349",
                        "s382","s386","s400","s420","s444","s510","s641","s713","s832","s838","s953",
                        "s1196","s1238","s1432","s1488","s1494","s5378","s9234","s13207","s15850","s35932",
                        "s38417","s38584"]
timeout_list =[]


def write_assertion_file(input_file, output_file, assertions):
    try:
        with open(input_file, "r") as file:
            content = file.readlines()
        
        module_start_found = False
        module_end_found = False
        modified_content = []

        module_start_pattern = re.compile(r"^\s*module\s+\w+")
        semicolon_pattern = re.compile(r";")

        for i, line in enumerate(content):
            # Check if the module declaration has been found
            if not module_start_found and module_start_pattern.match(line):
                module_start_found = True
                modified_content.append(line)
                continue
            
            if module_start_found and not module_end_found:
                modified_content.append(line)
                if semicolon_pattern.search(line):  
                    module_end_found = True
                    for assertion in assertions:
                        modified_content.append(f"    assert property ({assertion});\n")
                continue
            

            modified_content.append(line)

        if not module_start_found:
            print(f"Warning: {input_file} does not contain a module declaration.")
        elif not module_end_found:
            print(f"Warining: {input_file} does not contain a module end declaration.")

        with open(output_file, "w") as file:
            file.writelines(modified_content)

        # print(f"Assertions sucess {output_file}")

    except FileNotFoundError:
        print(f"Erorr: File '{input_file}' not found.")
    except IOError as e:
        print(f"Erorr: {e}")


def run_fm_on_verilog_files(directory, properties, sby_path="sby"):
    """
    Automatically runs `sby` on all Verilog files named mutant_*.sv in the specified directory.

    Parameters:
        directory (str): Path to the directory containing Verilog files.
        sby_path (str): Path to the `sby` executable (default: "sby").

    Returns:
        None
    """
    print("The directory is: ",directory)
    # Check if the directory exists
    if not os.path.exists(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        return

    # Get all Verilog files matching mutant_*.sv in the directory
    verilog_files = [
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if file.startswith("mutant_") and file.endswith(".sv") and not file.endswith("_assertion.sv")
    ]

    verilog_related_files =[
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if not file.startswith("mutant_") and file.endswith(".sv") and not file.endswith("_assertion.sv")
    ]

    if not verilog_files:
        print("No matching Verilog files found in the specified directory.")
        return

    print(f"Found {len(verilog_files)} Verilog files. Starting to process...")

    error_files = []  # List to track files with errors

    cnt = 0
    percentage = 0
    total_files = len(verilog_files)

    for verilog_file in verilog_files:
        # print(f"Running Formal Checker on: {verilog_file}")
        time_start = time.time()
            
        try:
            # Run sby command
            # firstly we should build .sby file
            sby_file = os.getcwd()+"/dis.sby"
            # print("The verilog file is: ",verilog_file)

            dir_name = os.path.dirname(verilog_file)         
            
            file_base, file_ext = os.path.splitext(os.path.basename(verilog_file))

            new_file_name = f"{file_base}_assertion{file_ext}"

            new_file_path = os.path.join(dir_name, new_file_name)

            write_assertion_file(verilog_file,new_file_path,properties)

            try:
                with open(sby_file, "w") as sby_run_file:
                    sby_run_file.write("[tasks]\n")
                    sby_run_file.write("task bmc_check files\n")
                    sby_run_file.write("\n")
                    sby_run_file.write("[options]\n")
                    sby_run_file.write("bmc_check:\n")
                    sby_run_file.write("mode bmc\n")
                    sby_run_file.write("depth 10\n")
                    sby_run_file.write("timeout 10000\n")
                    sby_run_file.write("vcd_sim on\n")
                    sby_run_file.write("\n")
                    sby_run_file.write("[engines]\n")
                    sby_run_file.write("bmc_check:\n")
                    sby_run_file.write("smtbmc --unroll --nopresat cvc5\n")
                    sby_run_file.write("\n")
                    sby_run_file.write("[script]\n")
                    sby_run_file.write("files:\n")
                    sby_run_file.write("read -sv "+new_file_path+"\n")
                    for file in verilog_related_files:
                        sby_run_file.write("read -sv "+file+"\n")
                    # file.write("prep -top "+)
                    sby_run_file.write("prep -top "+top_module+"\n")
                    

                cmd = ["timeout","20",sby_path,"-f",sby_file,"task"]
                # print("cmd: ",cmd)
                # cmd = [sby_path,"-f",sby_file,"task"]
                
                cmd2 = ["timeout","20","ebmc",new_file_path,"--bound","10","--top",top_module]

                # cmd2 = ["ebmc",new_file_path,"--bound","10","--top",top_module]

                result = subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
                
                if top_module in ebmc_unsupport_list:
                    if(result.returncode != 0):
                        if(result.returncode ==124):
                            timeout_list.append(verilog_file)
                        else:
                            error_files.append(verilog_file)
                else:
                    result2 = subprocess.run(cmd2,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
                    # print("the return code is: ",result.returncode)                     
                    if(result.returncode != 0 or result2.returncode != 0):
                        if(result.returncode ==124 or result2.returncode == 124):
                            timeout_list.append(verilog_file)
                        else:
                            error_files.append(verilog_file)
                # else:
                #     print("file: ",verilog_file," is verified")
                # subprocess.run(["rm",new_file_path])

            except Exception as e:
                print(f"Failed to create .sby file: {e}")
                error_files.append(verilog_file)
                exit(1)

        except Exception as e:
            print(f"Failed to run sby on {verilog_file}: {e}")
            error_files.append(verilog_file)
            exit(1)
        
        cnt += 1
        time_end = time.time()
        print("Finish evaluate "+str(cnt)+"/"+str(total_files)+" time: "+str(time_end-time_start)+" on the file: "+verilog_file)

    return error_files

def get_result_files(directory):

    result_files = set()
    
    if not os.path.exists(directory):
        raise FileNotFoundError(f"Directory '{directory}' does not exist.")

    for filename in os.listdir(directory):
        full_path = os.path.join(directory, filename)
        if filename.startswith("result") and os.path.isfile(full_path):
            result_files.add(full_path)

    return result_files

def get_mutant_files(directory):
    
    mutant_files = set()
        
    if not os.path.exists(directory):
        raise FileNotFoundError(f"Directory '{directory}' does not exist.")
    
    for filename in os.listdir(directory):
        full_path = os.path.join(directory, filename)
        if filename.startswith("mutant") and os.path.isfile(full_path):
            mutant_files.add(full_path)
    return mutant_files

def read_file(file):
    with open(file, "r") as file:
        content = file.read()
    return content

def count_logfile():
    with open('log.txt', 'r') as file:
        logfile_content = file.read()
    
    fm_checker_pattern = re.compile(r"FM checker are called (\d+) times")
    fm_timer_pattern = re.compile(r"FM checker Timer: ([\d\.]+) seconds")
    cvc5_pattern = re.compile(r"CVC5 are called (\d+) times")
    cvc5_timer_pattern = re.compile(r"CVC5 Timer: ([\d\.]+) seconds")

    fm_checker_calls = 0
    fm_checker_time = 0.0
    cvc5_calls = 0
    cvc5_time = 0.0

    for match in fm_checker_pattern.finditer(logfile_content):
        fm_checker_calls += int(match.group(1))
    for match in fm_timer_pattern.finditer(logfile_content):
        fm_checker_time += float(match.group(1))
    for match in cvc5_pattern.finditer(logfile_content):
        cvc5_calls += int(match.group(1))
    for match in cvc5_timer_pattern.finditer(logfile_content):
        cvc5_time += float(match.group(1))

    result = "FM Checker Total Calls: " + str(fm_checker_calls) + "\n"
    result += "FM Checker Total Time: " + str(fm_checker_time) + " seconds\n"
    result += "CVC5 Total Calls: " + str(cvc5_calls) + "\n"
    result += "CVC5 Total Time: " + str(cvc5_time) + " seconds\n"
    return result

if __name__ == "__main__":
    if(len(sys.argv) < 2):
        print("Usage: python3 evaluater.py top_module")
        exit(1)
    else:
        top_module = sys.argv[1]

    property_dir = "result"
    bound = 10
    ebmc_path = "ebmc"
    working_dir = os.getcwd()
    directory = working_dir+"/benchmarks"
    
    properties = []
    property_files = get_result_files(property_dir)
    for file in property_files:
        read = read_file(file)
        properties.append(read)
    print("Properties: ",properties)

    find_files = set()
    # find_files.update(run_ebmc_on_verilog_files(directory,properties,bound,ebmc_path))
    find_files = run_fm_on_verilog_files(directory,properties)

    # print("Found mutations: ",sorted(find_files))
    sorted(find_files)
    # print("Found mutations: ",find_files)

    all_file = [
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if file.startswith("mutant_") and file.endswith(".sv") and not file.endswith("_assertion.sv")
    ]

    unfind_file = set(all_file) - set(find_files) - set(timeout_list)

    sorted(unfind_file)
    if(len(unfind_file) != 0):
        print("UnFound mutations: ",len(unfind_file))
        print("UnFound mutations: ",unfind_file)

    total_mutations = len(unfind_file)+len(find_files)

    print("Found mutations: ",len(find_files))
    print("UnFound mutations: ",len(unfind_file))
    print("Timeout mutations: ",len(timeout_list))
    print("Total mutations: ",total_mutations+len(timeout_list))
    print("Total mutations(Without timeout file): ",total_mutations)
    print("Coverage percentage: ",(len(find_files)/total_mutations)*100)
    
    log_result = count_logfile()

    current_path = os.getcwd()
    resultfile = current_path+"/result_"+top_module+".txt"

    with open(resultfile,"a") as f:
        f.write(log_result)
        f.write("\n")
        f.write("Found mutations: "+str(len(find_files))+"\n")
        f.write("UnFound mutations: "+str(len(unfind_file))+"\n")
        f.write("Timeout mutations: "+str(len(timeout_list))+"\n")
        f.write("Total mutations(Without timeout file): "+str(total_mutations)+"\n")
        f.write("Coverage percentage: "+str((len(find_files)/total_mutations)*100)+"\n")