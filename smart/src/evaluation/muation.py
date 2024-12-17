import re
import os
import random

class VerilogMutation:
    def __init__(self, input_file, output_dir):
        self.input_file = input_file
        self.output_dir = output_dir
        self.code_lines = []
        self.mutations = []
        self.always_block = False  # 状态机：是否在 always 块内

    def load_verilog(self):
        with open(self.input_file, 'r') as file:
            self.code_lines = file.readlines()

    def define_mutations(self):
        # operation sets
        # self.arithmetic_ops = ["\+", "-", "*", "/", "%"]
        # self.relational_ops = ["==", "!=", ">", "<", ">=", "<="]
        # self.logical_ops = ["\&\&", "\|\|"]
        # self.bitwise_ops = ["\&", "\|", "\^"]
        # self.constants = ["1'b0", "1'b1"]

        self.bitwise_ops = {"&": r"\&", "|": r"\|", "^": r"\^"}

        # define muation operations
        self.mutations = []
        
        self.mutations.append(
            {
                "category": "logical_negation",
                "pattern": r"(?<![a-zA-Z_])\b(0|1)\b(?![a-zA-Z_])",  # 独立的 0 或 1
                "replacement": lambda m: "1" if m.group(1) == "0" else "0"  # 0 → 1, 1 → 0
            }
        )

        for op, escaped_op in self.bitwise_ops.items():
            self.mutations.append({
                "category": "bitwise",
                "pattern": escaped_op,
                "replacement": random.choice([y for y in self.bitwise_ops.keys() if y != op])
            })

        # logical negation muation
        self.mutations.append({
            "category": "variable_negation",
            "pattern": r"(?<![!~])\b([a-zA-Z_]\w*)\b", 
            "replacement": lambda m: random.choice([f"!{m.group(1)}", f"~{m.group(1)}"])
        })

        self.mutations.append({
            "category": "negation_flip",
            "pattern": r"(?<![a-zA-Z0-9_])([!~])([a-zA-Z_]\w*)\b", 
            "replacement": lambda m: f"{m.group(2)}"
        })

        print(f"Generated {len(self.mutations)} mutation rules.")

    def is_in_always_block(self, line):
        """
        精确检测 always 块的开始和结束，基于 begin-end 配对。
        """
        # 检查 always 块的开始
        if re.search(r"^\s*always\b", line):
            self.always_block = True
            self.begin_count = 1  # 初始化 begin-end 计数器
            return

        # 在 always 块中，检查 begin 和 end
        if self.always_block:
            if re.search(r"\bbegin\b", line):  # 检测 begin
                self.begin_count += 1
            if re.search(r"\bend\b", line):    # 检测 end
                self.begin_count -= 1
            
            # 如果 begin-end 配对完成，退出 always 块
            if self.begin_count == 0:
                self.always_block = False

    def write_to_file(self, mutation_count,line,modified_line):
        file_dir = self.output_dir+"/mutant_"+str(mutation_count)+".sv"
        with open(file_dir, 'w') as file:
            for self_line in self.code_lines:
                if self_line == line:
                    file.write(modified_line)
                else:
                    file.write(self_line)

    def generate_mutants(self):
        """生成变异文件"""
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

        # we should loop over each line
        # loop over each mutation rule
        # if a line and a rule match, apply the mutation, make a new file

        self.always_block = False 
        
        test_count = 0

        for line in self.code_lines:
            mutated_lines = []

            self.is_in_always_block(line)  # update always block status
            
            
            # when we are in always block, we should apply the mutation to the right side of the assignment
            if self.always_block:
                # print("In always block: "+line)
                assignment = re.search(r"^\s*([a-zA-Z_]\w*)\s*(<{0,1}=)\s*(.+?);", line)
                # print("assignment: "+str(assignment)+"\n")
                for muation in self.mutations:
                    if assignment:
                        lhs = assignment.group(1)
                        op = assignment.group(2)
                        rhs = assignment.group(3)
                        if len(re.findall(muation["pattern"], rhs))>0:
                            modified_rhs = re.sub(muation["pattern"], muation["replacement"], rhs)
                            test_count += 1
                            modified_line = lhs + " " + op + " " + modified_rhs + ";\n"
                            print("This is the "+str(test_count)+" mutation")
                            print("The pattern is: "+muation["pattern"])
                            print("Muations: "+line + " -> " + modified_line)
                            # then we should write the modified line to the file
                            self.write_to_file(test_count,line,modified_line)
                            
            

    def run(self):
        self.load_verilog()
        self.define_mutations()
        # print("Generating mutants...")
        for muation in self.mutations:
            print(muation)
        self.generate_mutants()

# 示例使用
if __name__ == "__main__":
    input_file = "arb2.sv"  # 输入的Verilog文件
    output_dir = "mutants"         # 输出目录
    mutation_tool = VerilogMutation(input_file, output_dir)
    mutation_tool.run()
