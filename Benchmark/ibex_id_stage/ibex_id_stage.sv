module ibex_register_file (
	clk_i,
	rst_ni,
	test_en_i,
	raddr_a_i,
	rdata_a_o,
	raddr_b_i,
	rdata_b_o,
	waddr_a_i,
	wdata_a_i,
	we_a_i
);
	parameter RV32E = 0;
	parameter [31:0] DataWidth = 32;
	input wire clk_i;
	input wire rst_ni;
	input wire test_en_i;
	input wire [4:0] raddr_a_i;
	/*output wire [(DataWidth - 1):0] rdata_a_o;*/
	output wire [31:0] rdata_a_o;
	input wire [4:0] raddr_b_i;
	/*output wire [(DataWidth - 1):0] rdata_b_o;*/
	output wire [31:0] rdata_b_o;
	input wire [4:0] waddr_a_i;
	/*input wire [(DataWidth - 1):0] wdata_a_i;*/
	input wire [31:0] wdata_a_i;
	input wire we_a_i;
	localparam [31:0] ADDR_WIDTH = (RV32E ? 4 : 5);
	localparam [31:0] NUM_WORDS = (2 ** ADDR_WIDTH);
    
    /*
	wire [(((NUM_WORDS - 1) >= 0) ? (((DataWidth - 1) >= 0) ? (((((NUM_WORDS - 1) >= 0) ? NUM_WORDS : (2 - NUM_WORDS)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) + -1) : (((((NUM_WORDS - 1) >= 0) ? NUM_WORDS : (2 - NUM_WORDS)) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)) + ((DataWidth - 1) - 1))) : (((DataWidth - 1) >= 0) ? ((((0 >= (NUM_WORDS - 1)) ? (2 - NUM_WORDS) : NUM_WORDS) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) + (((NUM_WORDS - 1) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) - 1)) : ((((0 >= (NUM_WORDS - 1)) ? (2 - NUM_WORDS) : NUM_WORDS) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)) + (((DataWidth - 1) + ((NUM_WORDS - 1) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth))) - 1)))):(((NUM_WORDS - 1) >= 0) ? (((DataWidth - 1) >= 0) ? 0 : (DataWidth - 1)) : (((DataWidth - 1) >= 0) ? ((NUM_WORDS - 1) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) : ((DataWidth - 1) + ((NUM_WORDS - 1) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)))))] rf_reg;

	reg [(((NUM_WORDS - 1) >= 1) ? (((DataWidth - 1) >= 0) ? (((((NUM_WORDS - 1) >= 1) ? (NUM_WORDS - 1) : (3 - NUM_WORDS)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) + ((((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)) - 1)) : (((((NUM_WORDS - 1) >= 1) ? (NUM_WORDS - 1) : (3 - NUM_WORDS)) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)) + (((DataWidth - 1) + ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)) - 1))) : (((DataWidth - 1) >= 0) ? ((((1 >= (NUM_WORDS - 1)) ? (3 - NUM_WORDS) : (NUM_WORDS - 1)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) + (((NUM_WORDS - 1) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) - 1)) : ((((1 >= (NUM_WORDS - 1)) ? (3 - NUM_WORDS) : (NUM_WORDS - 1)) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)) + (((DataWidth - 1) + ((NUM_WORDS - 1) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth))) - 1)))):(((NUM_WORDS - 1) >= 1) ? (((DataWidth - 1) >= 0) ? (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)) : ((DataWidth - 1) + ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth))) : (((DataWidth - 1) >= 0) ? ((NUM_WORDS - 1) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))) : ((DataWidth - 1) + ((NUM_WORDS - 1) * ((0 >= (DataWidth - 1)) ? (2 - DataWidth) : DataWidth)))))] rf_reg_tmp;
    */

    wire [1023:0] rf_reg;
    reg [1023:32] rf_reg_tmp;
	reg [31:1] we_a_dec;

   	always @(*) begin : we_a_decoder
		begin : sv2v_autoblock_2
			reg [31:0] i;
			for (i = 1; (i < NUM_WORDS); i = (i + 1))
				we_a_dec[i] = ((waddr_a_i == sv2v_cast_5(i)) ? we_a_i : 1'b0);
		end
	end
	always @(posedge clk_i or negedge rst_ni)
		if (!rst_ni)
			rf_reg_tmp <= {(((NUM_WORDS - 1) >= 1) ? (NUM_WORDS - 1) : (3 - NUM_WORDS)) {1'sb0}};
		else begin : sv2v_autoblock_3
			reg signed [31:0] r;
			for (r = 1; (r < NUM_WORDS); r = (r + 1))
				if (we_a_dec[r])
					rf_reg_tmp[((((DataWidth - 1) >= 0) ? 0 : (DataWidth - 1)) + ((((NUM_WORDS - 1) >= 1) ? r : (1 - (r - (NUM_WORDS - 1)))) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))))+:(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))] <= wdata_a_i;
		end
	assign rf_reg[((((DataWidth - 1) >= 0) ? 0 : (DataWidth - 1)) + ((((NUM_WORDS - 1) >= 0) ? 0 : (NUM_WORDS - 1)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))))+:(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))] = 1'sb0;
	assign rf_reg[(((((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)) + ((((NUM_WORDS - 1) >= 1) ? (NUM_WORDS - 1) : (3 - NUM_WORDS)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)))) - 1):(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))] = rf_reg_tmp[(((((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)) + ((((NUM_WORDS - 1) >= 1) ? (NUM_WORDS - 1) : (3 - NUM_WORDS)) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth)))) - 1):(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))];
	assign rdata_a_o = rf_reg[((((DataWidth - 1) >= 0) ? 0 : (DataWidth - 1)) + ((((NUM_WORDS - 1) >= 0) ? raddr_a_i : (0 - (raddr_a_i - (NUM_WORDS - 1)))) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))))+:(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))];
	assign rdata_b_o = rf_reg[((((DataWidth - 1) >= 0) ? 0 : (DataWidth - 1)) + ((((NUM_WORDS - 1) >= 0) ? raddr_b_i : (0 - (raddr_b_i - (NUM_WORDS - 1)))) * (((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))))+:(((DataWidth - 1) >= 0) ? DataWidth : (2 - DataWidth))];
	function [4:0] sv2v_cast_5;
		input [4:0] inp;
		sv2v_cast_5 = inp;
	endfunction
endmodule

module ibex_controller (
	clk_i,
	rst_ni,
	fetch_enable_i,
	ctrl_busy_o,
	illegal_insn_i,
	ecall_insn_i,
	mret_insn_i,
	dret_insn_i,
	wfi_insn_i,
	ebrk_insn_i,
	csr_pipe_flush_i,
	instr_valid_i,
	instr_i,
	instr_compressed_i,
	instr_is_compressed_i,
	instr_fetch_err_i,
	pc_id_i,
	instr_valid_clear_o,
	id_in_ready_o,
	instr_req_o,
	pc_set_o,
	pc_mux_o,
	exc_pc_mux_o,
	exc_cause_o,
	lsu_addr_last_i,
	load_err_i,
	store_err_i,
	branch_set_i,
	jump_set_i,
	csr_mstatus_mie_i,
	csr_msip_i,
	csr_mtip_i,
	csr_meip_i,
	csr_mfip_i,
	irq_pending_i,
	irq_nm_i,
	debug_req_i,
	debug_cause_o,
	debug_csr_save_o,
	debug_mode_o,
	debug_single_step_i,
	debug_ebreakm_i,
	debug_ebreaku_i,
	csr_save_if_o,
	csr_save_id_o,
	csr_restore_mret_id_o,
	csr_restore_dret_id_o,
	csr_save_cause_o,
	csr_mtval_o,
	priv_mode_i,
	csr_mstatus_tw_i,
	stall_lsu_i,
	stall_multdiv_i,
	stall_jump_i,
	stall_branch_i,
	perf_jump_o,
	perf_tbranch_o
);
	localparam [3:0] RESET = 0;
	localparam [3:0] BOOT_SET = 1;
	localparam [3:0] WAIT_SLEEP = 2;
	localparam [3:0] SLEEP = 3;
	localparam [3:0] FIRST_FETCH = 4;
	localparam [3:0] DECODE = 5;
	localparam [3:0] FLUSH = 6;
	localparam [3:0] IRQ_TAKEN = 7;
	localparam [3:0] DBG_TAKEN_IF = 8;
	localparam [3:0] DBG_TAKEN_ID = 9;
	input wire clk_i;
	input wire rst_ni;
	input wire fetch_enable_i;
	output reg ctrl_busy_o;
	input wire illegal_insn_i;
	input wire ecall_insn_i;
	input wire mret_insn_i;
	input wire dret_insn_i;
	input wire wfi_insn_i;
	input wire ebrk_insn_i;
	input wire csr_pipe_flush_i;
	input wire instr_valid_i;
	input wire [31:0] instr_i;
	input wire [15:0] instr_compressed_i;
	input wire instr_is_compressed_i;
	input wire instr_fetch_err_i;
	input wire [31:0] pc_id_i;
	output wire instr_valid_clear_o;
	output wire id_in_ready_o;
	output reg instr_req_o;
	output reg pc_set_o;
	output reg [2:0] pc_mux_o;
	output reg [1:0] exc_pc_mux_o;
	output reg [5:0] exc_cause_o;
	input wire [31:0] lsu_addr_last_i;
	input wire load_err_i;
	input wire store_err_i;
	input wire branch_set_i;
	input wire jump_set_i;
	input wire csr_mstatus_mie_i;
	input wire csr_msip_i;
	input wire csr_mtip_i;
	input wire csr_meip_i;
	input wire [14:0] csr_mfip_i;
	input wire irq_pending_i;
	input wire irq_nm_i;
	input wire debug_req_i;
	output reg [2:0] debug_cause_o;
	output reg debug_csr_save_o;
	output wire debug_mode_o;
	input wire debug_single_step_i;
	input wire debug_ebreakm_i;
	input wire debug_ebreaku_i;
	output reg csr_save_if_o;
	output reg csr_save_id_o;
	output reg csr_restore_mret_id_o;
	output reg csr_restore_dret_id_o;
	output reg csr_save_cause_o;
	output reg [31:0] csr_mtval_o;
	input wire [1:0] priv_mode_i;
	input wire csr_mstatus_tw_i;
	input wire stall_lsu_i;
	input wire stall_multdiv_i;
	input wire stall_jump_i;
	input wire stall_branch_i;
	output reg perf_jump_o;
	output reg perf_tbranch_o;
	`include "ibex_pkg.sv"
	reg [3:0] ctrl_fsm_cs;
	reg [3:0] ctrl_fsm_ns;
	reg nmi_mode_q;
	reg nmi_mode_d;
	reg debug_mode_q;
	reg debug_mode_d;
	reg load_err_q;
	wire load_err_d;
	reg store_err_q;
	wire store_err_d;
	reg exc_req_q;
	wire exc_req_d;
	reg illegal_insn_q;
	wire illegal_insn_d;
	wire stall;
	reg halt_if;
	reg flush_id;
	wire illegal_dret;
	wire illegal_umode;
	wire exc_req_lsu;
	wire special_req;
	wire enter_debug_mode;
	wire ebreak_into_debug;
	wire handle_irq;
	reg [3:0] mfip_id;
	wire unused_csr_mtip;
	wire ecall_insn;
	wire mret_insn;
	wire dret_insn;
	wire wfi_insn;
	wire ebrk_insn;
	wire csr_pipe_flush;
	wire instr_fetch_err;
	/* 
	NO $display allowed in Verilog code that will go thorugh GM static
	analysis
	always @(negedge clk_i)
		if (((((ctrl_fsm_cs == DECODE) && instr_valid_i) && !instr_fetch_err_i) && illegal_insn_d))
			$display("%t: Illegal instruction (hart %0x) at PC 0x%h: 0x%h", $time, ibex_core.hart_id_i, ibex_id_stage.pc_id_i, ibex_id_stage.instr_rdata_i);
	*/
	assign load_err_d = load_err_i;
	assign store_err_d = store_err_i;
	assign ecall_insn = (ecall_insn_i & instr_valid_i);
	assign mret_insn = (mret_insn_i & instr_valid_i);
	assign dret_insn = (dret_insn_i & instr_valid_i);
	assign wfi_insn = (wfi_insn_i & instr_valid_i);
	assign ebrk_insn = (ebrk_insn_i & instr_valid_i);
	assign csr_pipe_flush = (csr_pipe_flush_i & instr_valid_i);
	assign instr_fetch_err = (instr_fetch_err_i & instr_valid_i);
	assign illegal_dret = (dret_insn & ~debug_mode_q);
	assign illegal_umode = ((priv_mode_i != PRIV_LVL_M) & (mret_insn | (csr_mstatus_tw_i & wfi_insn)));
	assign illegal_insn_d = (((illegal_insn_i | illegal_dret) | illegal_umode) & (ctrl_fsm_cs != FLUSH));
	assign exc_req_d = ((((ecall_insn | ebrk_insn) | illegal_insn_d) | instr_fetch_err) & (ctrl_fsm_cs != FLUSH));
	assign exc_req_lsu = (store_err_i | load_err_i);
	assign special_req = (((((mret_insn | dret_insn) | wfi_insn) | csr_pipe_flush) | exc_req_d) | exc_req_lsu);
	assign enter_debug_mode = ((debug_req_i | (debug_single_step_i & instr_valid_i)) & ~debug_mode_q);
	assign ebreak_into_debug = ((priv_mode_i == PRIV_LVL_M) ? debug_ebreakm_i : ((priv_mode_i == PRIV_LVL_U) ? debug_ebreaku_i : 1'b0));
	assign handle_irq = (~debug_mode_q & ((irq_nm_i & ~nmi_mode_q) | (irq_pending_i & csr_mstatus_mie_i)));
	always @(*) begin : gen_mfip_id
		if (csr_mfip_i[14])
			mfip_id = 4'd14;
		else if (csr_mfip_i[13])
			mfip_id = 4'd13;
		else if (csr_mfip_i[12])
			mfip_id = 4'd12;
		else if (csr_mfip_i[11])
			mfip_id = 4'd11;
		else if (csr_mfip_i[10])
			mfip_id = 4'd10;
		else if (csr_mfip_i[9])
			mfip_id = 4'd9;
		else if (csr_mfip_i[8])
			mfip_id = 4'd8;
		else if (csr_mfip_i[7])
			mfip_id = 4'd7;
		else if (csr_mfip_i[6])
			mfip_id = 4'd6;
		else if (csr_mfip_i[5])
			mfip_id = 4'd5;
		else if (csr_mfip_i[5])
			mfip_id = 4'd5;
		else if (csr_mfip_i[4])
			mfip_id = 4'd4;
		else if (csr_mfip_i[3])
			mfip_id = 4'd3;
		else if (csr_mfip_i[2])
			mfip_id = 4'd2;
		else if (csr_mfip_i[1])
			mfip_id = 4'd1;
		else
			mfip_id = 4'd0;
	end
	assign unused_csr_mtip = csr_mtip_i;
	always @(*) begin
		instr_req_o = 1'b1;
		csr_save_if_o = 1'b0;
		csr_save_id_o = 1'b0;
		csr_restore_mret_id_o = 1'b0;
		csr_restore_dret_id_o = 1'b0;
		csr_save_cause_o = 1'b0;
		csr_mtval_o = 1'sb0;
		pc_mux_o = PC_BOOT;
		pc_set_o = 1'b0;
		exc_pc_mux_o = EXC_PC_IRQ;
		exc_cause_o = EXC_CAUSE_INSN_ADDR_MISA;
		ctrl_fsm_ns = ctrl_fsm_cs;
		ctrl_busy_o = 1'b1;
		halt_if = 1'b0;
		flush_id = 1'b0;
		debug_csr_save_o = 1'b0;
		debug_cause_o = DBG_CAUSE_EBREAK;
		debug_mode_d = debug_mode_q;
		nmi_mode_d = nmi_mode_q;
		perf_tbranch_o = 1'b0;
		perf_jump_o = 1'b0;
		case (ctrl_fsm_cs)
			RESET: begin
				instr_req_o = 1'b0;
				pc_mux_o = PC_BOOT;
				pc_set_o = 1'b1;
				if (fetch_enable_i)
					ctrl_fsm_ns = BOOT_SET;
			end
			BOOT_SET: begin
				instr_req_o = 1'b1;
				pc_mux_o = PC_BOOT;
				pc_set_o = 1'b1;
				ctrl_fsm_ns = FIRST_FETCH;
			end
			WAIT_SLEEP: begin
				ctrl_busy_o = 1'b0;
				instr_req_o = 1'b0;
				halt_if = 1'b1;
				flush_id = 1'b1;
				ctrl_fsm_ns = SLEEP;
			end
			SLEEP: begin
				instr_req_o = 1'b0;
				halt_if = 1'b1;
				flush_id = 1'b1;
				if (((((irq_nm_i || irq_pending_i) || debug_req_i) || debug_mode_q) || debug_single_step_i))
					ctrl_fsm_ns = FIRST_FETCH;
				else
					ctrl_busy_o = 1'b0;
			end
			FIRST_FETCH: begin
				if (id_in_ready_o)
					ctrl_fsm_ns = DECODE;
				if (handle_irq) begin
					ctrl_fsm_ns = IRQ_TAKEN;
					halt_if = 1'b1;
					flush_id = 1'b1;
				end
				if (enter_debug_mode) begin
					ctrl_fsm_ns = DBG_TAKEN_IF;
					halt_if = 1'b1;
				end
			end
			DECODE: begin
				if (instr_valid_i) begin
					if (special_req) begin
						ctrl_fsm_ns = FLUSH;
						halt_if = 1'b1;
					end
					else if ((branch_set_i || jump_set_i)) begin
						pc_mux_o = PC_JUMP;
						pc_set_o = 1'b1;
						perf_tbranch_o = branch_set_i;
						perf_jump_o = jump_set_i;
					end
					if (((enter_debug_mode || handle_irq) && stall))
						halt_if = 1'b1;
				end
				if ((!stall && !special_req))
					if (enter_debug_mode) begin
						ctrl_fsm_ns = DBG_TAKEN_IF;
						halt_if = 1'b1;
					end
					else if (handle_irq) begin
						ctrl_fsm_ns = IRQ_TAKEN;
						halt_if = 1'b1;
						flush_id = 1'b1;
					end
			end
			IRQ_TAKEN: begin
				if (handle_irq) begin
					pc_mux_o = PC_EXC;
					pc_set_o = 1'b1;
					exc_pc_mux_o = EXC_PC_IRQ;
					csr_save_if_o = 1'b1;
					csr_save_cause_o = 1'b1;
					if ((irq_nm_i && !nmi_mode_q)) begin
						exc_cause_o = EXC_CAUSE_IRQ_NM;
						nmi_mode_d = 1'b1;
					end
					else if ((csr_mfip_i != 15'b0))
						exc_cause_o = sv2v_cast_89EA8({2'b11, mfip_id});
					else if (csr_meip_i)
						exc_cause_o = EXC_CAUSE_IRQ_EXTERNAL_M;
					else if (csr_msip_i)
						exc_cause_o = EXC_CAUSE_IRQ_SOFTWARE_M;
					else
						exc_cause_o = EXC_CAUSE_IRQ_TIMER_M;
				end
				ctrl_fsm_ns = DECODE;
			end
			DBG_TAKEN_IF: begin
				if ((debug_single_step_i || debug_req_i)) begin
					flush_id = 1'b1;
					pc_mux_o = PC_EXC;
					pc_set_o = 1'b1;
					exc_pc_mux_o = EXC_PC_DBD;
					csr_save_if_o = 1'b1;
					debug_csr_save_o = 1'b1;
					csr_save_cause_o = 1'b1;
					if (debug_single_step_i)
						debug_cause_o = DBG_CAUSE_STEP;
					else
						debug_cause_o = DBG_CAUSE_HALTREQ;
					debug_mode_d = 1'b1;
				end
				ctrl_fsm_ns = DECODE;
			end
			DBG_TAKEN_ID: begin
				flush_id = 1'b1;
				pc_mux_o = PC_EXC;
				pc_set_o = 1'b1;
				exc_pc_mux_o = EXC_PC_DBD;
				if ((ebreak_into_debug && !debug_mode_q)) begin
					csr_save_cause_o = 1'b1;
					csr_save_id_o = 1'b1;
					debug_csr_save_o = 1'b1;
					debug_cause_o = DBG_CAUSE_EBREAK;
				end
				debug_mode_d = 1'b1;
				ctrl_fsm_ns = DECODE;
			end
			FLUSH: begin
				halt_if = 1'b1;
				flush_id = 1'b1;
				ctrl_fsm_ns = DECODE;
				if (((exc_req_q || store_err_q) || load_err_q)) begin
					pc_set_o = 1'b1;
					pc_mux_o = PC_EXC;
					exc_pc_mux_o = (debug_mode_q ? EXC_PC_DBG_EXC : EXC_PC_EXC);
					csr_save_id_o = 1'b1;
					csr_save_cause_o = 1'b1;
					if (instr_fetch_err) begin
						exc_cause_o = EXC_CAUSE_INSTR_ACCESS_FAULT;
						csr_mtval_o = pc_id_i;
					end
					else if (illegal_insn_q) begin
						exc_cause_o = EXC_CAUSE_ILLEGAL_INSN;
						csr_mtval_o = (instr_is_compressed_i ? {16'b0, instr_compressed_i} : instr_i);
					end
					else if (ecall_insn)
						exc_cause_o = ((priv_mode_i == PRIV_LVL_M) ? EXC_CAUSE_ECALL_MMODE : EXC_CAUSE_ECALL_UMODE);
					else if (ebrk_insn) begin
						if ((debug_mode_q | ebreak_into_debug)) begin
							pc_set_o = 1'b0;
							csr_save_id_o = 1'b0;
							csr_save_cause_o = 1'b0;
							ctrl_fsm_ns = DBG_TAKEN_ID;
							flush_id = 1'b0;
						end
						else
							exc_cause_o = EXC_CAUSE_BREAKPOINT;
					end
					else if (store_err_q) begin
						exc_cause_o = EXC_CAUSE_STORE_ACCESS_FAULT;
						csr_mtval_o = lsu_addr_last_i;
					end
					else begin
						exc_cause_o = EXC_CAUSE_LOAD_ACCESS_FAULT;
						csr_mtval_o = lsu_addr_last_i;
					end
				end
				else if (mret_insn) begin
					pc_mux_o = PC_ERET;
					pc_set_o = 1'b1;
					csr_restore_mret_id_o = 1'b1;
					if (nmi_mode_q)
						nmi_mode_d = 1'b0;
				end
				else if (dret_insn) begin
					pc_mux_o = PC_DRET;
					pc_set_o = 1'b1;
					debug_mode_d = 1'b0;
					csr_restore_dret_id_o = 1'b1;
				end
				else if (wfi_insn)
					ctrl_fsm_ns = WAIT_SLEEP;
				else if ((csr_pipe_flush && handle_irq))
					ctrl_fsm_ns = IRQ_TAKEN;
				if (enter_debug_mode)
					ctrl_fsm_ns = DBG_TAKEN_IF;
			end
			default: begin
			instr_req_o = 1'b0;
			ctrl_fsm_ns = 1'bX;
		end
		endcase
	end
	assign debug_mode_o = debug_mode_q;
	assign stall = (((stall_lsu_i | stall_multdiv_i) | stall_jump_i) | stall_branch_i);
	assign id_in_ready_o = (~stall & ~halt_if);
	assign instr_valid_clear_o = (~(stall | halt_if) | flush_id);
	always @(posedge clk_i or negedge rst_ni) begin : update_regs
		if (!rst_ni) begin
			ctrl_fsm_cs <= RESET;
			nmi_mode_q <= 1'b0;
			debug_mode_q <= 1'b0;
			load_err_q <= 1'b0;
			store_err_q <= 1'b0;
			exc_req_q <= 1'b0;
			illegal_insn_q <= 1'b0;
		end
		else begin
			ctrl_fsm_cs <= ctrl_fsm_ns;
			nmi_mode_q <= nmi_mode_d;
			debug_mode_q <= debug_mode_d;
			load_err_q <= load_err_d;
			store_err_q <= store_err_d;
			exc_req_q <= exc_req_d;
			illegal_insn_q <= illegal_insn_d;
		end
	end
	function [(6 - 1):0] sv2v_cast_89EA8;
		input [(6 - 1):0] inp;
		sv2v_cast_89EA8 = inp;
	endfunction
endmodule

module ibex_decoder (
	illegal_insn_o,
	ebrk_insn_o,
	mret_insn_o,
	dret_insn_o,
	ecall_insn_o,
	wfi_insn_o,
	jump_set_o,
	instr_new_i,
	instr_rdata_i,
	illegal_c_insn_i,
	imm_a_mux_sel_o,
	imm_b_mux_sel_o,
	imm_i_type_o,
	imm_s_type_o,
	imm_b_type_o,
	imm_u_type_o,
	imm_j_type_o,
	zimm_rs1_type_o,
	regfile_wdata_sel_o,
	regfile_we_o,
	regfile_raddr_a_o,
	regfile_raddr_b_o,
	regfile_waddr_o,
	alu_operator_o,
	alu_op_a_mux_sel_o,
	alu_op_b_mux_sel_o,
	mult_en_o,
	div_en_o,
	multdiv_operator_o,
	multdiv_signed_mode_o,
	csr_access_o,
	csr_op_o,
	csr_pipe_flush_o,
	data_req_o,
	data_we_o,
	data_type_o,
	data_sign_extension_o,
	jump_in_dec_o,
	branch_in_dec_o
);
	parameter RV32E = 0;
	parameter RV32M = 1;
	output wire illegal_insn_o;
	output reg ebrk_insn_o;
	output reg mret_insn_o;
	output reg dret_insn_o;
	output reg ecall_insn_o;
	output reg wfi_insn_o;
	output reg jump_set_o;
	input wire instr_new_i;
	input wire [31:0] instr_rdata_i;
	input wire illegal_c_insn_i;
	output reg imm_a_mux_sel_o;
	output reg [2:0] imm_b_mux_sel_o;
	output wire [31:0] imm_i_type_o;
	output wire [31:0] imm_s_type_o;
	output wire [31:0] imm_b_type_o;
	output wire [31:0] imm_u_type_o;
	output wire [31:0] imm_j_type_o;
	output wire [31:0] zimm_rs1_type_o;
	output reg [1:0] regfile_wdata_sel_o;
	output wire regfile_we_o;
	output wire [4:0] regfile_raddr_a_o;
	output wire [4:0] regfile_raddr_b_o;
	output wire [4:0] regfile_waddr_o;
	output reg [4:0] alu_operator_o;
	output reg [1:0] alu_op_a_mux_sel_o;
	output reg alu_op_b_mux_sel_o;
	output reg mult_en_o;
	output reg div_en_o;
	output reg [1:0] multdiv_operator_o;
	output reg [1:0] multdiv_signed_mode_o;
	output reg csr_access_o;
	output reg [1:0] csr_op_o;
	output reg csr_pipe_flush_o;
	output reg data_req_o;
	output reg data_we_o;
	output reg [1:0] data_type_o;
	output reg data_sign_extension_o;
	output reg jump_in_dec_o;
	output reg branch_in_dec_o;
	`include "ibex_pkg.sv"
	reg illegal_insn;
	wire illegal_reg_rv32e;
	reg csr_illegal;
	reg regfile_we;
	wire [31:0] instr;
	reg [1:0] csr_op;
	reg [6:0] opcode;
	assign instr = instr_rdata_i;
	assign imm_i_type_o = {{20 {instr[31]}}, instr[31:20]};
	assign imm_s_type_o = {{20 {instr[31]}}, instr[31:25], instr[11:7]};
	assign imm_b_type_o = {{19 {instr[31]}}, instr[31], instr[7], instr[30:25], instr[11:8], 1'b0};
	assign imm_u_type_o = {instr[31:12], 12'b0};
	assign imm_j_type_o = {{12 {instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};
	assign zimm_rs1_type_o = {27'b0, instr[19:15]};
	assign regfile_raddr_a_o = instr[19:15];
	assign regfile_raddr_b_o = instr[24:20];
	assign regfile_waddr_o = instr[11:07];
	generate
		if (RV32E) begin : gen_rv32e_reg_check_active
			assign illegal_reg_rv32e = (((regfile_raddr_a_o[4] & (alu_op_a_mux_sel_o == OP_A_REG_A)) | (regfile_raddr_b_o[4] & (alu_op_b_mux_sel_o == OP_B_REG_B))) | (regfile_waddr_o[4] & regfile_we));
		end
		else begin : gen_rv32e_reg_check_inactive
			assign illegal_reg_rv32e = 1'b0;
		end
	endgenerate
	always @(*) begin : csr_operand_check
		csr_op_o = csr_op;
		if ((((csr_op == CSR_OP_SET) || (csr_op == CSR_OP_CLEAR)) && (instr[19:15] == 1'sb0)))
			csr_op_o = CSR_OP_READ;
	end
	always @(*) begin : csr_pipeline_flushes
		csr_pipe_flush_o = 1'b0;
		if (((csr_access_o == 1'b1) && ((csr_op_o == CSR_OP_WRITE) || (csr_op_o == CSR_OP_SET)))) begin
			if (((sv2v_cast_290A1(instr[31:20]) == CSR_MSTATUS) || (sv2v_cast_290A1(instr[31:20]) == CSR_MIE)))
				csr_pipe_flush_o = 1'b1;
		end
		else if (((csr_access_o == 1'b1) && (csr_op_o != CSR_OP_READ)))
			if (((((sv2v_cast_290A1(instr[31:20]) == CSR_DCSR) || (sv2v_cast_290A1(instr[31:20]) == CSR_DPC)) || (sv2v_cast_290A1(instr[31:20]) == CSR_DSCRATCH0)) || (sv2v_cast_290A1(instr[31:20]) == CSR_DSCRATCH1)))
				csr_pipe_flush_o = 1'b1;
	end
	always @(*) begin
		jump_in_dec_o = 1'b0;
		jump_set_o = 1'b0;
		branch_in_dec_o = 1'b0;
		alu_operator_o = ALU_SLTU;
		alu_op_a_mux_sel_o = OP_A_IMM;
		alu_op_b_mux_sel_o = OP_B_IMM;
		imm_a_mux_sel_o = IMM_A_ZERO;
		imm_b_mux_sel_o = IMM_B_I;
		mult_en_o = 1'b0;
		div_en_o = 1'b0;
		multdiv_operator_o = MD_OP_MULL;
		multdiv_signed_mode_o = 2'b00;
		regfile_wdata_sel_o = RF_WD_EX;
		regfile_we = 1'b0;
		csr_access_o = 1'b0;
		csr_illegal = 1'b0;
		csr_op = CSR_OP_READ;
		data_we_o = 1'b0;
		data_type_o = 2'b00;
		data_sign_extension_o = 1'b0;
		data_req_o = 1'b0;
		illegal_insn = 1'b0;
		ebrk_insn_o = 1'b0;
		mret_insn_o = 1'b0;
		dret_insn_o = 1'b0;
		ecall_insn_o = 1'b0;
		wfi_insn_o = 1'b0;
		opcode = sv2v_cast_DB892(instr[6:0]);
		case (opcode)
			OPCODE_JAL: begin
				jump_in_dec_o = 1'b1;
				if (instr_new_i) begin
					alu_op_a_mux_sel_o = OP_A_CURRPC;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_b_mux_sel_o = IMM_B_J;
					alu_operator_o = ALU_ADD;
					regfile_we = 1'b0;
					jump_set_o = 1'b1;
				end
				else begin
					alu_op_a_mux_sel_o = OP_A_CURRPC;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_b_mux_sel_o = IMM_B_INCR_PC;
					alu_operator_o = ALU_ADD;
					regfile_we = 1'b1;
				end
			end
			OPCODE_JALR: begin
				jump_in_dec_o = 1'b1;
				if (instr_new_i) begin
					alu_op_a_mux_sel_o = OP_A_REG_A;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_b_mux_sel_o = IMM_B_I;
					alu_operator_o = ALU_ADD;
					regfile_we = 1'b0;
					jump_set_o = 1'b1;
				end
				else begin
					alu_op_a_mux_sel_o = OP_A_CURRPC;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_b_mux_sel_o = IMM_B_INCR_PC;
					alu_operator_o = ALU_ADD;
					regfile_we = 1'b1;
				end
				if ((instr[14:12] != 3'b0))
					illegal_insn = 1'b1;
			end
			OPCODE_BRANCH: begin
				branch_in_dec_o = 1'b1;
				case (instr[14:12])
					3'b000: alu_operator_o = ALU_EQ;
					3'b001: alu_operator_o = ALU_NE;
					3'b100: alu_operator_o = ALU_LT;
					3'b101: alu_operator_o = ALU_GE;
					3'b110: alu_operator_o = ALU_LTU;
					3'b111: alu_operator_o = ALU_GEU;
					default: illegal_insn = 1'b1;
				endcase
				if (instr_new_i) begin
					alu_op_a_mux_sel_o = OP_A_REG_A;
					alu_op_b_mux_sel_o = OP_B_REG_B;
				end
				else begin
					alu_op_a_mux_sel_o = OP_A_CURRPC;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_b_mux_sel_o = IMM_B_B;
					alu_operator_o = ALU_ADD;
					regfile_we = 1'b0;
				end
			end
			OPCODE_STORE: begin
				alu_op_a_mux_sel_o = OP_A_REG_A;
				alu_op_b_mux_sel_o = OP_B_REG_B;
				data_req_o = 1'b1;
				data_we_o = 1'b1;
				alu_operator_o = ALU_ADD;
				if (!instr[14]) begin
					imm_b_mux_sel_o = IMM_B_S;
					alu_op_b_mux_sel_o = OP_B_IMM;
				end
				else
					illegal_insn = 1'b1;
				case (instr[13:12])
					2'b00: data_type_o = 2'b10;
					2'b01: data_type_o = 2'b01;
					2'b10: data_type_o = 2'b00;
					default: illegal_insn = 1'b1;
				endcase
			end
			OPCODE_LOAD: begin
				alu_op_a_mux_sel_o = OP_A_REG_A;
				data_req_o = 1'b1;
				regfile_wdata_sel_o = RF_WD_LSU;
				regfile_we = 1'b1;
				data_type_o = 2'b00;
				alu_operator_o = ALU_ADD;
				alu_op_b_mux_sel_o = OP_B_IMM;
				imm_b_mux_sel_o = IMM_B_I;
				data_sign_extension_o = ~instr[14];
				case (instr[13:12])
					2'b00: data_type_o = 2'b10;
					2'b01: data_type_o = 2'b01;
					2'b10: begin
						data_type_o = 2'b00;
						if (instr[14])
							illegal_insn = 1'b1;
					end
					default: illegal_insn = 1'b1;
				endcase
			end
			OPCODE_LUI: begin
				alu_op_a_mux_sel_o = OP_A_IMM;
				alu_op_b_mux_sel_o = OP_B_IMM;
				imm_a_mux_sel_o = IMM_A_ZERO;
				imm_b_mux_sel_o = IMM_B_U;
				alu_operator_o = ALU_ADD;
				regfile_we = 1'b1;
			end
			OPCODE_AUIPC: begin
				alu_op_a_mux_sel_o = OP_A_CURRPC;
				alu_op_b_mux_sel_o = OP_B_IMM;
				imm_b_mux_sel_o = IMM_B_U;
				alu_operator_o = ALU_ADD;
				regfile_we = 1'b1;
			end
			OPCODE_OP_IMM: begin
				alu_op_a_mux_sel_o = OP_A_REG_A;
				alu_op_b_mux_sel_o = OP_B_IMM;
				imm_b_mux_sel_o = IMM_B_I;
				regfile_we = 1'b1;
				case (instr[14:12])
					3'b000: alu_operator_o = ALU_ADD;
					3'b010: alu_operator_o = ALU_SLT;
					3'b011: alu_operator_o = ALU_SLTU;
					3'b100: alu_operator_o = ALU_XOR;
					3'b110: alu_operator_o = ALU_OR;
					3'b111: alu_operator_o = ALU_AND;
					3'b001: begin
						alu_operator_o = ALU_SLL;
						if ((instr[31:25] != 7'b0))
							illegal_insn = 1'b1;
					end
					3'b101:
						if ((instr[31:25] == 7'b0))
							alu_operator_o = ALU_SRL;
						else if ((instr[31:25] == 7'b010_0000))
							alu_operator_o = ALU_SRA;
						else
							illegal_insn = 1'b1;
					default: alu_operator_o = sv2v_cast_F99D1({6 {1'bX}});
				endcase
			end
			OPCODE_OP: begin
				alu_op_a_mux_sel_o = OP_A_REG_A;
				alu_op_b_mux_sel_o = OP_B_REG_B;
				regfile_we = 1'b1;
				if (instr[31])
					illegal_insn = 1'b1;
				else
					case ({instr[30:25], instr[14:12]})
						{6'b00_0000, 3'b000}: alu_operator_o = ALU_ADD;
						{6'b10_0000, 3'b000}: alu_operator_o = ALU_SUB;
						{6'b00_0000, 3'b010}: alu_operator_o = ALU_SLT;
						{6'b00_0000, 3'b011}: alu_operator_o = ALU_SLTU;
						{6'b00_0000, 3'b100}: alu_operator_o = ALU_XOR;
						{6'b00_0000, 3'b110}: alu_operator_o = ALU_OR;
						{6'b00_0000, 3'b111}: alu_operator_o = ALU_AND;
						{6'b00_0000, 3'b001}: alu_operator_o = ALU_SLL;
						{6'b00_0000, 3'b101}: alu_operator_o = ALU_SRL;
						{6'b10_0000, 3'b101}: alu_operator_o = ALU_SRA;
						{6'b00_0001, 3'b000}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_MULL;
							mult_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b00;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b001}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_MULH;
							mult_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b11;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b010}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_MULH;
							mult_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b01;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b011}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_MULH;
							mult_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b00;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b100}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_DIV;
							div_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b11;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b101}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_DIV;
							div_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b00;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b110}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_REM;
							div_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b11;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						{6'b00_0001, 3'b111}: begin
							alu_operator_o = ALU_ADD;
							multdiv_operator_o = MD_OP_REM;
							div_en_o = (RV32M ? 1'b1 : 1'b0);
							multdiv_signed_mode_o = 2'b00;
							illegal_insn = (RV32M ? 1'b0 : 1'b1);
						end
						default: illegal_insn = 1'b1;
					endcase
			end
			OPCODE_MISC_MEM:
				case (instr[14:12])
					3'b000: begin
						alu_operator_o = ALU_ADD;
						alu_op_a_mux_sel_o = OP_A_REG_A;
						alu_op_b_mux_sel_o = OP_B_IMM;
						regfile_we = 1'b0;
					end
					3'b001: begin
						jump_in_dec_o = 1'b1;
						alu_op_a_mux_sel_o = OP_A_CURRPC;
						alu_op_b_mux_sel_o = OP_B_IMM;
						imm_b_mux_sel_o = IMM_B_INCR_PC;
						alu_operator_o = ALU_ADD;
						regfile_we = 1'b0;
						if (instr_new_i)
							jump_set_o = 1'b1;
					end
					default: illegal_insn = 1'b1;
				endcase
			OPCODE_SYSTEM:
				if ((instr[14:12] == 3'b000)) begin
					alu_op_a_mux_sel_o = OP_A_REG_A;
					alu_op_b_mux_sel_o = OP_B_IMM;
					case (instr[31:20])
						12'h000: ecall_insn_o = 1'b1;
						12'h001: ebrk_insn_o = 1'b1;
						12'h302: mret_insn_o = 1'b1;
						12'h7b2: dret_insn_o = 1'b1;
						12'h105: wfi_insn_o = 1'b1;
						default: illegal_insn = 1'b1;
					endcase
					if (((instr[19:15] != 5'b0) || (instr[11:07] != 5'b0)))
						illegal_insn = 1'b1;
				end
				else begin
					csr_access_o = 1'b1;
					regfile_wdata_sel_o = RF_WD_CSR;
					regfile_we = 1'b1;
					alu_op_b_mux_sel_o = OP_B_IMM;
					imm_a_mux_sel_o = IMM_A_Z;
					imm_b_mux_sel_o = IMM_B_I;
					if (instr[14])
						alu_op_a_mux_sel_o = OP_A_IMM;
					else
						alu_op_a_mux_sel_o = OP_A_REG_A;
					case (instr[13:12])
						2'b01: csr_op = CSR_OP_WRITE;
						2'b10: csr_op = CSR_OP_SET;
						2'b11: csr_op = CSR_OP_CLEAR;
						default: csr_illegal = 1'b1;
					endcase
					illegal_insn = csr_illegal;
				end
			default: illegal_insn = 1'b1;
		endcase
		if (illegal_c_insn_i)
			illegal_insn = 1'b1;
		if (illegal_insn) begin
			regfile_we = 1'b0;
			data_req_o = 1'b0;
			data_we_o = 1'b0;
			mult_en_o = 1'b0;
			div_en_o = 1'b0;
			jump_in_dec_o = 1'b0;
			jump_set_o = 1'b0;
			branch_in_dec_o = 1'b0;
			csr_access_o = 1'b0;
		end
	end
	assign illegal_insn_o = (illegal_insn | illegal_reg_rv32e);
	assign regfile_we_o = (regfile_we & ~illegal_reg_rv32e);
	function [(5 - 1):0] sv2v_cast_F99D1;
		input [(5 - 1):0] inp;
		sv2v_cast_F99D1 = inp;
	endfunction
	function [(12 - 1):0] sv2v_cast_290A1;
		input [(12 - 1):0] inp;
		sv2v_cast_290A1 = inp;
	endfunction
	function [(7 - 1):0] sv2v_cast_DB892;
		input [(7 - 1):0] inp;
		sv2v_cast_DB892 = inp;
	endfunction
endmodule

module ibex_id_stage (
	clk_i,
	rst_ni,
	test_en_i,
	fetch_enable_i,
	ctrl_busy_o,
	illegal_insn_o,
	instr_valid_i,
	instr_new_i,
	instr_rdata_i,
	instr_rdata_c_i,
	instr_is_compressed_i,
	instr_req_o,
	instr_valid_clear_o,
	id_in_ready_o,
	branch_decision_i,
	pc_set_o,
	pc_mux_o,
	exc_pc_mux_o,
	exc_cause_o,
	illegal_c_insn_i,
	instr_fetch_err_i,
	pc_id_i,
	ex_valid_i,
	lsu_valid_i,
	alu_operator_ex_o,
	alu_operand_a_ex_o,
	alu_operand_b_ex_o,
	mult_en_ex_o,
	div_en_ex_o,
	multdiv_operator_ex_o,
	multdiv_signed_mode_ex_o,
	multdiv_operand_a_ex_o,
	multdiv_operand_b_ex_o,
	csr_access_o,
	csr_op_o,
	csr_save_if_o,
	csr_save_id_o,
	csr_restore_mret_id_o,
	csr_restore_dret_id_o,
	csr_save_cause_o,
	csr_mtval_o,
	priv_mode_i,
	csr_mstatus_tw_i,
	illegal_csr_insn_i,
	data_req_ex_o,
	data_we_ex_o,
	data_type_ex_o,
	data_sign_ext_ex_o,
	data_wdata_ex_o,
	lsu_addr_incr_req_i,
	lsu_addr_last_i,
	csr_mstatus_mie_i,
	csr_msip_i,
	csr_mtip_i,
	csr_meip_i,
	csr_mfip_i,
	irq_pending_i,
	irq_nm_i,
	lsu_load_err_i,
	lsu_store_err_i,
	debug_mode_o,
	debug_cause_o,
	debug_csr_save_o,
	debug_req_i,
	debug_single_step_i,
	debug_ebreakm_i,
	debug_ebreaku_i,
	regfile_wdata_lsu_i,
	regfile_wdata_ex_i,
	csr_rdata_i,
	rfvi_reg_raddr_ra_o,
	rfvi_reg_rdata_ra_o,
	rfvi_reg_raddr_rb_o,
	rfvi_reg_rdata_rb_o,
	rfvi_reg_waddr_rd_o,
	rfvi_reg_wdata_rd_o,
	rfvi_reg_we_o,
	perf_jump_o,
	perf_branch_o,
	perf_tbranch_o,
	instr_ret_o,
	instr_ret_compressed_o
);
	localparam [0:0] IDLE = 0;
	localparam [0:0] WAIT_MULTICYCLE = 1;
	parameter RV32E = 0;
	parameter RV32M = 1;
	input wire clk_i;
	input wire rst_ni;
	input wire test_en_i;
	input wire fetch_enable_i;
	output wire ctrl_busy_o;
	output wire illegal_insn_o;
	input wire instr_valid_i;
	input wire instr_new_i;
	input wire [31:0] instr_rdata_i;
	input wire [15:0] instr_rdata_c_i;
	input wire instr_is_compressed_i;
	output wire instr_req_o;
	output wire instr_valid_clear_o;
	output wire id_in_ready_o;
	input wire branch_decision_i;
	output wire pc_set_o;
	output wire [2:0] pc_mux_o;
	output wire [1:0] exc_pc_mux_o;
	output wire [5:0] exc_cause_o;
	input wire illegal_c_insn_i;
	input wire instr_fetch_err_i;
	input wire [31:0] pc_id_i;
	input wire ex_valid_i;
	input wire lsu_valid_i;
	output wire [4:0] alu_operator_ex_o;
	output wire [31:0] alu_operand_a_ex_o;
	output wire [31:0] alu_operand_b_ex_o;
	output wire mult_en_ex_o;
	output wire div_en_ex_o;
	output wire [1:0] multdiv_operator_ex_o;
	output wire [1:0] multdiv_signed_mode_ex_o;
	output wire [31:0] multdiv_operand_a_ex_o;
	output wire [31:0] multdiv_operand_b_ex_o;
	output wire csr_access_o;
	output wire [1:0] csr_op_o;
	output wire csr_save_if_o;
	output wire csr_save_id_o;
	output wire csr_restore_mret_id_o;
	output wire csr_restore_dret_id_o;
	output wire csr_save_cause_o;
	output wire [31:0] csr_mtval_o;
	input [1:0] priv_mode_i;
	input wire csr_mstatus_tw_i;
	input wire illegal_csr_insn_i;
	output wire data_req_ex_o;
	output wire data_we_ex_o;
	output wire [1:0] data_type_ex_o;
	output wire data_sign_ext_ex_o;
	output wire [31:0] data_wdata_ex_o;
	input wire lsu_addr_incr_req_i;
	input wire [31:0] lsu_addr_last_i;
	input wire csr_mstatus_mie_i;
	input wire csr_msip_i;
	input wire csr_mtip_i;
	input wire csr_meip_i;
	input wire [14:0] csr_mfip_i;
	input wire irq_pending_i;
	input wire irq_nm_i;
	input wire lsu_load_err_i;
	input wire lsu_store_err_i;
	output wire debug_mode_o;
	output wire [2:0] debug_cause_o;
	output wire debug_csr_save_o;
	input wire debug_req_i;
	input wire debug_single_step_i;
	input wire debug_ebreakm_i;
	input wire debug_ebreaku_i;
	input wire [31:0] regfile_wdata_lsu_i;
	input wire [31:0] regfile_wdata_ex_i;
	input wire [31:0] csr_rdata_i;
	output wire [4:0] rfvi_reg_raddr_ra_o;
	output wire [31:0] rfvi_reg_rdata_ra_o;
	output wire [4:0] rfvi_reg_raddr_rb_o;
	output wire [31:0] rfvi_reg_rdata_rb_o;
	output wire [4:0] rfvi_reg_waddr_rd_o;
	output wire [31:0] rfvi_reg_wdata_rd_o;
	output wire rfvi_reg_we_o;
	output wire perf_jump_o;
	output reg perf_branch_o;
	output wire perf_tbranch_o;
	output reg instr_ret_o;
	output wire instr_ret_compressed_o;
	`include "ibex_pkg.sv"
	wire illegal_insn_dec;
	wire ebrk_insn;
	wire mret_insn_dec;
	wire dret_insn_dec;
	wire ecall_insn_dec;
	wire wfi_insn_dec;
	wire branch_in_dec;
	reg branch_set_n;
	reg branch_set_q;
	wire jump_in_dec;
	wire jump_set;
	wire instr_executing;
	wire instr_multicycle;
	reg instr_multicycle_done_n;
	reg instr_multicycle_done_q;
	reg stall_lsu;
	reg stall_multdiv;
	reg stall_branch;
	reg stall_jump;
	wire [31:0] imm_i_type;
	wire [31:0] imm_s_type;
	wire [31:0] imm_b_type;
	wire [31:0] imm_u_type;
	wire [31:0] imm_j_type;
	wire [31:0] zimm_rs1_type;
	wire [31:0] imm_a;
	reg [31:0] imm_b;
	wire [4:0] regfile_raddr_a;
	wire [4:0] regfile_raddr_b;
	wire [4:0] regfile_waddr;
	wire [31:0] regfile_rdata_a;
	wire [31:0] regfile_rdata_b;
	reg [31:0] regfile_wdata;
	wire [1:0] regfile_wdata_sel;
	wire regfile_we;
	reg regfile_we_wb;
	wire regfile_we_dec;
	wire [4:0] alu_operator;
	wire [1:0] alu_op_a_mux_sel;
	wire [1:0] alu_op_a_mux_sel_dec;
	wire alu_op_b_mux_sel;
	wire alu_op_b_mux_sel_dec;
	wire imm_a_mux_sel;
	wire [2:0] imm_b_mux_sel;
	wire [2:0] imm_b_mux_sel_dec;
	wire mult_en_id;
	wire mult_en_dec;
	wire div_en_id;
	wire div_en_dec;
	wire multdiv_en_dec;
	wire [1:0] multdiv_operator;
	wire [1:0] multdiv_signed_mode;
	wire data_we_id;
	wire [1:0] data_type_id;
	wire data_sign_ext_id;
	wire data_req_id;
	wire data_req_dec;
	wire csr_pipe_flush;
	reg [31:0] alu_operand_a;
	wire [31:0] alu_operand_b;
	assign alu_op_a_mux_sel = (lsu_addr_incr_req_i ? OP_A_FWD : alu_op_a_mux_sel_dec);
	assign alu_op_b_mux_sel = (lsu_addr_incr_req_i ? OP_B_IMM : alu_op_b_mux_sel_dec);
	assign imm_b_mux_sel = (lsu_addr_incr_req_i ? IMM_B_INCR_ADDR : imm_b_mux_sel_dec);
	assign imm_a = ((imm_a_mux_sel == IMM_A_Z) ? zimm_rs1_type : 1'b0);
	always @(*) begin : alu_operand_a_mux
		case (alu_op_a_mux_sel)
			OP_A_REG_A: alu_operand_a = regfile_rdata_a;
			OP_A_FWD: alu_operand_a = lsu_addr_last_i;
			OP_A_CURRPC: alu_operand_a = pc_id_i;
			OP_A_IMM: alu_operand_a = imm_a;
			default: alu_operand_a = 1'bX;
		endcase
	end
	always @(*) begin : immediate_b_mux
		case (imm_b_mux_sel)
			IMM_B_I: imm_b = imm_i_type;
			IMM_B_S: imm_b = imm_s_type;
			IMM_B_B: imm_b = imm_b_type;
			IMM_B_U: imm_b = imm_u_type;
			IMM_B_J: imm_b = imm_j_type;
			IMM_B_INCR_PC: imm_b = (instr_is_compressed_i ? 32'h2 : 32'h4);
			IMM_B_INCR_ADDR: imm_b = 32'h4;
			default: imm_b = 1'bX;
		endcase
	end
	assign alu_operand_b = ((alu_op_b_mux_sel == OP_B_IMM) ? imm_b : regfile_rdata_b);
	assign regfile_we = ((illegal_csr_insn_i || !instr_executing) ? 1'b0 : ((data_req_dec || multdiv_en_dec) ? regfile_we_wb : regfile_we_dec));
	always @(*) begin : regfile_wdata_mux
		case (regfile_wdata_sel)
			RF_WD_EX: regfile_wdata = regfile_wdata_ex_i;
			RF_WD_LSU: regfile_wdata = regfile_wdata_lsu_i;
			RF_WD_CSR: regfile_wdata = csr_rdata_i;
			default: regfile_wdata = 1'bX;
		endcase
	end
	ibex_register_file #(.RV32E(RV32E)) registers_i(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.test_en_i(test_en_i),
		.raddr_a_i(regfile_raddr_a),
		.rdata_a_o(regfile_rdata_a),
		.raddr_b_i(regfile_raddr_b),
		.rdata_b_o(regfile_rdata_b),
		.waddr_a_i(regfile_waddr),
		.wdata_a_i(regfile_wdata),
		.we_a_i(regfile_we)
	);
	assign rfvi_reg_raddr_ra_o = regfile_raddr_a;
	assign rfvi_reg_rdata_ra_o = regfile_rdata_a;
	assign rfvi_reg_raddr_rb_o = regfile_raddr_b;
	assign rfvi_reg_rdata_rb_o = regfile_rdata_b;
	assign rfvi_reg_waddr_rd_o = regfile_waddr;
	assign rfvi_reg_wdata_rd_o = regfile_wdata;
	assign rfvi_reg_we_o = regfile_we;
	ibex_decoder #(
		.RV32E(RV32E),
		.RV32M(RV32M)
	) decoder_i(
		.illegal_insn_o(illegal_insn_dec),
		.ebrk_insn_o(ebrk_insn),
		.mret_insn_o(mret_insn_dec),
		.dret_insn_o(dret_insn_dec),
		.ecall_insn_o(ecall_insn_dec),
		.wfi_insn_o(wfi_insn_dec),
		.jump_set_o(jump_set),
		.instr_new_i(instr_new_i),
		.instr_rdata_i(instr_rdata_i),
		.illegal_c_insn_i(illegal_c_insn_i),
		.imm_a_mux_sel_o(imm_a_mux_sel),
		.imm_b_mux_sel_o(imm_b_mux_sel_dec),
		.imm_i_type_o(imm_i_type),
		.imm_s_type_o(imm_s_type),
		.imm_b_type_o(imm_b_type),
		.imm_u_type_o(imm_u_type),
		.imm_j_type_o(imm_j_type),
		.zimm_rs1_type_o(zimm_rs1_type),
		.regfile_wdata_sel_o(regfile_wdata_sel),
		.regfile_we_o(regfile_we_dec),
		.regfile_raddr_a_o(regfile_raddr_a),
		.regfile_raddr_b_o(regfile_raddr_b),
		.regfile_waddr_o(regfile_waddr),
		.alu_operator_o(alu_operator),
		.alu_op_a_mux_sel_o(alu_op_a_mux_sel_dec),
		.alu_op_b_mux_sel_o(alu_op_b_mux_sel_dec),
		.mult_en_o(mult_en_dec),
		.div_en_o(div_en_dec),
		.multdiv_operator_o(multdiv_operator),
		.multdiv_signed_mode_o(multdiv_signed_mode),
		.csr_access_o(csr_access_o),
		.csr_op_o(csr_op_o),
		.csr_pipe_flush_o(csr_pipe_flush),
		.data_req_o(data_req_dec),
		.data_we_o(data_we_id),
		.data_type_o(data_type_id),
		.data_sign_extension_o(data_sign_ext_id),
		.jump_in_dec_o(jump_in_dec),
		.branch_in_dec_o(branch_in_dec)
	);
	assign illegal_insn_o = (instr_valid_i & (illegal_insn_dec | illegal_csr_insn_i));
	ibex_controller controller_i(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.fetch_enable_i(fetch_enable_i),
		.ctrl_busy_o(ctrl_busy_o),
		.illegal_insn_i(illegal_insn_o),
		.ecall_insn_i(ecall_insn_dec),
		.mret_insn_i(mret_insn_dec),
		.dret_insn_i(dret_insn_dec),
		.wfi_insn_i(wfi_insn_dec),
		.ebrk_insn_i(ebrk_insn),
		.csr_pipe_flush_i(csr_pipe_flush),
		.instr_valid_i(instr_valid_i),
		.instr_i(instr_rdata_i),
		.instr_compressed_i(instr_rdata_c_i),
		.instr_is_compressed_i(instr_is_compressed_i),
		.instr_fetch_err_i(instr_fetch_err_i),
		.pc_id_i(pc_id_i),
		.instr_valid_clear_o(instr_valid_clear_o),
		.id_in_ready_o(id_in_ready_o),
		.instr_req_o(instr_req_o),
		.pc_set_o(pc_set_o),
		.pc_mux_o(pc_mux_o),
		.exc_pc_mux_o(exc_pc_mux_o),
		.exc_cause_o(exc_cause_o),
		.lsu_addr_last_i(lsu_addr_last_i),
		.load_err_i(lsu_load_err_i),
		.store_err_i(lsu_store_err_i),
		.branch_set_i(branch_set_q),
		.jump_set_i(jump_set),
		.csr_mstatus_mie_i(csr_mstatus_mie_i),
		.csr_msip_i(csr_msip_i),
		.csr_mtip_i(csr_mtip_i),
		.csr_meip_i(csr_meip_i),
		.csr_mfip_i(csr_mfip_i),
		.irq_pending_i(irq_pending_i),
		.irq_nm_i(irq_nm_i),
		.csr_save_if_o(csr_save_if_o),
		.csr_save_id_o(csr_save_id_o),
		.csr_restore_mret_id_o(csr_restore_mret_id_o),
		.csr_restore_dret_id_o(csr_restore_dret_id_o),
		.csr_save_cause_o(csr_save_cause_o),
		.csr_mtval_o(csr_mtval_o),
		.priv_mode_i(priv_mode_i),
		.csr_mstatus_tw_i(csr_mstatus_tw_i),
		.debug_mode_o(debug_mode_o),
		.debug_cause_o(debug_cause_o),
		.debug_csr_save_o(debug_csr_save_o),
		.debug_req_i(debug_req_i),
		.debug_single_step_i(debug_single_step_i),
		.debug_ebreakm_i(debug_ebreakm_i),
		.debug_ebreaku_i(debug_ebreaku_i),
		.stall_lsu_i(stall_lsu),
		.stall_multdiv_i(stall_multdiv),
		.stall_jump_i(stall_jump),
		.stall_branch_i(stall_branch),
		.perf_jump_o(perf_jump_o),
		.perf_tbranch_o(perf_tbranch_o)
	);
	assign multdiv_en_dec = (mult_en_dec | div_en_dec);
	assign instr_multicycle = (((data_req_dec | multdiv_en_dec) | branch_in_dec) | jump_in_dec);
	assign instr_executing = ((instr_new_i | (instr_multicycle & ~instr_multicycle_done_q)) & ~instr_fetch_err_i);
	assign data_req_id = (instr_executing ? data_req_dec : 1'b0);
	assign mult_en_id = (instr_executing ? mult_en_dec : 1'b0);
	assign div_en_id = (instr_executing ? div_en_dec : 1'b0);
	assign data_req_ex_o = data_req_id;
	assign data_we_ex_o = data_we_id;
	assign data_type_ex_o = data_type_id;
	assign data_sign_ext_ex_o = data_sign_ext_id;
	assign data_wdata_ex_o = regfile_rdata_b;
	assign alu_operator_ex_o = alu_operator;
	assign alu_operand_a_ex_o = alu_operand_a;
	assign alu_operand_b_ex_o = alu_operand_b;
	assign mult_en_ex_o = mult_en_id;
	assign div_en_ex_o = div_en_id;
	assign multdiv_operator_ex_o = multdiv_operator;
	assign multdiv_signed_mode_ex_o = multdiv_signed_mode;
	assign multdiv_operand_a_ex_o = regfile_rdata_a;
	assign multdiv_operand_b_ex_o = regfile_rdata_b;
	reg [0:0] id_wb_fsm_cs;
	reg [0:0] id_wb_fsm_ns;
	always @(posedge clk_i or negedge rst_ni) begin : id_wb_pipeline_reg
		if (!rst_ni) begin
			id_wb_fsm_cs <= IDLE;
			branch_set_q <= 1'b0;
			instr_multicycle_done_q <= 1'b0;
		end
		else begin
			id_wb_fsm_cs <= id_wb_fsm_ns;
			branch_set_q <= branch_set_n;
			instr_multicycle_done_q <= instr_multicycle_done_n;
		end
	end
	always @(*) begin : id_wb_fsm
		id_wb_fsm_ns = id_wb_fsm_cs;
		instr_multicycle_done_n = instr_multicycle_done_q;
		regfile_we_wb = 1'b0;
		stall_lsu = 1'b0;
		stall_multdiv = 1'b0;
		stall_jump = 1'b0;
		stall_branch = 1'b0;
		branch_set_n = 1'b0;
		perf_branch_o = 1'b0;
		instr_ret_o = 1'b0;
		case (id_wb_fsm_cs)
			IDLE:
				if ((instr_new_i & ~instr_fetch_err_i))
					case (1'b1)
						data_req_dec: begin
							id_wb_fsm_ns = WAIT_MULTICYCLE;
							stall_lsu = 1'b1;
							instr_multicycle_done_n = 1'b0;
						end
						multdiv_en_dec: begin
							id_wb_fsm_ns = WAIT_MULTICYCLE;
							stall_multdiv = 1'b1;
							instr_multicycle_done_n = 1'b0;
						end
						branch_in_dec: begin
							id_wb_fsm_ns = (branch_decision_i ? WAIT_MULTICYCLE : IDLE);
							stall_branch = branch_decision_i;
							instr_multicycle_done_n = ~branch_decision_i;
							branch_set_n = branch_decision_i;
							perf_branch_o = 1'b1;
							instr_ret_o = ~branch_decision_i;
						end
						jump_in_dec: begin
							id_wb_fsm_ns = WAIT_MULTICYCLE;
							stall_jump = 1'b1;
							instr_multicycle_done_n = 1'b0;
						end
						default: begin
						instr_multicycle_done_n = 1'b0;
						instr_ret_o = 1'b1;
					end
					endcase
			WAIT_MULTICYCLE:
				if (((data_req_dec & lsu_valid_i) | (~data_req_dec & ex_valid_i))) begin
					id_wb_fsm_ns = IDLE;
					instr_multicycle_done_n = 1'b1;
					regfile_we_wb = (regfile_we_dec & ~lsu_load_err_i);
					instr_ret_o = 1'b1;
				end
				else begin
					stall_lsu = data_req_dec;
					stall_multdiv = multdiv_en_dec;
					stall_branch = branch_in_dec;
					stall_jump = jump_in_dec;
				end
			default: id_wb_fsm_ns = 1'bX;
		endcase
	end
	assign instr_ret_compressed_o = (instr_ret_o & instr_is_compressed_i);

	assert property ((illegal_insn_o == 1 & lsu_store_err_i == 0 & illegal_c_insn_i == 1 & instr_rdata_i[2] == 1) ##1 (illegal_c_insn_i == 1) |-> (instr_valid_clear_o == 1));
endmodule

