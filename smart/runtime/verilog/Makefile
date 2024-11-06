clean:
	rm -rf *.swp
	rm -rf sim_build
	rm -rf __pycache__
sim:
	python test.py
	mv sim_build/dump.vcd addsub.vcd

c1:
	ebmc addsub.sv --module addsub --bound 10 --vcd smt.vcd -p "add_sub==1"

c2:
	ebmc addsub.sv --module addsub --bound 10 --vcd smt.vcd -p "datab!=0"

state:
	ebmc add_sub2.sv --module add_sub --bound 10 --trace