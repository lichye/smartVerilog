import re
import configparser
import sys
import math
import random
from itertools import combinations, chain

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

def split_modules(file_path):
    try:
        with open(file_path, 'r') as file:
            content = file.read()
        
        # Regex to match module content and extract module name
        module_pattern = r"module\s+(\w+)\b.*?endmodule\b"
        matches = re.finditer(module_pattern, content, re.DOTALL)
        
        modules = []
        for match in matches:
            module_id = match.group(1)  # Extract module name
            module_content = match.group(0)  # Extract full module 
            modules.append({"id": module_id, "content": module_content})
        
        return modules
    
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return []

def generate_subsets(my_set, max_size):
    subsets = chain.from_iterable(combinations(my_set, r) for r in range(min(len(my_set), max_size), -1, -1))
    return subsets

def get_random_subset(variables, subset_size):
    return random.sample(variables, subset_size)
    
def extract_variables(module_content):
    """
    Extract all variables involved in operations within a Verilog module.
    This function processes different types of operations step by step.

    Args:
        module_content (str): The content of the Verilog module as a string.

    Returns:
        set: A set of variable names involved in operations.
    """
    # Define regex patterns for different types of operations
    rules = {
        "arithmetic": re.compile(r'[\w\[\]\.]+(?:\s*[+\-*/%]\s*[\w\[\]\.]+)+'),  # All arithmetic operations
        "logical": re.compile(r'[\w\[\]\.]+(?:\s*[&|^]{2}\s*[\w\[\]\.]+)+'),     # Logical operations
        "bitwise": re.compile(r'[\w\[\]\.]+(?:\s*[&|^~]\s*[\w\[\]\.]+)+'),       # Bitwise operations
        "relational": re.compile(r'[\w\[\]\.]+(?:\s*(==|!=|>=|<=|>|<)\s*[\w\[\]\.]+)+'),  # Relational operations
        "shift": re.compile(r'[\w\[\]\.]+(?:\s*(<<|>>)\s*[\w\[\]\.]+)+'),         # Shift operations
        "conditional": re.compile(r'[\w\[\]\.]+(?:\s*\?\s*[\w\[\]\.]+\s*:\s*[\w\[\]\.]+)'),  # Conditional operations
        "assignment": re.compile(r'[\w\[\]\.]+\s*(=|<=)\s*[\w\[\]\.]+'),                # Assignment operations
        # "module_instantiation": re.compile(r'^\s*(?!module\b)(\w+)\s+(\w+)\s*\(([^;]*)\);'),
        "input": re.compile(r'input\s+(\w+)'),
        "output": re.compile(r'output\s+(\w+)'),
        "inout": re.compile(r'inout\s+(\w+)'),
        "wire": re.compile(r'wire\s+(\w+)'),
        "wire2": re.compile(r'\w+(?=,|$)'),
        "VCDvar": re.compile(r'\$var\s+(\w+)\s+(\d+)\s+([^\s]+)\s+([\w_]+)\s+\[([\d:]+)\]\s+\$end'),
        # "module": re.compile(r'module\s+(\w+)'),
    }
    
    variables = set()
    # print("This is the module content")
    # Loop through each operation type and find all matches
    for line in module_content.split("\n"):
        # print("This line is "+line)
        for operation_type, pattern in rules.items():
            # print("\t We check rule "+operation_type)
            matches = pattern.findall(line)
            if(matches):
            # print("\t\t We found matches")
                variable_pattern = re.compile(r'\b[a-zA-Z_][a-zA-Z0-9_]*\b')
                variable = variable_pattern.findall(line)
                # print("\t\t\t We found variables "+str(variable))
                variables.update(variable)
            
    return variables

def write_to_file(output_file, data):
    try:
        with open(output_file, 'w') as file:
            file.write(data)
        # print(f"Data written to {output_file}")
    except Exception as e:
        print(f"Error writing to file: {e}")

def readFile(file_path):
    try:
        with open(file_path, 'r') as file:
            content = file.read()
        return content
    except:
        print(f"Error: File {file_path} not found.")
        return []

