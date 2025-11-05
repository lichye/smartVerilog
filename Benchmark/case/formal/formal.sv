
`include "./cacheline_consts.vh"

module formal (
    input clk
);

    reg reset;
    reg [2:0] counter;
    reg init;
    
    initial begin
        reset = 1;
        init = 1;
        counter = 0;
    end

    reg io_os_req1;
    reg [`NUM_WAYS-1:0] io_hitmap1;
    reg io_user_req1;
    reg [`ADDR_WIDTH-1:0] io_addr1;
    reg io_hit1;
    reg io_os_req2;
    reg [`NUM_WAYS-1:0] io_hitmap2;
    reg io_user_req2;
    reg [`ADDR_WIDTH-1:0] io_addr2;
    reg io_hit2;
    (* anyseq *) reg os_req1;
    (* anyseq *) reg os_req2;
    (* anyseq *) reg [`NUM_WAYS-1:0] hitmap1;
    (* anyseq *) reg [`NUM_WAYS-1:0] hitmap2;
    (* anyseq *) reg user_req1;
    (* anyseq *) reg user_req2;
    (* anyseq *) reg [`ADDR_WIDTH:0] addr1;
    (* anyseq *) reg [`ADDR_WIDTH:0] addr2;
    (* anyseq *) reg [1:0] choice;

    (* anyconst *) reg [`NUM_WAYS-1:0] attacker_hitmap;

    wire [`NUM_WAYS-1:0] io_metadata1;
    wire [`NUM_WAYS-1:0] io_metadata2;
    wire [`NUM_WAYS-1:0] io_policy_hitmap1;
    wire [`NUM_WAYS-1:0] io_policy_hitmap2;
    wire [`ADDR_WIDTH*`NUM_WAYS-1:0] io_all_tags1;
    wire [`ADDR_WIDTH*`NUM_WAYS-1:0] io_all_tags2;
    wire [`NUM_WAYS-1:0] io_all_valid1;
    wire [`NUM_WAYS-1:0] io_all_valid2;

    nru c1 (
        .clk(clk),
        .reset(reset),
        .os_req(io_os_req1),
        .hitmap(io_hitmap1),
        .user_req(io_user_req1),
        .addr(io_addr1),
        .hit(io_hit1)
        , .metadata_o(io_metadata1)
        , .all_tags_o(io_all_tags1)
        , .all_valid_o(io_all_valid1)
        , .policy_hitmap_o(io_policy_hitmap1)
    );

    nru c2 (
        .clk(clk),
        .reset(reset),
        .os_req(io_os_req2),
        .hitmap(io_hitmap2),
        .user_req(io_user_req2),
        .addr(io_addr2),
        .hit(io_hit2)
        , .metadata_o(io_metadata2)
        , .all_tags_o(io_all_tags2)
        , .all_valid_o(io_all_valid2)
        , .policy_hitmap_o(io_policy_hitmap2)
    );


    reg attacker_domain;
    reg check;

    wire [`NUM_WAYS-1:0] tags_equal;
    genvar i;
    generate
        for (i = 0; i < `NUM_WAYS; i = i + 1) begin
            assign tags_equal[i] = (io_all_tags1[`ADDR_WIDTH*i+:`ADDR_WIDTH] == io_all_tags2[`ADDR_WIDTH*i+:`ADDR_WIDTH]);
        end
    endgenerate


    wire all_equal;
    assign all_equal = (io_os_req1 == io_os_req2) && (io_hitmap1 == io_hitmap2) && (io_user_req1 == io_user_req2) && (io_addr1 == io_addr2);

    wire eq_hit;
    assign eq_hit = (io_hit1 == io_hit2);

    always @(posedge clk) begin
        counter <= counter + 1'b1;
        if (counter == 7 && init) begin
            init <= 0;
        end
        if (counter == 1 && init) begin
            reset <= 0;
            attacker_domain = 0;
            check = 0;
            // assume(choice == 2'b00 || choice == 2'b01);
        end

    end

endmodule


