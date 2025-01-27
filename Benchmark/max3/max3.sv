module max3(
input [7:0] a,
input [7:0] b,
input [7:0] c,
output [7:0] max_val
);
wire [7:0] ab_max;
assign ab_max = (a >= b) ? a : b;
assign max_val = (ab_max >= c) ? ab_max : c;
endmodule