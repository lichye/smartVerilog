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
	reg [7:0] dataa_copy;
	reg [7:0] datab_copy;

	always @ (posedge clk)
	begin
		if (add_sub) 
		begin
			result <= dataa + datab;
			dataa_copy <= dataa;
			datab_copy <= datab;
		end
		else
			result <= dataa - datab;
	end

	p1: assert property (@(posedge clk) 
	((add_sub |-> ##1(result == dataa_copy + datab_copy)))
	);


	// always @ (posedge clk) begin
	// 	if (add_sub) begin
	// 		assert(result == dataa_copy + datab_copy);
	// 	end
	// end

endmodule
