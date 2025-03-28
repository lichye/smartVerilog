
import os
import sys
import re
import time
import subprocess
import threading
import uuid
from concurrent.futures import ProcessPoolExecutor
#this is the list that ebmc does not support

ebmc_unsupport_list = ["c432","c499","c880","c1355","c2670","c3540","c6288","c7552","s298","s344","s349",
                        "s382","s386","s400","s420","s444","s510","s641","s713","s832","s838","s953",
                        "s1196","s1238","s1432","s1488","s1494","s5378","s9234","s13207","s15850","s35932",
                        "s38417","s38584","ibex_controller","ibex_decoder","ibex_id_stage","ibex_multdiv_slow"]
timeout_list =[]


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

        new_file_name = f"{file_base}_assertion{file_ext}"

        new_file_path = os.path.join(dir_name, new_file_name)

        write_assertion_file(verilog_file,new_file_path,Tproperty)

        return_result = []

        try:
            with open(sby_file, "w") as sby_run_file:
                sby_run_file.write("[tasks]\n")
                sby_run_file.write("task bmc_check files\n")
                sby_run_file.write("\n")
                sby_run_file.write("[options]\n")
                sby_run_file.write("bmc_check:\n")
                sby_run_file.write("mode bmc\n")
                sby_run_file.write("depth 1\n")
                sby_run_file.write("timeout 180\n")
                sby_run_file.write("vcd_sim on\n")
                sby_run_file.write("\n")
                sby_run_file.write("[engines]\n")
                sby_run_file.write("bmc_check:\n")
                sby_run_file.write("smtbmc --unroll\n")
                sby_run_file.write("\n")
                sby_run_file.write("[script]\n")
                sby_run_file.write("files:\n")
                sby_run_file.write("read -sv "+new_file_path+"\n")
                for file in verilog_related_files:
                    sby_run_file.write("read -sv "+file+"\n")
                # file.write("prep -top "+)
                sby_run_file.write("prep -top "+top_module+"\n")
                

            cmd = ["timeout","180","sby","-f",sby_file,"task"]
            # print("cmd: ",cmd)
            # cmd = [sby_path,"-f",sby_file,"task"]
            
            cmd2 = ["timeout","180","ebmc",new_file_path,"--bound","10","--top",top_module]

            # cmd2 = ["ebmc",new_file_path,"--bound","10","--top",top_module]

            result = subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            
            if top_module in ebmc_unsupport_list:
                if(result.returncode != 0):
                    if(result.returncode ==124):
                        return_result.append({verilog_file:"timeout"})
                    else:
                        return_result.append({verilog_file:"error"})
                else:
                    return_result.append({verilog_file:"verified"})
            else:
                result2 = subprocess.run(cmd2,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
                # print("the return code is: ",result.returncode)                     
                if(result.returncode != 0 or result2.returncode != 0):
                    if(result.returncode ==124 or result2.returncode == 124):
                        return_result.append({verilog_file:"timeout"})
                    else:
                        return_result.append({verilog_file:"error"})
                else:
                    return_result.append({verilog_file:"verified"})

        except Exception as e:
            print(f"Failed to create .sby file: {e}")
            return_result.append({verilog_file:"error"})

    except Exception as e:
        print(f"Failed to run sby on {verilog_file}: {e}")
        return_result.append({verilog_file:"error"})
    # subprocess.run(["rm","-rf",sby_file])
    # subprocess.run(["rm","-rf",sby_result])
    time_end = time.time()
    print("Finish evaluate time: "+str(time_end-time_start)+" on the file: "+verilog_file + "with sby file: "+sby_file)
    return return_result

if __name__ == "__main__":
    if(len(sys.argv) < 2):
        print("Usage: python3 evaluater.py top_module")
        exit(1)
    else:
        top_module = sys.argv[1]

    
   
    working_dir = os.getcwd()

    property_file = working_dir+"/invariants.txt"
    
    properties = read_file(property_file)

    verilogDir = working_dir+"/user/"+top_module+".sv"

    verified_cnt = 0
    total_cnt = 0
    for propert in properties:
        result = run_fm_on_verilog_file(verilogDir,propert,[])
        if(result[0][verilogDir] == "verified"):
            verified_cnt+=1
        total_cnt+=1
    print("The total number of properties is: "+str(total_cnt))
    print("The number of verified properties is: "+str(verified_cnt))        