module timer (clk,rst,flush);

input clk, rst;
output reg flush;
reg[3:0] count;

always @(posedge clk) begin
    if (rst) begin
        count <= 4'b0000;
        flush <= 0;
    end else begin
        count <= count + 1; 
    end
end

always @(posedge clk) begin
    if (count == 0) begin
        flush <= 1;  
    end
end

endmodule
