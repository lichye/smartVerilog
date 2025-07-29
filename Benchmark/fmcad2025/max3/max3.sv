module max3(
  input  [7:0] a,
  input  [7:0] b,
  input  [7:0] c,
  output [7:0] result
);
  wire   [7:0] d;
  // d = max(a, b)
  assign d = (a>=b) ? a:b;
  // result = max(d, c)
  assign result = (d>=c) ? d:c;
  

endmodule