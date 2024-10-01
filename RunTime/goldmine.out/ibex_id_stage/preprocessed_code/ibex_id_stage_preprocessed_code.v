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
localparam   OPCODE_LOAD     = 7'h03;
localparam   OPCODE_MISC_MEM = 7'h0f;
localparam   OPCODE_OP_IMM   = 7'h13;
localparam   OPCODE_AUIPC    = 7'h17;
localparam   OPCODE_STORE    = 7'h23;
localparam   OPCODE_OP       = 7'h33;
localparam   OPCODE_LUI      = 7'h37;
localparam   OPCODE_BRANCH   = 7'h63;
localparam   OPCODE_JALR     = 7'h67;
localparam   OPCODE_JAL      = 7'h6f;
localparam   OPCODE_SYSTEM   = 7'h73;


////////////////////
// ALU operations //
////////////////////

  // Arithmetics
localparam   ALU_ADD = 5'h0;
localparam   ALU_SUB = 5'h1;

  // Logics
localparam   ALU_XOR = 5'h2;
localparam   ALU_OR = 5'h3;
localparam   ALU_AND = 5'h4;

  // Shifts
localparam   ALU_SRA = 5'h5;
localparam   ALU_SRL = 5'h6;
localparam   ALU_SLL = 5'h7;

  // Comparisons
localparam   ALU_LT = 5'h8;
localparam   ALU_LTU = 5'h9;
localparam   ALU_GE = 5'ha;
localparam   ALU_GEU = 5'hb;
localparam   ALU_EQ = 5'hc;
localparam   ALU_NE = 5'hd;

  // Set lower than
localparam   ALU_SLT = 5'he;
localparam   ALU_SLTU = 5'hf;

  // Multiplier/divider
localparam   MD_OP_MULL = 2'h0;
localparam   MD_OP_MULH = 2'h1;
localparam   MD_OP_DIV = 2'h2;
localparam   MD_OP_REM = 2'h3;


//////////////////////////////////
// Control and status registers //
//////////////////////////////////

// CSR operations
localparam   CSR_OP_READ = 2'h0;
localparam   CSR_OP_WRITE = 2'h1;
localparam   CSR_OP_SET = 2'h2;
localparam   CSR_OP_CLEAR = 2'h3;


// Privileged mode
localparam   PRIV_LVL_M = 2'b11;
localparam   PRIV_LVL_H = 2'b10;
localparam   PRIV_LVL_S = 2'b01;
localparam   PRIV_LVL_U = 2'b00;

// Constants for the dcsr.xdebugver fields
localparam    XDEBUGVER_NO     = 4'd0; // no external debug support
localparam    XDEBUGVER_STD    = 4'd4; // external debug according to RISC-V debug spec
localparam    XDEBUGVER_NONSTD = 4'd15; // debug not conforming to RISC-V debug spec


//////////////
// ID stage //
//////////////

// Operand a selection
localparam   OP_A_REG_A = 2'h0;
localparam   OP_A_FWD = 2'h1;
localparam   OP_A_CURRPC = 2'h2;
localparam   OP_A_IMM = 2'h3;

// Immediate a selection
localparam   IMM_A_Z = 1'b0;
localparam   IMM_A_ZERO = 1'b1;

// Operand b selection
localparam   OP_B_REG_B = 1'b0;
localparam   OP_B_IMM = 1'b1;

// Immediate b selection
localparam   IMM_B_I = 3'h0;
localparam   IMM_B_S = 3'h1;
localparam   IMM_B_B = 3'h2;
localparam   IMM_B_U = 3'h3;
localparam   IMM_B_J = 3'h4;
localparam   IMM_B_INCR_PC = 3'h4;
localparam   IMM_B_INCR_ADDR = 3'h5;

// Regfile write data selection
localparam   RF_WD_LSU = 2'h0;
localparam   RF_WD_EX = 2'h1;
localparam   RF_WD_CSR = 2'h2;

//////////////
// IF stage //
//////////////

// PC mux selection
localparam   PC_BOOT = 3'h0;
localparam   PC_JUMP = 3'h1;
localparam   PC_EXC = 3'h2;
localparam   PC_ERET = 3'h3;
localparam   PC_DRET = 3'h4;

