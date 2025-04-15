
import os
import sys
import re
import time
import subprocess
import threading
import uuid
from concurrent.futures import ProcessPoolExecutor
#this is the list that ebmc does not support

def write_assertion_file(input_file, output_file, assertion):
    try:
        with open(input_file, "r") as file:
            content = file.readlines()
        
        module_start_found = False
        module_end_found = False
        modified_content = []

        module_start_pattern = re.compile(r"^\s*module\s+\w+")
        module_end_pattern = re.compile(r"^\s*endmodule")
        semicolon_pattern = re.compile(r";")

        found_module = False
        
        for i, line in enumerate(content):
            if top_module in line:
                found_module = True
            
            if module_end_pattern.match(line) and found_module:
                modified_content.append(f"    assert property ({assertion});\n")
                found_module = False
                
            modified_content.append(line)
        
        with open(output_file, "w") as file:
            file.writelines(modified_content)

        # print(f"Assertions sucess {output_file}")

    except FileNotFoundError:
        print(f"Erorr: File '{input_file}' not found.")
    except IOError as e:
        print(f"Erorr: {e}")

def read_file(file):
    with open(file, "r") as file:
        content = file.readlines()
    
    content = [content.strip() for content in content]
    return content

def run_fm_on_verilog_file(verilog_file,Tproperty,verilog_related_files):
    time_start = time.time()
    unique_id = uuid.uuid4() 
    sby_file = os.path.join(os.getcwd(), f"dis_{unique_id}.sby")
    sby_result = os.path.join(os.getcwd(), f"dis_{unique_id}_task")

    try:
        dir_name = os.path.dirname(verilog_file)         
        
        file_base, file_ext = os.path.splitext(os.path.basename(verilog_file))

        pid = os.getpid()

        new_file_name = f"{file_base}_{pid}_assertion{file_ext}"

        new_file_path = os.path.join(dir_name, new_file_name)

        write_assertion_file(verilog_file,new_file_path,Tproperty)

        return_result = []

        try:
            # cmd = ["timeout","180","sby","-f",sby_file,"task"]
            # print("cmd: ",cmd)
            # cmd = [sby_path,"-f",sby_file,"task"]
            
            # ebmc_cmd = ["timeout","180","ebmc",new_file_path,"--bound","10","--top",top_module]
            ebmc_cmd = ["timeout","180","ebmc",new_file_path,"--k-induction","--top",top_module]
            # cmd2 = ["ebmc",new_file_path,"--bound","10","--top",top_module]

            # result = subprocess.run(cmd2,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            
            ebmc_result = subprocess.run(ebmc_cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            # print("the return code is: ",result.returncode)                     
            if(ebmc_result.returncode != 0):
                if(ebmc_result.returncode == 124):
                    return_result.append({Tproperty,"timeout"})
                else:
                    return_result.append({Tproperty,"error"})
            else:
                return_result.append({Tproperty,"verified"})
                subprocess.run(["rm","-rf",new_file_path])

        except Exception as e:
            print(f"Failed to create .sby file: {e}")
            return_result.append({verilog_file:"error"})

    except Exception as e:
        print(f"Failed to run sby on {verilog_file}: {e}")
        return_result.append({verilog_file:"error"})
    # subprocess.run(["rm","-rf",sby_file])
    # subprocess.run(["rm","-rf",sby_result])
    time_end = time.time()
    print("Finish check  property\t"+Tproperty+ "\t\tin time: "+str(time_end-time_start)+ "\tin the thread: "+str(pid))
    return return_result

def extract_property_only(results):
    ignore_tags = {'verified', 'error'}
    
    if isinstance(results, set):
        return [item for item in results if item not in ignore_tags]
    
    elif isinstance(results, list):
        extracted = []
        for result in results:
            props = [item for item in result if item not in ignore_tags]
            if props:
                extracted.append(props[0])
        return extracted
    
    else:
        raise TypeError("Unsupported input type for extract_property_only")


if __name__ == "__main__":
    if(len(sys.argv) < 2):
        print("Usage: python3 evaluater.py top_module")
        exit(1)
    else:
        top_module = sys.argv[1]

    
   
    working_dir = os.getcwd()

    property_file = working_dir+"/assertions.txt"
    
    properties = read_file(property_file)

    verilogDir = working_dir+"/user/"+top_module+".sv"

    verified_cnt = 0
    total_cnt = 0
    verified_assertions = []
    error_assertions = []

    with ProcessPoolExecutor() as executor:
        futures = []
        for propert in properties:
            future = executor.submit(run_fm_on_verilog_file,verilogDir,propert,[])
            futures.append(future)

        for future in futures:
            result = future.result()
            if("verified" in result[0]):
                verified_cnt+=1
                verified_assertions.append(result[0])
            total_cnt+=1
    verified_assertions = extract_property_only(verified_assertions)
    print("The total number of properties is: "+str(total_cnt))
    print("The number of verified properties is: "+str(verified_cnt))     

    with open("invariants.txt","w") as file:
        for assertion in verified_assertions:
            file.write(assertion+"\n")
    
    print("The verified assertions are written to invariants.txt")