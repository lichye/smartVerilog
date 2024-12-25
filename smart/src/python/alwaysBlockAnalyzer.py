import re
import configparser

def write_to_ini(module_name,variables,signals):
    print("module_name: ",module_name)
    print("variables: ",variables)
    print("signals: ",signals)
    file_path = "user/config.ini"

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
    
    print(f"INI 文件已写入: {file_path}")