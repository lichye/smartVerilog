clean:
	rm -rf *.swp
	rm -rf sim_build
	rm -rf __pycache__
sim:
	python test.py
	mv sim_build/dump.vcd sim.vcd

c1:
	ebmc addsub.sv --module addsub --bound 10 --vcd smt.vcd -p "add_sub==1"

c2:
	ebmc addsub.sv --module addsub --bound 10 --vcd smt.vcd -p "datab!=0"

