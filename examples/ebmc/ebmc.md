# bound checking

ebmc example3.sv --module counter --bound 10 --vcd example3.vcd

ebmc add.v --top main --bound 10 --vcd add.vcd



# unbound checking

## use bdd
ebmc ring_buffer.sv --top ring_buffer --bdd 

## use k-induction
ebmc ring_buffer.sv --top ring_buffer --k-induction 