Have_done:
# Make one botton run shell
# Make Unreachable state from Traces

Wokring_On:
# rebuild Traces and Sygus gather with states

# Make Verilog Expr
# Merge Assertion and verilog design and feed to the EBMC

To_Be_Dis:
#   EBMC does not take cmdline of sv assertions
### merging assertions files with verilog files
#   TIME problem

To_Do:
#   Make program that can make verilog assertions

## input: smt results in smt format
### requires all type system of SMT
### https://github.com/polgreen/oracles/blob/master/image_oracles/define_fun_parser.h

## input: identifer, value type???

## output: system verilog assertions-- Verilog - Expr
### requires all type system of Verilog

##  Merge verilog code with the assertion
##  Translate smt function to verilog assertions
##  Translate a state into verilog assertions

Future:
#   Mutate the verilog circut and check the assertion is violate

#   Automatic variables poicker for sygus
##  Constrainst the variables
##  Huristic

#   Make more test file (?)
##  Use the open-source benchmark

Sideway_Work: 
##  There is possible way to adjust to K-induction prove