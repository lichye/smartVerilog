import re
import configparser
import sys
from itertools import combinations, chain

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
        "module_instantiation": re.compile(r'^\s*(?!module\b)(\w+)\s+(\w+)\s*\(([^;]*)\);'),
        "input": re.compile(r'input\s+(\w+)')
    }
    
    key_words = ["module", "endmodule", "input", "output", "inout", 
                "wire", "reg", "assign", "always", "posedge", 
                "negedge", "if", "else", "begin", "end", "case", 
                "default", "for", "while", "repeat", "forever", 
                "initial", "function", "task", "fork", "join", 
                "disable", "wait", "disable", "repeat"]
    key_words += ["clk","clk_copy"]

    ret_variables = set()
    remove_variables = set()
    print("This is the module content")
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
                variables.update(variable)
    
    for ret_variable in ret_variables:
        if ret_variable in key_words:
            remove_variables.add(ret_variable)

    for remove_variable in remove_variables:
        ret_variables.remove(remove_variable)
        
    return ret_variables

def write_to_file(output_file, data):
    try:
        with open(output_file, 'w') as file:
            file.write(data)
        # print(f"Data written to {output_file}")
    except Exception as e:
        print(f"Error writing to file: {e}")

if __name__ == "__main__":
    if(len(sys.argv)!=4):
        print("Usage: python3 preAnalyzer.py <input_file> <top_module> <runtimeVariablesDir>")
        sys.exit(1)
    else:
        input_file = sys.argv[1]
        top_module = sys.argv[2]
        output_file = sys.argv[3]

    modules = split_modules(input_file)
    
    variables = set()

    for module in modules:
        if(module["id"] == top_module):
            top_module_content = module["content"]
            variables.update(extract_variables(top_module_content))

    # then we come to the a good variables sets
    subsets = generate_subsets(variables, 5)
    cnt = 0
    
    # for subset in subsets:
    #     file_dir = output_file+"Init_"+str(cnt)+".txt"
    #     write_to_file(file_dir, "\n".join([f"{var}" for var in subset]))
    #     cnt += 1
    #     # print(subset)
    #     if(cnt>=100):
    #         break

    write_to_file(output_file+"All.txt", "\n".join([f"{var}" for var in variables]))
    # write_to_file(output_file, "\n".join([f"{var}" for var in variables]))
    
    
