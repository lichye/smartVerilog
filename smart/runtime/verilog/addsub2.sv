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

	always @ (posedge clk) begin
		dataa_copy <= dataa;
		datab_copy <= datab;
	end

	always @ (posedge clk)
	begin
		if (add_sub) 
			result <= dataa + datab;			
		else
			result <= dataa - datab;
	end

	//This the property that we want to verify
	//unreachableState: assert property (@(posedge clk) clk|->##1!(dataa_copy== 8'b00000001 && datab_copy== 8'b00000001 && result == 8'b00000100));

	// //this will be refuted
	// p2: assert property (@(posedge clk) !(dataa_copy== 8'b00000001 && datab_copy== 8'b00000001 && result == 8'b00000000));

	// addproperty: assert property (@(posedge clk) add_sub |-> ##1 result == dataa_copy + datab_copy);

	// subproperty: assert property (@(posedge clk) !add_sub |-> ##1 result == dataa_copy - datab_copy);
	
	add_sub2: assert property (@(posedge clk) clk |-> ##1 add_sub);

endmodule
