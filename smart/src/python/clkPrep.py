import re
import configparser
import sys

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
            module_content = match.group(0)  # Extract full module content
            modules.append({"id": module_id, "content": module_content})
        
        return modules
    
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return []

def extract_signals(content):
    variable_pattern = r'\b(input|output|inout|reg|wire)(\s+reg)?\s*(\[[^\]]+\])?\s+(\w+)\s*'
    matches = re.finditer(variable_pattern, content)
    
    parsed_signals = set()

    for match in matches:
        var_type = match.group(1)
        reg_type = match.group(2) if match.group(2) else ""
        bit_range = match.group(3) if match.group(3) else ""
        var_names = match.group(4)

        full_type = f"{var_type}{reg_type}".strip()
        var_name_list = [name.strip().rstrip(",") for name in var_names.split(",")] 
        for var_name in var_name_list:
            if var_name not in parsed_signals:
                parsed_signals.add((full_type, bit_range, var_name))

    print(f"Extracted variables: {parsed_signals}") 
    return parsed_signals
    
def generate_copy_variables(variables):
    print(f"Generating copy variables for: {variables}")
    copy_lines = []
    copy_lines.append("\n")
    exist_variables = set()
    for _, bit_range, var_name in variables:
        if var_name in exist_variables:
            continue
        bit_range_str = f"{bit_range} " if bit_range else ""
        copy_line = f"\treg {bit_range_str}{var_name}_copy;"
        copy_lines.append(copy_line)
        exist_variables.add(var_name)
    copy_lines.append("\n")
    return copy_lines

def generate_always_block(variables, clk_signal="clk"):
    always_block = [f"\talways @(posedge {clk_signal}) begin"]
    for _, _, var_name in variables:
        assign_line = f"\t\t{var_name}_copy <= {var_name};"
        always_block.append(assign_line)
    always_block.append("\tend\n")
    return always_block

def insert_copy_variables_and_always_block(module, copy_lines, always_block):
    insert_index = 0
    for i, line in enumerate(module):
        if line.strip().startswith(");"):
            insert_index = i + 1
            break

    if insert_index == 0:
        insert_index = insert_index + 1

    modified_lines = module[:insert_index] + copy_lines + module[insert_index:]

    endmodule_index = len(modified_lines)
    for i, line in enumerate(modified_lines):
        if line.strip() == "endmodule":
            endmodule_index = i
            break

    modified_lines = modified_lines[:endmodule_index] + ["\n"] + always_block + ["\n"] + modified_lines[endmodule_index:]

    return modified_lines
   
def write_to_file(file_path, modules):
    try:
       with open(file_path, 'w') as file:
        for module in modules:
            for line in module:
                file.write(line + '\n')
    except Exception as e:
        print(f"Error writing to file {file_path}: {e}")

# Test the function
if  __name__ == "__main__":
    
    if(len(sys.argv) ==3):
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    else:
        print("Should give verilog design path")
        print(len(sys.argv))
        input_file = "User/addsub.sv"
        output_file = "runtime/verilog/addsub.sv"

    modules = split_modules(input_file)

    new_module = []

    interesting_varibales_collections = []

    for module in modules:
        print("Module name: ", module['id'])
        # this will be a list of complex defined signals
        signals = extract_signals(module['content'])
        
        # this will be a list of list of variable names
        interesting_varibales = extrace_interesting_signals(module['content'])

        copy_signals = generate_copy_variables(signals)

        assign_always_block = generate_always_block(signals)

        modified_module_lines = insert_copy_variables_and_always_block(module['content'].split("\n"), copy_signals, assign_always_block)

        new_module.append(modified_module_lines)

        interesting_varibales_collections.append({"interesting":interesting_varibales,"signals":signals,"module_name":module['id']})
    
    write_to_file(output_file, new_module)

    # print(interesting_varibales_collections[0])