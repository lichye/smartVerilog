import re
import os
import random
import sys
import subprocess
import shutil

ebmc_unsupport_list = []

verilog_keywords = [
    # Module and Ports
    "module", "endmodule", "input", "output", "inout",
    
    # Data Types
    "wire", "reg", "integer", "real", "time", "parameter", "localparam",
    "genvar", "logic", "bit", "tri", "tri0", "tri1", "supply0", "supply1",
    "trireg", "vectored", "scalared", "signed", "unsigned", "event",
    
    # Control Flow
    "always", "initial", "if", "else", "for", "while", "do", "repeat", 
    "forever", "case", "endcase", "casex", "casez", "default", "disable",
    "wait", "fork", "join",
    
    # Procedural Blocks
    "begin", "end", "assign", "deassign", "force", "release",
    
    # Timing
    "posedge", "negedge", "edge", "specify", "endspecify", 
    "timeunit", "timeprecision",
    
    # Operators
    "and", "or", "nand", "nor", "xor", "xnor", "not",
    
    # Compiler Directives
    "`define", "`undef", "`ifdef", "`else", "`elsif", "`endif", 
    "`include", "`timescale", "`default_nettype", "`celldefine", 
    "`endcelldefine", "`unconnected_drive", "`nounconnected_drive", 
    "`resetall", "`line",
    
    # Assertions and Simulation Control
    "assert", "assume", "cover", "posedge", "negedge", 
    "$display", "$monitor", "$stop", "$finish", "$time", "$random", 
    "$realtime",
    
    # Functions and Tasks
    "function", "endfunction", "task", "endtask",
    
    # Generate Blocks
    "generate", "endgenerate",
]

