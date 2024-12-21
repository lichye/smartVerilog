import re
import os
import random
import sys
import subprocess
import shutil

class VerilogMutation:
    def __init__(self, input_file, output_dir):
        self.input_file = input_file
        self.output_dir = output_dir
        self.code_lines = []
        self.mutations = []
        self.always_block = False 

    def load_verilog(self):
        with open(self.input_file, 'r') as file:
            self.code_lines = file.readlines()

    def define_mutations(self):    
        # define muation operations
        self.mutations = []
    
        # operation sets
        self.bitwise_ops = {"&": r"\&", "|": r"\|", "^": r"\^"}
        self.arithmatic_ops = {"+": r"\+", "-": r"-", "*": r"\*", "/": r"/", "%": r"%"}
        self.relational_ops = {"==": r"==", "!=": r"!=", ">": r">", "<": r"<", ">=": r">=", "<=": r"<="}

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
        
        # Logical negation mutation
        self.mutations.append(
            {
                "category": "logical_negation",
                "pattern": r"(?<![a-zA-Z_])\b(0|1)\b(?![a-zA-Z_'])",  
                "replacement": lambda m: "1" if m.group(1) == "0" else "0"  # 0 → 1, 1 → 0
            }
        )

        # logical negation mutation
        self.mutations.append({
            "category": "variable_negation",
            "pattern": r"(?<![!~\d0-9'])\b[a-zA-Z_]\w*\b", 
            "replacement": lambda m: random.choice([f"!{m.group(0)}", f"~{m.group(0)}"])
        })

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

    def write_to_file(self, mutation_count,line,modified_line):
        file_dir = self.output_dir+"/mutant_"+str(mutation_count)+".sv"
        with open(file_dir, 'w') as file:
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
            
            
            # when we are in always block, we should apply the mutation to the right side of the assignment
            if self.always_block:
                # print("In always block: "+line)
                assignment = re.search(r"^\s*([a-zA-Z_]\w*)\s*(<{0,1}=)\s*(.+?);", line)
                # print("assignment: "+str(assignment)+"\n")
                for muation in self.mutations:
                    if assignment:
                        lhs = assignment.group(1)
                        op = assignment.group(2)
                        rhs = assignment.group(3)
                        if len(re.findall(muation["pattern"], rhs))>0:
                            modified_rhs = re.sub(muation["pattern"], muation["replacement"], rhs)
                            test_count += 1
                            modified_line = lhs + " " + op + " " + modified_rhs + ";\n"
                            print("This is the "+str(test_count)+" mutation")
                            my_matchs = re.findall(muation["pattern"], rhs)
                            print("The match is : "+str(my_matchs))
                            print("The pattern is: "+muation["category"])
                            print("Muations: "+line + " -> " + modified_line)
                            # then we should write the modified line to the file
                            self.write_to_file(test_count,line,modified_line)
            # if we are not in always block, we should apply the mutation to the whole line
            else:
                for muation in self.mutations:
                    if len(re.findall(muation["pattern"], line))>0:
                        modified_line = re.sub(muation["pattern"], muation["replacement"], line)
                        test_count += 1
                        print("This is the "+str(test_count)+" mutation")
                        my_matchs = re.findall(muation["pattern"], line)
                        print("The match is : "+str(my_matchs))
                        print("The pattern is: "+muation["category"])
                        print("Muations: "+line + " -> " + modified_line)
                        # then we should write the modified line to the file
                        self.write_to_file(test_count,line,modified_line)
        
        return test_count
        
    def run(self):
        self.load_verilog()
        self.define_mutations()
        # print("Generating mutants...")
        for muation in self.mutations:
            print(muation)
        file_count=self.generate_mutants()
        return file_count

def run_ebmc_on_verilog_files(directory, ebmc_path="ebmc"):
    """
    Automatically runs `ebmc` on all Verilog files named mutant_*.sv in the specified directory.

    Parameters:
        directory (str): Path to the directory containing Verilog files.
        ebmc_path (str): Path to the `ebmc` executable (default: "ebmc").

    Returns:
        None
    """
    # Check if the directory exists
    if not os.path.exists(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        return

    # Get all Verilog files matching mutant_*.sv in the directory
    verilog_files = [
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if file.startswith("mutant_") and file.endswith(".sv")
    ]

    if not verilog_files:
        print("No matching Verilog files found in the specified directory.")
        return

    print(f"Found {len(verilog_files)} Verilog files. Starting to process...")

    error_files = []  # List to track files with errors

    for verilog_file in verilog_files:
        # print(f"Running ebmc on: {verilog_file}")
        try:
            # Run ebmc command
            result = subprocess.run([ebmc_path, verilog_file],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    text=True)
            
            # Print the output of the command
            # print(f"Output for {verilog_file}:")
            # print(result.stdout)

            # Check for errors
            if result.returncode != 0:
                # print(f"Error for {verilog_file}:")
                # print(result.stderr)
                error_files.append(verilog_file)

        except Exception as e:
            print(f"Failed to run ebmc on {verilog_file}: {e}")
            error_files.append(verilog_file)    
    return error_files

def move_files(src_folder, dest_folder):
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
    input_file_dir = "verilog/"
    input_file = input_file_dir+sys.argv[1]  
    output_dir = "mutants"         
    mutation_tool = VerilogMutation(input_file, output_dir)
    file_count = mutation_tool.run()
    move_files(input_file_dir, output_dir)
    error_files=run_ebmc_on_verilog_files(output_dir)

    print(f"Generated {file_count} mutant files in '{output_dir}' directory.")
    print("Number of error files: "+str(len(error_files)))
    remove_files(error_files)

    print("Remain files number: "+str(file_count-len(error_files)))