import re
import configparser

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

def extrace_interesting_signals(content):
    # Regular expression to match always blocks
    always_block_pattern = re.compile(r'always\s*@\s*\(.*?\)\s*begin(.*?)end', re.S)
    
    # Regular expression to find variable names (identifiers)
    variable_pattern = re.compile(r'\b([a-zA-Z_][a-zA-Z0-9_]*)\b')
    
    # Extract all always blocks
    always_blocks = always_block_pattern.findall(content)
    result = []
    
    for block in always_blocks:
        # Find all unique variable names in the block
        variables = set(variable_pattern.findall(block))
        
        # Filter out keywords (you can expand this list as needed)
        keywords = {'if', 'else', 'begin', 'end', 'case', 'default', 'assign', 'always', 'posedge', 'negedge', 'or'}
        variables = {var for var in variables if var not in keywords}
        result.append(variables)
        print(f"Interesting signals in always block: {variables}")
    return result;    
    
def generate_copy_variables(variables):
    copy_lines = []
    for _, bit_range, var_name in variables:
        bit_range_str = f"{bit_range} " if bit_range else ""
        copy_line = f"\treg {bit_range_str}{var_name}_copy;\n"
        copy_lines.append(copy_line)
    return copy_lines

def generate_always_block(variables, clk_signal="clk"):
    always_block = [f"\talways @(posedge {clk_signal}) begin\n"]
    for _, _, var_name in variables:
        assign_line = f"\t\t{var_name}_copy <= {var_name};\n"
        always_block.append(assign_line)
    always_block.append("\tend\n")
    return always_block

def insert_copy_variables_and_always_block(module, copy_lines, always_block):
    insert_index = 0
    for i, line in enumerate(module):
        if line.strip().startswith(");"):
            insert_index = i + 1
            break

    modified_lines = module[:insert_index] + copy_lines + ["\n"] + module[insert_index:]

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

def parse_always_block(module, signals):

    all_blocks = []  # List to hold variables for each always block

    return all_blocks

def write_to_ini(module_name,variables,signals):
    print("module_name: ",module_name)
    print("variables: ",variables)
    print("signals: ",signals)
    file_path = "User/config.ini"

    config = configparser.ConfigParser()

    for variable in variables:

        pick_signal =[]
        for signal in signals:
            if variable == signal[2]:
                pick_signal = signal
                break
        print("pick_signal: ",pick_signal)


        bit_range = pick_signal[1]

        if bit_range.startswith('[') and bit_range.endswith(']'):
            lindex, rindex = map(int, bit_range[1:-1].split(':'))
            signalType = 1
        else:
            lindex, rindex = -1, -1
            signalType = 0
       
        config[variable]={
            "moduleName":module_name,
            "signalName":variable,
            "signalType":signalType,
            "lindex":lindex,
            "rindex":rindex
        }
        variable_copy = variable + "_copy"
        config[variable_copy]={
            "moduleName":module_name,
            "signalName":variable_copy,
            "signalType":signalType,
            "lindex":lindex,
            "rindex":rindex
        }

    with open(file_path, 'w') as configfile:
        config.write(configfile)

    print(f"INI 文件已写入: {file_path}")



# Test the function
if  __name__ == "__main__":
    file_path = "User/addsub.sv"
    modules = split_modules(file_path)
    result_file_path = "test_modified.sv"

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
    
    # write_to_file(result_file_path, new_module)

    #we just fucus on the first module's first always block
    write_to_ini(interesting_varibales_collections[0]["module_name"],interesting_varibales_collections[0]["interesting"][0],interesting_varibales_collections[0]["signals"])
    # print(interesting_varibales_collections[0])