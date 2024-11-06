ebmc addsub.sv --module addsub --bound 10 --vcd smt.vcd -p "add_sub==1"


	// p1: assert property (@(posedge clk) 
	// ((add_sub |-> ##1(result == dataa_copy + datab_copy)))
	// );


	// always @ (posedge clk) begin
	// 	if (add_sub) begin
	// 		assert(result == dataa_copy + datab_copy);
	// 	end
	// end

	//resultNoZero: assert property (@(posedge clk) ((result != 8'h00)));