class VerilogMutation:
    def __init__(self, input_file, output_dir):
        self.input_file = input_file
        self.output_dir = output_dir
        self.code_lines = []
        self.mutations = []
        self.unmatched_rules = []
        self.always_block = False 

    def load_verilog(self):
        with open(self.input_file, 'r') as file:
            self.code_lines = file.readlines()

    def mutate_localparam(self,localparam_str):
        match = re.match(
            r"localparam\s+(?P<range>\[\d+:\d+\])\s+(?P<variable>[a-zA-Z_]\w*)\s*=\s*(?P<value>\d+);",
            localparam_str
        )
        if not match:
            return localparam_str

        current_value = int(match.group("value"))

        bit_range = match.group("range")
        bit_width = int(bit_range.split(":")[0][1:]) - int(bit_range.split(":")[1][:-1]) + 1
        max_value = (1 << bit_width) - 1
        mutated_value = random.choice([v for v in range(0, max_value + 1) if v != current_value])
        return f"localparam {match.group('range')} {match.group('variable')} = {mutated_value};"

    def define_mutations(self):    
        # define muation operations
        self.mutations = []
    
        # operation sets
        self.bitwise_ops = {"&": r"\&", "|": r"\|", "^": r"\^"}
        self.arithmatic_ops = {"+": r"\+", "-": r"-", "*": r"\*", "/": r"/", "%": r"%"}
        self.relational_ops = {"==": r"==", "!=": r"!=", ">": r">", "<": r"<", ">=": r">=", "<=": r"<="}
        # • Verilog has 8 gate types that are primitive components: and, or, nand, nor, xor, xnor, not, buf
        self.gate_ops = {"and", "or", "nand", "nor", "xor", "xnor","not","buf"}

        # gate mutation
        for op in self.gate_ops:
            self.mutations.append({
                "category": "gate",
                "pattern": rf"\b{op}\b",
                "replacement": random.choice([y for y in self.gate_ops if y != op])
            })
        
        # bitwise mutation
        for op, escaped_op in self.bitwise_ops.items():
            self.mutations.append({
                "category": "bitwise",
                "pattern": escaped_op,
                "replacement": random.choice([y for y in self.bitwise_ops.keys() if y != op])
            })
        
        # arithmatic mutation
        for op, escaped_op in self.arithmatic_ops.items():
            self.mutations.append({
                "category": "arithmatic",
                "pattern": escaped_op,
                "replacement": random.choice([y for y in self.arithmatic_ops.keys() if y != op])
            })
        
        # relational mutation
        for op, escaped_op in self.relational_ops.items():
            self.mutations.append({
                "category": "relational",
                "pattern": escaped_op,
                "replacement": random.choice([y for y in self.relational_ops.keys() if y != op])
            })

        self.mutations.append(
            {
                "category": "logical_negation",
                "pattern": r"(?<![a-zA-Z_])\b(0|1)\b(?![a-zA-Z_'])",  
                "replacement": lambda m: "1" if m.group(1) == "0" else "0"  # 0 → 1, 1 → 0
            }
        )

        # variable_negation mutation
        self.mutations.append({
            "category": "variable_negation",
            "pattern": r"(?<![!~\d0-9'])\b[a-zA-Z_]\w*\b", 
            "replacement": lambda m: random.choice([f"!{m.group(0)}", f"~{m.group(0)}"])
        })

        # localparam mutation
        self.mutations.append({
            "category": "localparam_mutation",
            "pattern": r"localparam\s+\[\d+:\d+\]\s+[a-zA-Z_]\w*\s*=\s*\d+;",
            "replacement": lambda m: self.mutate_localparam(m.group(0))
        })
        
        # if negation mutation
        self.mutations.append({
            "category": "if_negation",
            "pattern": r"if\s*\(\s*(csr_mfip_i\[\d+\])\s*\)",  # 匹配 if 条件中的数组元素
            "replacement": lambda m: f"if (!{m.group(1)})"
        })
        
        # negation flip mutation
        self.mutations.append({
            "category": "negation_flip",
            "pattern": r"(?<![a-zA-Z0-9_])([!~])([a-zA-Z_]\w*)\b", 
            "replacement": lambda m: f"{m.group(2)}"
        })

        # logical mutation for random number generation
        self.mutations.append({
            "category": "verilog_random_literal",
            "pattern": r"(\d+)'([bBoOdDhH])[01]+",
            "replacement": lambda m: f"{m.group(1)}'{m.group(2)}{''.join(random.choice(['0', '1']) for _ in range(int(m.group(1))))}"
        })

        # Logical mutation for changing input to output
        self.mutations.append({
            "category": "verilog_io_mutation",
            "pattern": r"\binput\s+wire\s+(\w+);",
            "replacement": lambda m: f"output wire {m.group(1)};"
        })

        # Logical mutation for changing output to input
        self.mutations.append({
            "category": "verilog_io_mutation",
            "pattern": r"\boutput\s+wire\s+(\w+);",
            "replacement": lambda m: f"input wire {m.group(1)};"
        })

        # Below is unmatch rule
        self.unmatched_rules.append("always")
        self.unmatched_rules.append("//")
        print(f"Generated {len(self.mutations)} mutation rules.")

    def is_in_always_block(self, line):
        if re.search(r"^\s*always\b", line):
            self.always_block = True
            self.begin_count = 1 
            return

        if self.always_block:
            if re.search(r"\bbegin\b", line):  
                self.begin_count += 1
            if re.search(r"\bend\b", line):    
                self.begin_count -= 1
            
            if self.begin_count == 0:
                self.always_block = False

    def write_to_file(self, mutation_count,line,modified_line,comment=""):
        file_dir = self.output_dir+"/mutant_"+str(mutation_count)+".sv"
        with open(file_dir, 'w') as file:
            file.write(comment)
            for self_line in self.code_lines:
                if self_line == line:
                    file.write(modified_line)
                else:
                    file.write(self_line)

    def generate_mutants(self):
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

        # we should loop over each line
        # loop over each mutation rule
        # if a line and a rule match, apply the mutation, make a new file

        self.always_block = False 
        
        test_count = 0

        for line in self.code_lines:
            mutated_lines = []
            self.is_in_always_block(line)  # update always block status
            unmatch_flag = False
            for rule in self.unmatched_rules:
                if rule in line:
                    unmatch_flag = True
                    break
            if unmatch_flag:
                continue

            for mutation in self.mutations:
                if(mutation["category"]=="variable_negation"):
                    if not self.always_block:
                        # print("Not in always block")
                        continue
                    matches = re.findall(mutation["pattern"], line)
                    for match_index, match in enumerate(matches):
                        modified_line = re.sub(
                            rf"\b{re.escape(match)}\b", 
                            mutation["replacement"](re.match(rf"\b{re.escape(match)}\b", match)), 
                            line
                        )
                        test_count += 1
                        # print(f"This is mutation {test_count}")
                        # print(f"The match is: {match}")
                        # print(f"The pattern is: {mutation['category']}")
                        # print(f"Mutations: {line.strip()} -> {modified_line.strip()}")
                        comment = f"// This is mutation {test_count} for {mutation['category']}\n"
                        comment += f"// The match is: {match}\n"
                        comment += f"// The pattern is: {mutation['category']}\n"
                        comment += f"// Mutations: {line.strip()} -> {modified_line.strip()}\n"
                        self.write_to_file(test_count, line, modified_line, comment)
                else:   
                    if len(re.findall(mutation["pattern"], line))>0:
                        modified_line = re.sub(mutation["pattern"], mutation["replacement"], line)
                        test_count += 1
                        
                        my_matchs = re.findall(mutation["pattern"], line)
                        # print("This is the "+str(test_count)+" mutation")
                        # print("The match is : "+str(my_matchs))
                        # print("The pattern is: "+mutation["category"])
                        # print("Muations: "+line + " -> " + modified_line)
                        # then we should write the modified line to the file
                        comment = f"// This is mutation {test_count} for {mutation['category']}\n"
                        comment += f"// The match is: {my_matchs}\n"
                        comment += f"// The pattern is: {mutation['category']}\n"
                        comment += f"// Mutations: {line.strip()} -> {modified_line.strip()}\n"
                        self.write_to_file(test_count, line, modified_line, comment)
        
        return test_count
        
    def run(self):
        self.load_verilog()
        self.define_mutations()
        # print("Generating mutants...")
        for muation in self.mutations:
            print(muation)
        file_count=self.generate_mutants()
        return file_count

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
        cnt += 1
        if((cnt / total_files) * 100- percentage >10):
            percentage = (cnt / total_files) * 100
            print(f"Processing file {cnt}/{total_files} ({percentage:.2f}%)...")
            
        # print(f"Running Formal Checker on: {verilog_file}")
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
                
                subprocess.run(["rm",new_file_path])
                    # print("file: ",verilog_file," is not verified")
                    # print("result: ",result.stdout)
                    # print("result2: ",result2.stdout)
                    # exit(1)
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

