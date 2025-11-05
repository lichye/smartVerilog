
`include "./cacheline_consts.vh"

module cacheline_tb ();

    parameter CLK_CYCLE_TIME = 10;
    parameter INIT_INTERVAL = 30;
    parameter SIM_CYCLE = 21; // 100000000;
    parameter SIM_TIME = SIM_CYCLE * CLK_CYCLE_TIME * 2;

    reg [31:0] 			CLK_CYCLE;
    reg 				clk;
    reg 				reset;

    initial begin
        clk = 1;
        forever #CLK_CYCLE_TIME clk = ~clk;
    end

    initial begin
        reset = 1;
        #INIT_INTERVAL 
        reset = 0;
    end
    
    initial begin
        CLK_CYCLE = 32'h0;
    end
    
    always @(posedge clk) begin
        CLK_CYCLE <= CLK_CYCLE + 1;
    end
        
    initial begin
        $dumpfile("cacheline_tb.vcd");
        $dumpvars(0, cacheline_tb);
    end

    initial begin
        #INIT_INTERVAL;
        #SIM_TIME;
        $finish;
    end


    reg os_req;
    reg [`NUM_WAYS-1:0] hitmap;
    reg user_req;
    reg [`ADDR_WIDTH-1:0] addr;

    wire io_os_req;
    wire [`NUM_WAYS-1:0] io_hitmap;
    wire io_user_req;
    wire [`ADDR_WIDTH-1:0] io_addr;
  
    assign io_os_req = os_req;
    assign io_hitmap = hitmap;
    assign io_user_req = user_req;
    assign io_addr = addr;

    cacheline c (
        .clk(clk),
        .reset(reset),
        .os_req(io_os_req),
        .hitmap(io_hitmap),
        .user_req(io_user_req),
        .addr(io_addr)
    );

    
endmodule