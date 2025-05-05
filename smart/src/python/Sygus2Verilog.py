import re
import sys

def parse_expr(expr):
    tokens = re.findall(r'\(|\)|[^\s()]+', expr)

    def parse(tokens):
        token = tokens.pop(0)
        if token == '(':
            op = tokens.pop(0)
            args = []
            while tokens[0] != ')':
                args.append(parse(tokens))
            tokens.pop(0)  # remove ')'
            return (op, args)
        else:
            return token

    return parse(tokens)

def translate(expr):
    if isinstance(expr, str):
        return expr

    op, args = expr
    if op == 'not':
        return f'(! {translate(args[0])})'
    elif op == 'and':
        return f'({translate(args[0])} && {translate(args[1])})'
    elif op == 'or':
        return f'({translate(args[0])} || {translate(args[1])})'
    elif op == '=':
        return f'({translate(args[0])} == {translate(args[1])})'
    else:
        return f'{op}({", ".join(map(translate, args))})'

def translate_file(input_path, output_path):
    with open(input_path, 'r') as infile, open(output_path, 'w') as outfile:
        for line in infile:
            line = line.strip()
            if line:  # skip empty lines
                parsed = parse_expr(line)
                translated = translate(parsed)
                outfile.write(translated + '\n')


if __name__ == "__main__":
    if(len(sys.argv) != 3):
        print("Usage: python Sygus2Verilog.py <input_file> <output_file>")
        sys.exit(1)
    else:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    # Example usage
    translate_file(input_file, output_file)
