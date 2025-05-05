def process_lines(input_text):
    processed_lines = []
    for line in input_text.strip().splitlines():
        line = line.strip()
        if line.startswith("always"):
            line = line[len("always"):].strip()
        line = line.replace("top::", "")
        processed_lines.append(line)
    return processed_lines


if __name__ == "__main__":
    input_file = "result.txt"
    output_file = "assertions.txt"

    with open(input_file, "r") as f:
        input_text = f.read()

    output_lines = process_lines(input_text)

    with open(output_file, "w") as f:
        for line in output_lines:
            f.write(line + "\n")

    print(f"Write to {output_file}")
