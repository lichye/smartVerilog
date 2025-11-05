
`include "./cacheline_consts.vh"

module cacheline (
    input clk,
    input reset,
    input os_req,
    input [`NUM_WAYS-1:0] hitmap,
    input user_req,
    input [`ADDR_WIDTH-1:0] addr,
    output hit
`ifdef INVARIANTS
    , output [`NUM_WAYS-1:0] metadata_o
    , output [`NUM_WAYS-1:0] policy_hitmap_o
    , output [`NUM_WAYS*`ADDR_WIDTH-1:0] all_tags_o
    , output [`NUM_WAYS-1:0] all_valid_o
`endif

);
    
    reg [`ADDR_WIDTH-1:0] tags [`NUM_WAYS-1:0];
    reg [`NUM_WAYS-1:0] metadata;
    assign metadata_o = metadata;
    
    reg [`NUM_WAYS-1:0] valid;

    reg [`NUM_WAYS-1:0] policy_hitmap;
    assign policy_hitmap_o = policy_hitmap;

    // PLRU policy
    wire [`NUM_WAYS-1:0] plru_policy;
    wire [`NUM_WAYS-1:0] plru_mask;
    assign plru_mask[4] = !(policy_hitmap[0] && policy_hitmap[1]);
    assign plru_mask[5] = !(policy_hitmap[2] && policy_hitmap[3]);
    assign plru_mask[6] = !(policy_hitmap[4] && policy_hitmap[5]);
    assign plru_mask[7] = !(policy_hitmap[6] && policy_hitmap[7]);
    assign plru_mask[2] = !(!plru_mask[4] && !plru_mask[5]);
    assign plru_mask[3] = !(!plru_mask[6] && !plru_mask[7]);
    assign plru_mask[1] = !(!plru_mask[2] && !plru_mask[3]);
    // assign // INLINED: plru_policy = configure_plru_policy(policy_hitmap);
    assign plru_policy[4] = policy_hitmap[1];
    assign plru_policy[5] = policy_hitmap[3];
    assign plru_policy[6] = policy_hitmap[5];
    assign plru_policy[7] = policy_hitmap[7];
    assign plru_policy[2] = policy_hitmap[3] || policy_hitmap[2];
    assign plru_policy[3] = policy_hitmap[6] || policy_hitmap[7];
    assign plru_policy[1] = policy_hitmap[6] || policy_hitmap[7] || policy_hitmap[5] || policy_hitmap[4];


    // identified by victim selector
    reg [`NUM_WAYS_WIDTH-1:0] victim_way;

    // identified by hit/miss checker
    reg [`NUM_WAYS_WIDTH-1:0] hit_way;
    reg hit;

    wire [`ADDR_WIDTH-1:0] tags_0;
    wire [`ADDR_WIDTH-1:0] tags_1;
    wire [`ADDR_WIDTH-1:0] tags_2;
    wire [`ADDR_WIDTH-1:0] tags_3;
    wire [`ADDR_WIDTH-1:0] tags_4;
    wire [`ADDR_WIDTH-1:0] tags_5;
    wire [`ADDR_WIDTH-1:0] tags_6;
    wire [`ADDR_WIDTH-1:0] tags_7;
    assign tags_0 = tags[0];
    assign tags_1 = tags[1];
    assign tags_2 = tags[2];
    assign tags_3 = tags[3];
    assign tags_4 = tags[4];
    assign tags_5 = tags[5];
    assign tags_6 = tags[6];
    assign tags_7 = tags[7];

    wire [`ADDR_WIDTH*`NUM_WAYS-1:0] all_tags_o;
    assign all_tags_o = {tags_7, tags_6, tags_5, tags_4, tags_3, tags_2, tags_1, tags_0};
    wire [`NUM_WAYS-1:0] all_valid_o;
    assign all_valid_o = valid;

    task switch_domain;
        input [`NUM_WAYS-1:0] _hitmap; 
        begin
            policy_hitmap[4] = _hitmap[4];
            policy_hitmap[5] = _hitmap[5];
            policy_hitmap[6] = _hitmap[6];
            policy_hitmap[7] = _hitmap[7];
            policy_hitmap[2] = _hitmap[2];
            policy_hitmap[3] = _hitmap[3];
            policy_hitmap[1] = _hitmap[1];
            policy_hitmap[0] = _hitmap[0];
            // INLINED: plru_mask = configure_plru_mask(policy_hitmap);
            // plru_mask[4] = !(_hitmap[0] && _hitmap[1]);
            // plru_mask[5] = !(_hitmap[2] && _hitmap[3]);
            // plru_mask[6] = !(_hitmap[4] && _hitmap[5]);
            // plru_mask[7] = !(_hitmap[6] && _hitmap[7]);
            // plru_mask[2] = !(!plru_mask[4] && !plru_mask[5]);
            // plru_mask[3] = !(!plru_mask[6] && !plru_mask[7]);
            // plru_mask[1] = !(!plru_mask[2] && !plru_mask[3]);
            // // INLINED: plru_policy = configure_plru_policy(policy_hitmap);
            // plru_policy[4] = _hitmap[1];
            // plru_policy[5] = _hitmap[3];
            // plru_policy[6] = _hitmap[5];
            // plru_policy[7] = _hitmap[7];
            // plru_policy[2] = _hitmap[3] || _hitmap[2];
            // plru_policy[3] = _hitmap[6] || _hitmap[7];
            // plru_policy[1] = _hitmap[6] || _hitmap[7] || _hitmap[5] || _hitmap[4];
        end
    endtask



    task flush_cache;
        begin
            if (policy_hitmap[0]) begin
                tags[0] = 0;
                valid[0] = 0;
            end
            if (policy_hitmap[1]) begin
                tags[1] = 0;
                valid[1] = 0;
            end
            if (policy_hitmap[2]) begin
                tags[2] = 0;
                valid[2] = 0;
            end
            if (policy_hitmap[3]) begin
                tags[3] = 0;
                valid[3] = 0;
            end
            if (policy_hitmap[4]) begin
                tags[4] = 0;
                valid[4] = 0;
            end
            if (policy_hitmap[5]) begin
                tags[5] = 0;
                valid[5] = 0;
            end
            if (policy_hitmap[6]) begin
                tags[6] = 0;
                valid[6] = 0;
            end
            if (policy_hitmap[7]) begin
                tags[7] = 0;
                valid[7] = 0;
            end
        end
    endtask

    task update_plru_tree;
        input [`NUM_WAYS_WIDTH-1:0] hit_ways;
        reg [`NUM_WAYS-1:0] plru_update;
        begin
            plru_update[0] = metadata[0];
            plru_update[1] = metadata[1];
            plru_update[2] = metadata[2];
            plru_update[3] = metadata[3];
            plru_update[4] = metadata[4];
            plru_update[5] = metadata[5];
            plru_update[6] = metadata[6];
            plru_update[7] = metadata[7];
            if (hit_ways == 0) begin
                plru_update[1] = 1;
                plru_update[2] = 1;
                plru_update[4] = 1;
            end
            if (hit_ways == 1) begin
                plru_update[1] = 1;
                plru_update[2] = 1;
                plru_update[4] = 0;
            end
            if (hit_ways == 2) begin
                plru_update[1] = 1;
                plru_update[2] = 0;
                plru_update[5] = 1;
            end
            if (hit_ways == 3) begin
                plru_update[1] = 1;
                plru_update[2] = 0;
                plru_update[5] = 0;
            end
            if (hit_ways == 4) begin
                plru_update[1] = 0;
                plru_update[3] = 1;
                plru_update[6] = 1;
            end
            if (hit_ways == 5) begin
                plru_update[1] = 0;
                plru_update[3] = 1;
                plru_update[6] = 0;
            end
            if (hit_ways == 6) begin
                plru_update[1] = 0;
                plru_update[3] = 0;
                plru_update[7] = 1;
            end
            if (hit_ways == 7) begin
                plru_update[1] = 0;
                plru_update[3] = 0;
                plru_update[7] = 0;
            end

            if (!plru_mask[0]) begin
                metadata[0] = plru_update[0];
            end
            if (!plru_mask[1]) begin
                metadata[1] = plru_update[1];
            end
            if (!plru_mask[2]) begin
                metadata[2] = plru_update[2];
            end
            if (!plru_mask[3]) begin
                metadata[3] = plru_update[3];
            end
            if (!plru_mask[4]) begin
                metadata[4] = plru_update[4];
            end
            if (!plru_mask[5]) begin
                metadata[5] = plru_update[5];
            end
            if (!plru_mask[6]) begin
                metadata[6] = plru_update[6];
            end
            if (!plru_mask[7]) begin
                metadata[7] = plru_update[7];
            end
        end
    endtask

    reg [`NUM_WAYS-1:0] v_plru_victim;
    task find_plru_victim;
        begin
            v_plru_victim[0] = (metadata[0] && !plru_mask[0]) || (plru_mask[0] && plru_policy[0]);
            v_plru_victim[1] = (metadata[1] && !plru_mask[1]) || (plru_mask[1] && plru_policy[1]);
            v_plru_victim[2] = (metadata[2] && !plru_mask[2]) || (plru_mask[2] && plru_policy[2]);
            v_plru_victim[3] = (metadata[3] && !plru_mask[3]) || (plru_mask[3] && plru_policy[3]);
            v_plru_victim[4] = (metadata[4] && !plru_mask[4]) || (plru_mask[4] && plru_policy[4]);
            v_plru_victim[5] = (metadata[5] && !plru_mask[5]) || (plru_mask[5] && plru_policy[5]);
            v_plru_victim[6] = (metadata[6] && !plru_mask[6]) || (plru_mask[6] && plru_policy[6]);
            v_plru_victim[7] = (metadata[7] && !plru_mask[7]) || (plru_mask[7] && plru_policy[7]);

            if (!v_plru_victim[1]) begin
                if (!v_plru_victim[2]) begin
                    if (!v_plru_victim[4]) begin
                        victim_way = 0;
                    end else begin
                        victim_way = 1;
                    end
                end else begin
                    if (!v_plru_victim[5]) begin
                        victim_way = 2;
                    end else begin
                        victim_way = 3;
                    end
                end
            end else begin
                if (!v_plru_victim[3]) begin
                    if (!v_plru_victim[6]) begin
                        victim_way = 4;
                    end else begin
                        victim_way = 5;
                    end
                end else begin
                    if (!v_plru_victim[7]) begin
                        victim_way = 6;
                    end else begin
                        victim_way = 7;
                    end
                end
            end
        end
    endtask

    task check_hit;
        input [`ADDR_WIDTH-1:0] _addr;
        begin
            if (valid[0] && tags[0] == _addr && policy_hitmap[0]) begin
                hit = 1;
                hit_way = 0;
            end else if (valid[1] && tags[1] == _addr && policy_hitmap[1]) begin
                hit = 1;
                hit_way = 1;
            end else if (valid[2] && tags[2] == _addr && policy_hitmap[2]) begin
                hit = 1;
                hit_way = 2;
            end else if (valid[3] && tags[3] == _addr && policy_hitmap[3]) begin
                hit = 1;
                hit_way = 3;
            end else if (valid[4] && tags[4] == _addr && policy_hitmap[4]) begin
                hit = 1;
                hit_way = 4;
            end else if (valid[5] && tags[5] == _addr && policy_hitmap[5]) begin
                hit = 1;
                hit_way = 5;
            end else if (valid[6] && tags[6] == _addr && policy_hitmap[6]) begin
                hit = 1;
                hit_way = 6;
            end else if (valid[7] && tags[7] == _addr && policy_hitmap[7]) begin
                hit = 1;
                hit_way = 7;
            end else begin
                hit = 0;
            end
        end
    endtask

    always @(posedge clk) begin
        
        if (reset) begin
            tags[0] = 0;
            tags[1] = 0;
            tags[2] = 0;
            tags[3] = 0;
            tags[4] = 0;
            tags[5] = 0;
            tags[6] = 0;
            tags[7] = 0;
            metadata = 0;
            valid = 0;
            policy_hitmap = 0;
            // plru_policy = 0;
            // plru_mask = 0;
            victim_way = 0;
            hit_way = 0;
            hit = 0;
        end else if (os_req) begin
            switch_domain(hitmap);
            hit = 0;
        end else if (user_req) begin
            check_hit(addr);
            if (hit) begin
                update_plru_tree(hit_way);
            end else begin
                find_plru_victim();
                update_plru_tree(victim_way);
                tags[victim_way] = addr;
                valid[victim_way] = 1;
            end
        end
    end

endmodule