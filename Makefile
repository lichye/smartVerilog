ENGINE ?= docker
IMAGE ?= smartverilog-local
PULL_IMAGE ?= magna2024/smart
DOCKERFILE ?= Docker/Dockerfile
CONTAINER_NAME ?= smart-dev
PROJECT_ROOT := $(shell pwd)
RESULTS_DIR := $(PROJECT_ROOT)/Results
CONFIG_DIR := $(PROJECT_ROOT)/Config
CONFIG ?= Config/smart.json

.PHONY: docker-builder pull shell shell-root run md clean clean-results

docker-builder:
	@$(ENGINE) build \
		-f $(DOCKERFILE) \
		-t $(IMAGE) \
		$(PROJECT_ROOT)

pull:
	@$(ENGINE) pull $(PULL_IMAGE)

shell:
	@mkdir -p $(RESULTS_DIR)
	@$(ENGINE) run --rm -it \
		--name $(CONTAINER_NAME) \
		-w /workspace/smartVerilog \
		-v $(RESULTS_DIR):/workspace/smartVerilog/Results:Z \
		-v $(CONFIG_DIR):/workspace/smartVerilog/Config:ro,Z \
		$(IMAGE) /bin/bash

shell-root:
	@mkdir -p $(RESULTS_DIR)
	@$(ENGINE) run --rm -it \
		--user root \
		--name $(CONTAINER_NAME) \
		-w /workspace/smartVerilog \
		-v $(RESULTS_DIR):/workspace/smartVerilog/Results:Z \
		-v $(CONFIG_DIR):/workspace/smartVerilog/Config:ro,Z \
		$(IMAGE) /bin/bash

run:
	@mkdir -p $(RESULTS_DIR)
	@$(ENGINE) run --rm -it \
		-w /workspace/smartVerilog \
		-v $(RESULTS_DIR):/workspace/smartVerilog/Results:Z \
		-v $(CONFIG_DIR):/workspace/smartVerilog/Config:ro,Z \
		$(IMAGE) python run.py $(BENCH) $(CONFIG)

md:
	@mkdir -p $(RESULTS_DIR)
	@$(ENGINE) run --rm -it \
		-w /workspace/smartVerilog \
		-v $(RESULTS_DIR):/workspace/smartVerilog/Results:Z \
		-v $(CONFIG_DIR):/workspace/smartVerilog/Config:ro,Z \
		$(IMAGE) python md.py $(BENCH) $(CONFIG)

clean:
	@rm -rf smart/*.txt
	@rm -rf smart/*.sby
	@rm -rf smart/*task
	@rm -rf smart/*.log
	@rm -rf smart/*.sl
	@rm -rf smart/result/*
	@rm -rf smart/src/python/__pycache__
	@$(MAKE) -C smart all_clean
	@rm -f md.dat
	@rm -f assertions.dat
	@rm -f assertions_found.dat
	@rm -f runtime.dat
	@rm -f md_rate_flat.csv
	@rm -f table_block_msa.tex
	@rm -f table_block_msa_mini.tex

clean-results:
	@rm -rf $(RESULTS_DIR)
