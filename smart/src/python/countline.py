import sys
def countline(input_path):
    count = 0
    with open(input_path, 'r') as infile:
        for line in infile:
            count= count + 1
    return count


if __name__ == "__main__":
    if(len(sys.argv) != 2):
        print("Usage: python Sygus2Verilog.py <input_file>")
        sys.exit(1)
    else:
        input_file = sys.argv[1]
    # Example usage
    number = countline(input_file)
    print("Number of lines in the file:", number)