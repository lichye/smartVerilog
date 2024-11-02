module my_add(input a, b, output [1:0] y);

  assign y[0]=a^b;
  assign y[1]=a&b;

endmodule

module main(input a, b);

  wire [1:0] result;

  my_add(a, b, result);

  always assert p1: (a+b==result);

endmodule