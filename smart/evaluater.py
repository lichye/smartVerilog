
import os
import sys
import subprocess
#this is the list that ebmc does not support

ebmc_unsupport_list = []

def write_assertion_file(input_file,output_file,assertions):
    
    with open(input_file, "r") as file:
        content = file.readlines()
        for i in range(len(content)):
            if content[i].startswith("module"):
                for assertion in assertions:
                    write_assertion = "assert property ("+assertion+");\n"
                    content.insert(i+1,write_assertion)
                break
    with open(output_file, "w") as file:
        file.writelines(content)

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

    for verilog_file in verilog_files:
        print(f"Running sby on: {verilog_file}")
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
                    

                cmd = [sby_path,"-f",sby_file,"task"]
                # print("cmd: ",cmd)
                
                cmd2 = ["ebmc",new_file_path,"--bound","10","--top",top_module]

                result = subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
                
                if top_module in ebmc_unsupport_list:
                    result2 = 0
                else:
                    result2 = subprocess.run(cmd2,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)

                # print("the return code is: ",result.returncode)                     
                if(result.returncode != 0 or result2.returncode != 0):
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
    print("Found total mutations: ",len(find_files))
        
    all_file = [
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if file.startswith("mutant_") and file.endswith(".sv") and not file.endswith("_assertion.sv")
    ]

    unfind_file = set(all_file) - set(find_files)

    sorted(unfind_file)
    if(len(unfind_file) != 0):
        print("UnFound mutations: ",len(unfind_file))
        print("UnFound mutations: ",unfind_file)

    print("Total mutations: ",len(unfind_file)+len(find_files))
    print("Coverage percentage: ",(len(find_files)/(len(unfind_file)+len(find_files)))*100)
    