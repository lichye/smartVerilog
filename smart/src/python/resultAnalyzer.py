import os
import re
import sys

def parse_variables_from_file(file_path):
    """
    Parse variables from a given file.
    Assumes variables are defined in the form of `variable_name = value`.
    """
    variables = set()
    try:
        with open(file_path, 'r') as file:
            for line in file:
                # print(f"line: {line}")
                # Use regex to match variable definitions (e.g., variable_name = value)
                matches = re.findall(r"(?<![\d0-9'])\b[a-zA-Z_]\w*\b", line)
                variables.update(matches)
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
    return variables

def parse_variables_from_directory(directory):
    """
    Read all files in a directory and parse variables from them.
    """
    all_variables = {}

    if not os.path.exists(directory):
        print(f"Directory {directory} does not exist.")
        return all_variables

    for root, _, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            print(f"Parsing file: {file_path}")
            variables = parse_variables_from_file(file_path)
            all_variables[file_path] = variables

    return all_variables

def write_to_file(file_path, data):
    """
    Write the data to a file.
    """
    with open(file_path, 'w') as file:
        for key, value in data.items():
            # file.write(f"File: {key}\n")
            for variable in value:
                file.write(f"{variable}\n")
            # file.write("\n")

if __name__ == "__main__":
    # Specify the directory to parse
    if(len(sys.argv) != 3):
        print("Usage: python resultAnalyzer.py <directory_to_parse> <result_file>")
        sys.exit(1)
    else:
        directory_to_parse = sys.argv[1]
        result_file = sys.argv[2]

    # Parse variables from the directory
    files_results = parse_variables_from_directory(directory_to_parse)
    print(f"Result: {files_results}")
    write_to_file(result_file, files_results)