def clean_verilog_input(verilog_code):
    verilog_code = verilog_code.encode('utf-8').decode('utf-8-sig')
    verilog_code = verilog_code.replace('\r\n', '\n')
    verilog_code = verilog_code.replace('\t', '    ')
    return verilog_code

def extract_variables_from_file(verilog_code):
    pattern = re.compile(r'(always\s*@\s*\([^\)]*\)\s*(?:.|\n)*?)(?=^\s*always\s*@|\Z)', re.IGNORECASE | re.MULTILINE)
    verilog_code = clean_verilog_input(verilog_code)
    matches = pattern.findall(verilog_code)
    variables_list = list()
    for always_block in matches:
        varibales = extract_variables_from_block(always_block)
        variables_list.append(varibales)
    return variables_list

def extract_variables_from_block(block):
    assignments = re.findall(r'(\w+)\s*(<=|=)\s*[^;]+;', block)
    vars_used = set()
    for left_var, _ in assignments:
        vars_used.add(left_var.strip())
    right_vars = re.findall(r'(?:<=|=)\s*([^;]+);', block)
    for expr in right_vars:
        for token in re.findall(r'\b\w+\b', expr):
            vars_used.add(token)
    return vars_used

if __name__ == "__main__":
    random.seed(42)
    if(len(sys.argv)!=5):
        print("Usage: python3 preAnalyzer.py <input_file> <top_module> <runtimeVariablesDir>")
        sys.exit(1)
    else:
        work_dir = sys.argv[1]
        input_file = sys.argv[2]
        top_module = sys.argv[3]
        output_file = sys.argv[4]

    vcd_file = readFile(work_dir+"/runtime/sim_results/sim1.vcd")

    modules = split_modules(input_file)
    
    variables = set()
    
    verilog_variables = set()
    vcd_variables = set()

    vcd_variables = extract_variables(vcd_file)
    
    for module in modules:
        if(module["id"] == top_module):
            top_module_content = module["content"]
            vari = extract_variables(top_module_content)
            verilog_variables.update(vari)

    for var in verilog_variables:
        if var in vcd_variables and var not in verilog_keywords:
            variables.add(var)

    print("Filtered variabls size is "+str(len(variables)))
    # print("The final variables are "+str(variables))

    resultfile = work_dir+"/result_"+top_module+".txt"

    # with open(resultfile, "a") as f:
    #     f.write("There is "+str(len(variables))+" variables\n")
    #     f.write("The variables are\n")
    #     f.write(" ".join([f"{var}" for var in variables]))
    #     f.write("\n")
    
    
    
    variables = sorted(list(variables))
    size_of_variables = len(variables)
    if size_of_variables >= 400:
        subset_size = 20
        smart_loop = size_of_variables
    elif size_of_variables >10:
        subset_size = 5
        smart_loop = size_of_variables*2
    else:
        subset_size = size_of_variables
        smart_loop = size_of_variables*3


    init_cnt = 0
    # subsets = generate_combinations(variables, subset_size)
    if(size_of_variables >10):
        for i in range(0,smart_loop):
            subset = get_random_subset(variables, subset_size)
            write_to_file(output_file+"Init_"+str(i)+".txt", "\n".join([f"{var}" for var in subset]))
    else:
        print("The subset size is "+str(subset_size))
        subsets = generate_subsets(variables, subset_size)
        for subset in subsets:
            if(len(subset) >=2):
                write_to_file(output_file+"Init_"+str(init_cnt)+".txt", "\n".join([f"{var}" for var in subset]))
                init_cnt += 1

    # top_module_content

    variables_list = extract_variables_from_file(top_module_content)
    for variables in variables_list:
        write_varibles = set()
        for var in variables:
            if var in vcd_variables and var not in verilog_keywords:
                write_varibles.add(var)
        if len(write_varibles) > 0:
            write_to_file(output_file+"always_"+str(init_cnt)+".txt", "\n".join([f"{var}" for var in write_varibles]))
            init_cnt += 1
    
    
