//This example come from Intel Website
//https://www.intel.com/content/www/us/en/support/programmable/support-resources/design-examples/horizontal/ver-add-sub.html
module addsub
(
	input [7:0] dataa,
	input [7:0] datab,
	input add_sub,	  // if this is 1, add; else subtract
	input clk,
	output reg [7:0] result
);
	always @ (posedge clk)
	begin
		if (add_sub) 
			result <= dataa + datab;			
		else
			result <= dataa - datab;
	end

endmodule
