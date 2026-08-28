CMAKE ?= cmake
BUILD_DIR ?= build
JOBS ?= $(shell nproc)
BENCH ?= c17
CONFIG ?= Config/smart.json

.PHONY: build test check-env run smoke clean distclean clean-results

build:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) -j$(JOBS)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

check-env: build
	./$(BUILD_DIR)/smart --check-env

run: build
	python3 run.py $(BENCH) --config $(CONFIG)

smoke: build
	tools/smoke.sh $(JOBS)

# Only generated files are removed. Research data, fixed mutants, and locally
# installed tools deliberately remain outside this target.
clean:
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		$(CMAKE) --build "$(BUILD_DIR)" --target clean; \
	fi
	@rm -rf __pycache__ .pytest_cache smart-work-* \
		tools/__pycache__ tools/.pytest_cache \
		tools/experiments/__pycache__ tools/experiments/.pytest_cache

distclean: clean
	@rm -rf $(BUILD_DIR) dist smoke-out

# Deliberately separate: experiment results are evidence, not build output.
clean-results:
	@rm -rf Results
