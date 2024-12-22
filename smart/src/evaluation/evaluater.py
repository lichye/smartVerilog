
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

if __name__ == "__main__":
    # Example usage
    directory = "smart/User"
    property = "G ! (req1 & req2)"
    bound = 10
    ebmc_path = "ebmc"
    error_files = run_ebmc_on_verilog_files(directory, property,bound, ebmc_path)
    if error_files:
        print(f"Errors occurred while running ebmc on the following files:")
        for file in error_files:
            print(file)
    else:
        print("All files processed successfully.")