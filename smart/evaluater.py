
import os
import sys
import subprocess

def run_ebmc_on_verilog_files(directory, property,bound,ebmc_path="ebmc"):
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
        print("In the directory: ",directory)
        print("No matching Verilog files found in the specified directory.")
        return

    print(f"Found {len(verilog_files)} Verilog files. Starting to process...")

    error_files = []  # List to track files with errors

    for verilog_file in verilog_files:
        # print(f"Running ebmc on: {verilog_file}")
        try:
            # Run ebmc command
            result = subprocess.run([ebmc_path, verilog_file,"-p",property,"--bound",str(bound)],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    text=True)
            
            # Print the output of the command
            # print()
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

def get_result_files(directory):

    result_files = set()
    
    if not os.path.exists(directory):
        raise FileNotFoundError(f"Directory '{directory}' does not exist.")

    for filename in os.listdir(directory):
        full_path = os.path.join(directory, filename)
        if filename.startswith("result") and os.path.isfile(full_path):
            result_files.add(full_path)

    return result_files

def get_mutant_files(directory):
    
    mutant_files = set()
        
    if not os.path.exists(directory):
        raise FileNotFoundError(f"Directory '{directory}' does not exist.")
    
    for filename in os.listdir(directory):
        full_path = os.path.join(directory, filename)
        if filename.startswith("mutant") and os.path.isfile(full_path):
            mutant_files.add(full_path)
    return mutant_files

def read_file(file):
    with open(file, "r") as file:
        content = file.read()
    return content

if __name__ == "__main__":
    property_dir = "result"
    bound = 10
    ebmc_path = "ebmc"
    directory = "benchmarks"
    
    properties = []
    property_files = get_result_files(property_dir)
    for file in property_files:
        read = read_file(file)
        properties.append(read)
    print("Properties: ",properties)

    find_files = set()
    for p in properties:
        print("Property: ",p)
        p_find_files = run_ebmc_on_verilog_files(directory, p,bound,ebmc_path)
        for file in p_find_files:
            find_files.add(file)

    print("Found mutations: ",sorted(find_files))
    print("Found total mutations: ",len(find_files))
    print("UnFound mutations: ",len(get_mutant_files(directory))-len(find_files))
    
    mutant_files = get_mutant_files(directory)
    print("Total mutations: ",len(mutant_files))
    print("Coverage percentage: ",(len(find_files)/len(mutant_files))*100)
    