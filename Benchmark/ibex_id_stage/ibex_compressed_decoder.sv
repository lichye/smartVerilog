module ibex_compressed_decoder (
	instr_i,
	instr_o,
	is_compressed_o,
	illegal_instr_o
);
	input wire [31:0] instr_i;
	output reg [31:0] instr_o;
	output wire is_compressed_o;
	output reg illegal_instr_o;
	`include "ibex_pkg.sv"
	always @(*) begin
		illegal_instr_o = 1'b0;
		instr_o = 1'bX;
		case (instr_i[1:0])
			2'b00:
				case (instr_i[15:13])
					3'b000: begin
						instr_o = {2'b0, instr_i[10:7], instr_i[12:11], instr_i[5], instr_i[6], 2'b00, 5'h02, 3'b000, 2'b01, instr_i[4:2], OPCODE_OP_IMM};
						if ((instr_i[12:5] == 8'b0))
							illegal_instr_o = 1'b1;
					end
					3'b010: instr_o = {5'b0, instr_i[5], instr_i[12:10], instr_i[6], 2'b00, 2'b01, instr_i[9:7], 3'b010, 2'b01, instr_i[4:2], OPCODE_LOAD};
					3'b110: instr_o = {5'b0, instr_i[5], instr_i[12], 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b010, instr_i[11:10], instr_i[6], 2'b00, OPCODE_STORE};
					default: illegal_instr_o = 1'b1;
				endcase
			2'b01:
				case (instr_i[15:13])
					3'b000: instr_o = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2], instr_i[11:7], 3'b0, instr_i[11:7], OPCODE_OP_IMM};
					3'b001, 3'b101: instr_o = {instr_i[12], instr_i[8], instr_i[10:9], instr_i[6], instr_i[7], instr_i[2], instr_i[11], instr_i[5:3], {9 {instr_i[12]}}, 4'b0, ~instr_i[15], OPCODE_JAL};
					3'b010: instr_o = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2], 5'b0, 3'b0, instr_i[11:7], OPCODE_OP_IMM};
					3'b011: begin
						instr_o = {{15 {instr_i[12]}}, instr_i[6:2], instr_i[11:7], OPCODE_LUI};
						if ((instr_i[11:7] == 5'h02))
							instr_o = {{3 {instr_i[12]}}, instr_i[4:3], instr_i[5], instr_i[2], instr_i[6], 4'b0, 5'h02, 3'b000, 5'h02, OPCODE_OP_IMM};
						if (({instr_i[12], instr_i[6:2]} == 6'b0))
							illegal_instr_o = 1'b1;
					end
					3'b100:
						case (instr_i[11:10])
							2'b00, 2'b01: begin
								instr_o = {1'b0, instr_i[10], 5'b0, instr_i[6:2], 2'b01, instr_i[9:7], 3'b101, 2'b01, instr_i[9:7], OPCODE_OP_IMM};
								if ((instr_i[12] == 1'b1))
									illegal_instr_o = 1'b1;
							end
							2'b10: instr_o = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2], 2'b01, instr_i[9:7], 3'b111, 2'b01, instr_i[9:7], OPCODE_OP_IMM};
							2'b11:
								case ({instr_i[12], instr_i[6:5]})
									3'b000: instr_o = {2'b01, 5'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b000, 2'b01, instr_i[9:7], OPCODE_OP};
									3'b001: instr_o = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b100, 2'b01, instr_i[9:7], OPCODE_OP};
									3'b010: instr_o = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b110, 2'b01, instr_i[9:7], OPCODE_OP};
									3'b011: instr_o = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b111, 2'b01, instr_i[9:7], OPCODE_OP};
									3'b100, 3'b101, 3'b110, 3'b111: illegal_instr_o = 1'b1;
									default: illegal_instr_o = 1'bX;
								endcase
							default: illegal_instr_o = 1'bX;
						endcase
					3'b110, 3'b111: instr_o = {{4 {instr_i[12]}}, instr_i[6:5], instr_i[2], 5'b0, 2'b01, instr_i[9:7], 2'b00, instr_i[13], instr_i[11:10], instr_i[4:3], instr_i[12], OPCODE_BRANCH};
					default: illegal_instr_o = 1'bX;
				endcase
			2'b10:
				case (instr_i[15:13])
					3'b000: begin
						instr_o = {7'b0, instr_i[6:2], instr_i[11:7], 3'b001, instr_i[11:7], OPCODE_OP_IMM};
						if ((instr_i[12] == 1'b1))
							illegal_instr_o = 1'b1;
					end
					3'b010: begin
						instr_o = {4'b0, instr_i[3:2], instr_i[12], instr_i[6:4], 2'b00, 5'h02, 3'b010, instr_i[11:7], OPCODE_LOAD};
						if ((instr_i[11:7] == 5'b0))
							illegal_instr_o = 1'b1;
					end
					3'b100:
						if ((instr_i[12] == 1'b0)) begin
							if ((instr_i[6:2] != 5'b0))
								instr_o = {7'b0, instr_i[6:2], 5'b0, 3'b0, instr_i[11:7], OPCODE_OP};
							else begin
								instr_o = {12'b0, instr_i[11:7], 3'b0, 5'b0, OPCODE_JALR};
								if ((instr_i[11:7] == 5'b0))
									illegal_instr_o = 1'b1;
							end
						end
						else if ((instr_i[6:2] != 5'b0))
							instr_o = {7'b0, instr_i[6:2], instr_i[11:7], 3'b0, instr_i[11:7], OPCODE_OP};
						else if ((instr_i[11:7] == 5'b0))
							instr_o = 32'h00_10_00_73;
						else
							instr_o = {12'b0, instr_i[11:7], 3'b000, 5'b00001, OPCODE_JALR};
					3'b110: instr_o = {4'b0, instr_i[8:7], instr_i[12], instr_i[6:2], 5'h02, 3'b010, instr_i[11:9], 2'b00, OPCODE_STORE};
					3'b001, 3'b011, 3'b101, 3'b111: illegal_instr_o = 1'b1;
					default: illegal_instr_o = 1'bX;
				endcase
			default: instr_o = instr_i;
		endcase
	end
	assign is_compressed_o = (instr_i[1:0] != 2'b11);
endmodule
