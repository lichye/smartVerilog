import re
import sys

def extract_variables(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    variable_pattern = r'\b(input|output|inout|reg|wire)(\s+reg)?\s*(\[[^\]]+\])?\s+(\w+)\s*'
    matches = re.finditer(variable_pattern, content)

    parsed_variables = []

    for match in matches:
        var_type = match.group(1)
        reg_type = match.group(2) if match.group(2) else ""
        bit_range = match.group(3) if match.group(3) else ""
        var_names = match.group(4)

        full_type = f"{var_type}{reg_type}".strip()
        var_name_list = [name.strip().rstrip(",") for name in var_names.split(",")] 
        for var_name in var_name_list:
            parsed_variables.append((full_type, bit_range, var_name))

    print(f"Extracted variables: {parsed_variables}") 
    return parsed_variables

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

def insert_copy_variables_and_always_block(file_path, copy_lines, always_block, output_file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    insert_index = 0
    for i, line in enumerate(lines):
        if line.strip().startswith(");"):
            insert_index = i + 1
            break

    modified_lines = lines[:insert_index] + copy_lines + ["\n"] + lines[insert_index:]

    endmodule_index = len(modified_lines)
    for i, line in enumerate(modified_lines):
        if line.strip() == "endmodule":
            endmodule_index = i
            break

    modified_lines = modified_lines[:endmodule_index] + ["\n"] + always_block + ["\n"] + modified_lines[endmodule_index:]

    with open(output_file_path, 'w') as file:
        file.writelines(modified_lines)

    print(f"Generated file with copy variables and always block: {output_file_path}")

if __name__ == "__main__":
    if(len(sys.argv) ==3):
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    else:
        input_file = "User/addsub.sv"
        output_file = "runtime/verilog/addsub.sv"
    

    # we get all the varibles from the input file
    variables = extract_variables(input_file)
    copy_lines = generate_copy_variables(variables)
    always_block = generate_always_block(variables)
    insert_copy_variables_and_always_block(input_file, copy_lines, always_block, output_file)
