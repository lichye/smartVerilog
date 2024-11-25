
* *      EBMC - Copyright (C) 2001-2017 Daniel Kroening     * *
* *                     Version 5.2                         * *
* *     University of Oxford, Computer Science Department   * *
* *                  kroening@kroening.com                  * *

Usage:                       Purpose:

 ebmc [-?] [-h] [--help]      show help
 ebmc file ...                source file names

Additonal options:
 --bound nr                   set bound (default: 1)
 --module module              set top module (deprecated)
 --top module                 set top module
 -p expr                      specify a property
 --outfile file name          set output file name (default: stdout)
 --json-result file name      use JSON for property status and traces
 --trace                      generate a trace for failing properties
 --vcd file name              generate traces in VCD format
 --waveform                   show a waveform for failing properties
 --numbered-trace             give a trace with identifiers numbered by 
                              timeframe
 --show-properties            list the properties in the model
 --property id                check the property with given ID
 -I path                      set include path
 --systemverilog              force SystemVerilog instead of Verilog
 --reset expr                 set up module reset
 --liveness-to-safety         translate liveness properties to safety properties

Methods:
 --k-induction                do k-induction with k=bound
 --bdd                        use (unbounded) BDD engine
 --ic3                        use IC3 engine with options described below
    --constr                  use constraints specified in 'file.cnstr'
    --new-mode                new mode is switched on
    --aiger                   print out the instance in aiger format
 --random-traces              generate random traces
    --traces number           generate the given number of traces
    --random-seed number      use the given random seed
    --trace-steps number      set the number of random transitions (default: 10 
                              steps)
 --random-trace               generate a random trace
    --random-seed number      use the given random seed
    --trace-steps number      set the number of random transitions (default: 10 
                              steps)
 --random-waveform            generate a random trace and show it in horizontal 
                              form
    --random-seed number      use the given random seed
    --trace-steps number      set the number of random transitions (default: 10 
                              steps)
 --ranking-function f         prove a liveness property using given ranking 
                              funnction (experimental)
    --property id             the liveness property to prove
 --neural-liveness            check liveness properties using neural inference 
                              (experimental)
    --neural-engine cmd       the neural engine to use

Solvers:
 --aig                        bit-level SAT with AIGs
 --dimacs                     output bit-level CNF in DIMACS format
 --smt2                       output word-level SMT 2 formula
 --boolector                  use Boolector as solver
 --cvc4                       use CVC4 as solver
 --mathsat                    use MathSAT as solver
 --yices                      use Yices as solver
 --z3                         use Z3 as solver

Debugging options:
 --preprocess                 output the preprocessed source file
 --show-parse                 show parse trees
 --show-modules               show a list of the modules
 --show-module-hierarchy      show the hierarchy of module instantiations
 --show-varmap                show variable map
 --show-netlist               show netlist
 --show-ldg                   show latch dependencies
 --show-formula               show the formula that is generated
 --smv-netlist                show netlist in SMV format
 --dot-netlist                show netlist in DOT format
 --show-trans                 show transition system
 --verbosity #                verbosity level, from 0 (silent) to 10 
                              (everything)