// Exception PC mux selection
localparam   EXC_PC_EXC = 2'h0;
localparam   EXC_PC_IRQ = 2'h1;
localparam   EXC_PC_DBD = 2'h2;
localparam   EXC_PC_DBG_EXC = 2'h3; // Exception while in debug mode

// Exception cause
localparam   EXC_CAUSE_IRQ_SOFTWARE_M     = {1'b1, 5'd03};
localparam   EXC_CAUSE_IRQ_TIMER_M        = {1'b1, 5'd07};
localparam   EXC_CAUSE_IRQ_EXTERNAL_M     = {1'b1, 5'd11};
  // EXC_CAUSE_IRQ_FAST_0      = {1'b1, 5'd16};
  // EXC_CAUSE_IRQ_FAST_14     = {1'b1, 5'd30};
localparam   EXC_CAUSE_IRQ_NM             = {1'b1, 5'd31}; // == EXC_CAUSE_IRQ_FAST_15
localparam   EXC_CAUSE_INSN_ADDR_MISA     = {1'b0, 5'd00};
localparam   EXC_CAUSE_INSTR_ACCESS_FAULT = {1'b0, 5'd01};
localparam   EXC_CAUSE_ILLEGAL_INSN       = {1'b0, 5'd02};
localparam   EXC_CAUSE_BREAKPOINT         = {1'b0, 5'd03};
localparam   EXC_CAUSE_LOAD_ACCESS_FAULT  = {1'b0, 5'd05};
localparam   EXC_CAUSE_STORE_ACCESS_FAULT = {1'b0, 5'd07};
localparam   EXC_CAUSE_ECALL_UMODE        = {1'b0, 5'd08};
localparam   EXC_CAUSE_ECALL_MMODE        = {1'b0, 5'd11};

// Debug cause
localparam   DBG_CAUSE_NONE    = 3'h0;
localparam   DBG_CAUSE_EBREAK  = 3'h1;
localparam   DBG_CAUSE_TRIGGER = 3'h2;
localparam   DBG_CAUSE_HALTREQ = 3'h3;
localparam   DBG_CAUSE_STEP    = 3'h4;

// PMP constants
parameter   PMP_MAX_REGIONS      = 16;
parameter   PMP_CFG_W            = 8;

// PMP acces type
parameter   PMP_I = 0;
parameter   PMP_D = 1;

localparam   PMP_ACC_EXEC    = 2'b00;
localparam   PMP_ACC_WRITE   = 2'b01;
localparam   PMP_ACC_READ    = 2'b10;

// PMP cfg structures
localparam   PMP_MODE_OFF   = 2'b00;
localparam   PMP_MODE_TOR   = 2'b01;
localparam   PMP_MODE_NA4   = 2'b10;
localparam   PMP_MODE_NAPOT = 2'b11;

//typedef struct packed {
//  logic          lock;
//  pmp_cfg_mode_e mode;
//  logic          exec;
//  logic          write;
//  logic          read;

// CSRs
  // Machine information
localparam   CSR_MHARTID   = 12'hF14;

  // Machine trap setup
localparam   CSR_MSTATUS   = 12'h300;
localparam   CSR_MISA      = 12'h301;
localparam   CSR_MIE       = 12'h304;
localparam   CSR_MTVEC     = 12'h305;

  // Machine trap handling
localparam   CSR_MSCRATCH  = 12'h340;
localparam   CSR_MEPC      = 12'h341;
localparam   CSR_MCAUSE    = 12'h342;
localparam   CSR_MTVAL     = 12'h343;
localparam   CSR_MIP       = 12'h344;

  // Physical memory protection
localparam   CSR_PMPCFG0   = 12'h3A0;
localparam   CSR_PMPCFG1   = 12'h3A1;
localparam   CSR_PMPCFG2   = 12'h3A2;
localparam   CSR_PMPCFG3   = 12'h3A3;
localparam   CSR_PMPADDR0  = 12'h3B0;
localparam   CSR_PMPADDR1  = 12'h3B1;
localparam   CSR_PMPADDR2  = 12'h3B2;
localparam   CSR_PMPADDR3  = 12'h3B3;
localparam   CSR_PMPADDR4  = 12'h3B4;
localparam   CSR_PMPADDR5  = 12'h3B5;
localparam   CSR_PMPADDR6  = 12'h3B6;
localparam   CSR_PMPADDR7  = 12'h3B7;
localparam   CSR_PMPADDR8  = 12'h3B8;
localparam   CSR_PMPADDR9  = 12'h3B9;
localparam   CSR_PMPADDR10 = 12'h3BA;
localparam   CSR_PMPADDR11 = 12'h3BB;
localparam   CSR_PMPADDR12 = 12'h3BC;
localparam   CSR_PMPADDR13 = 12'h3BD;
localparam   CSR_PMPADDR14 = 12'h3BE;
localparam   CSR_PMPADDR15 = 12'h3BF;

  // Debug/trace