def move_files(src_folder, dest_folder,delete_file):
    """
    Move all files from the source folder to the destination folder.

    Parameters:
        src_folder (str): Path to the source folder containing files to move.
        dest_folder (str): Path to the destination folder where files will be moved.

    Returns:
        None
    """
    # Check if the source folder exists
    if not os.path.exists(src_folder):
        print(f"Error: Source folder '{src_folder}' does not exist.")
        return

    # Create the destination folder if it doesn't exist
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)

    # Get a list of all files in the source folder
    files = [file for file in os.listdir(src_folder) if os.path.isfile(os.path.join(src_folder, file))]

    if not files:
        print(f"No files found in source folder '{src_folder}' to move.")
        return

    # Move each file to the destination folder
    for file in files:
        src_path = os.path.join(src_folder, file)
        dest_path = os.path.join(dest_folder, file)
        try:
            if(src_path == delete_file):
                continue
            shutil.copy(src_path, dest_path)
            print(f"Moved: {file}")
        except Exception as e:
            print(f"Error moving file {file}: {e}")

    print(f"All files moved from '{src_folder}' to '{dest_folder}'.")

def remove_files(file_list):
    for file in file_list:
        # print(f"Removing: {file}")
        try:
            os.remove(file)
            # print(f"Removed: {file}")
        except Exception as e:
            print(f"Error removing file {file}: {e}")

if __name__ == "__main__":
    # Set the configuration
    random.seed(42)
    if(len(sys.argv) != 5):
        print("Usage: python mutation.py <input_file> <filename>")
        exit(1)
        # print("Run default configuration")
        # input_file_dir = "verilog/"
        # input_file = input_file_dir+"ibex_controller.sv"  
        # output_dir = "mutants"
    else:
        input_file_dir = sys.argv[1]
        input_file = input_file_dir+sys.argv[2]
        output_dir = sys.argv[3]
        top_module = sys.argv[4]
        # output_dir = sys.argv[3]

    # output_dir = "mutants"
    
    # Generate mutants
    mutation_tool = VerilogMutation(input_file, output_dir)
    file_count = mutation_tool.run()
    
    # Setup the environment
    move_files(input_file_dir, output_dir,input_file)

    # Run ebmc on the generated mutants and remove error files
    # bad_files = []
    bad_files = run_fm_on_verilog_files(output_dir,["1'b1"])
    
    remove_files(bad_files)

    # finish the process
    print("Generated "+str(file_count)+" mutant files in '"+output_dir+"' directory.")
    print("Removed "+str(len(bad_files))+" bad files.")
    print("Remain files number: "+str(file_count-len(bad_files)))