localparam   CSR_DCSR      = 12'h7b0;
localparam   CSR_DPC       = 12'h7b1;

  // Debug
localparam   CSR_DSCRATCH0 = 12'h7b2; // optional
localparam   CSR_DSCRATCH1 = 12'h7b3; // optional

  // Machine Counter/Timers
localparam   CSR_MCOUNTINHIBIT  = 12'h320;
localparam   CSR_MHPMEVENT3     = 12'h323;
localparam   CSR_MHPMEVENT4     = 12'h324;
localparam   CSR_MHPMEVENT5     = 12'h325;
localparam   CSR_MHPMEVENT6     = 12'h326;
localparam   CSR_MHPMEVENT7     = 12'h327;
localparam   CSR_MHPMEVENT8     = 12'h328;
localparam   CSR_MHPMEVENT9     = 12'h329;
localparam   CSR_MHPMEVENT10    = 12'h32A;
localparam   CSR_MHPMEVENT11    = 12'h32B;
localparam   CSR_MHPMEVENT12    = 12'h32C;
localparam   CSR_MHPMEVENT13    = 12'h32D;
localparam   CSR_MHPMEVENT14    = 12'h32E;
localparam   CSR_MHPMEVENT15    = 12'h32F;
localparam   CSR_MHPMEVENT16    = 12'h330;
localparam   CSR_MHPMEVENT17    = 12'h331;
localparam   CSR_MHPMEVENT18    = 12'h332;
localparam   CSR_MHPMEVENT19    = 12'h333;
localparam   CSR_MHPMEVENT20    = 12'h334;
localparam   CSR_MHPMEVENT21    = 12'h335;
localparam   CSR_MHPMEVENT22    = 12'h336;
localparam   CSR_MHPMEVENT23    = 12'h337;
localparam   CSR_MHPMEVENT24    = 12'h338;
localparam   CSR_MHPMEVENT25    = 12'h339;
localparam   CSR_MHPMEVENT26    = 12'h33A;
localparam   CSR_MHPMEVENT27    = 12'h33B;
localparam   CSR_MHPMEVENT28    = 12'h33C;
localparam   CSR_MHPMEVENT29    = 12'h33D;
localparam   CSR_MHPMEVENT30    = 12'h33E;
localparam   CSR_MHPMEVENT31    = 12'h33F;
localparam   CSR_MCYCLE         = 12'hB00;
localparam   CSR_MINSTRET       = 12'hB02;
localparam   CSR_MHPMCOUNTER3   = 12'hB03;
localparam   CSR_MHPMCOUNTER4   = 12'hB04;
localparam   CSR_MHPMCOUNTER5   = 12'hB05;
localparam   CSR_MHPMCOUNTER6   = 12'hB06;
localparam   CSR_MHPMCOUNTER7   = 12'hB07;
localparam   CSR_MHPMCOUNTER8   = 12'hB08;
localparam   CSR_MHPMCOUNTER9   = 12'hB09;
localparam   CSR_MHPMCOUNTER10  = 12'hB0A;
localparam   CSR_MHPMCOUNTER11  = 12'hB0B;
localparam   CSR_MHPMCOUNTER12  = 12'hB0C;
localparam   CSR_MHPMCOUNTER13  = 12'hB0D;
localparam   CSR_MHPMCOUNTER14  = 12'hB0E;
localparam   CSR_MHPMCOUNTER15  = 12'hB0F;
localparam   CSR_MHPMCOUNTER16  = 12'hB10;
localparam   CSR_MHPMCOUNTER17  = 12'hB11;
localparam   CSR_MHPMCOUNTER18  = 12'hB12;
localparam   CSR_MHPMCOUNTER19  = 12'hB13;
localparam   CSR_MHPMCOUNTER20  = 12'hB14;
localparam   CSR_MHPMCOUNTER21  = 12'hB15;
localparam   CSR_MHPMCOUNTER22  = 12'hB16;
localparam   CSR_MHPMCOUNTER23  = 12'hB17;
localparam   CSR_MHPMCOUNTER24  = 12'hB18;
localparam   CSR_MHPMCOUNTER25  = 12'hB19;
localparam   CSR_MHPMCOUNTER26  = 12'hB1A;
localparam   CSR_MHPMCOUNTER27  = 12'hB1B;
localparam   CSR_MHPMCOUNTER28  = 12'hB1C;
localparam   CSR_MHPMCOUNTER29  = 12'hB1D;
localparam   CSR_MHPMCOUNTER30  = 12'hB1E;
localparam   CSR_MHPMCOUNTER31  = 12'hB1F;
localparam   CSR_MCYCLEH        = 12'hB80;
localparam   CSR_MINSTRETH      = 12'hB82;
localparam   CSR_MHPMCOUNTER3H  = 12'hB83;
localparam   CSR_MHPMCOUNTER4H  = 12'hB84;
localparam   CSR_MHPMCOUNTER5H  = 12'hB85;
localparam   CSR_MHPMCOUNTER6H  = 12'hB86;
localparam   CSR_MHPMCOUNTER7H  = 12'hB87;
localparam   CSR_MHPMCOUNTER8H  = 12'hB88;
localparam   CSR_MHPMCOUNTER9H  = 12'hB89;
localparam   CSR_MHPMCOUNTER10H = 12'hB8A;
localparam   CSR_MHPMCOUNTER11H = 12'hB8B;
localparam   CSR_MHPMCOUNTER12H = 12'hB8C;
localparam   CSR_MHPMCOUNTER13H = 12'hB8D;
localparam   CSR_MHPMCOUNTER14H = 12'hB8E;
localparam   CSR_MHPMCOUNTER15H = 12'hB8F;
localparam   CSR_MHPMCOUNTER16H = 12'hB90;
localparam   CSR_MHPMCOUNTER17H = 12'hB91;
localparam   CSR_MHPMCOUNTER18H = 12'hB92;
localparam   CSR_MHPMCOUNTER19H = 12'hB93;
localparam   CSR_MHPMCOUNTER20H = 12'hB94;
localparam   CSR_MHPMCOUNTER21H = 12'hB95;
localparam   CSR_MHPMCOUNTER22H = 12'hB96;
localparam   CSR_MHPMCOUNTER23H = 12'hB97;
localparam   CSR_MHPMCOUNTER24H = 12'hB98;
localparam   CSR_MHPMCOUNTER25H = 12'hB99;
localparam   CSR_MHPMCOUNTER26H = 12'hB9A;
localparam   CSR_MHPMCOUNTER27H = 12'hB9B;
localparam   CSR_MHPMCOUNTER28H = 12'hB9C;
localparam   CSR_MHPMCOUNTER29H = 12'hB9D;
localparam   CSR_MHPMCOUNTER30H = 12'hB9E;
localparam   CSR_MHPMCOUNTER31H = 12'hB9F;

// CSR pmp-related offsets
parameter [11:0] CSR_OFF_PMP_CFG  = 12'h3A0; // pmp_cfg  @ 12'h3a0 - 12'h3a3
parameter [11:0] CSR_OFF_PMP_ADDR = 12'h3B0; // pmp_addr @ 12'h3b0 - 12'h3bf

// CSR status bits
parameter   CSR_MSTATUS_MIE_BIT      = 3;
parameter   CSR_MSTATUS_MPIE_BIT     = 7;
parameter   CSR_MSTATUS_MPP_BIT_LOW  = 11;
parameter   CSR_MSTATUS_MPP_BIT_HIGH = 12;
parameter   CSR_MSTATUS_MPRV_BIT     = 17;
parameter   CSR_MSTATUS_TW_BIT       = 21;

// CSR errupt pending/enable  int bits
parameter   CSR_MSIX_BIT      = 3;
parameter   CSR_MTIX_BIT      = 7;
parameter   CSR_MEIX_BIT      = 11;
parameter   CSR_MFIX_BIT_LOW  = 16;
parameter   CSR_MFIX_BIT_HIGH = 30;

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
endmodule
