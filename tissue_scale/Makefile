# ============================================================================
# BRCA1-ONSET Tissue-Scale Model (10^7 cells, Figure 6, Figure S6, Figure S7)
# ============================================================================
CXX ?= g++
CXXFLAGS ?= -O3 -std=c++20 -Wall -Wextra
OPENMP ?= -fopenmp

BIN = bin/tissue_scale_sim
SRC = src/tissue_scale_sim.cpp
HEADER = src/TissueScaleSimulator.h

all: $(BIN)

$(BIN): $(SRC) $(HEADER)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(OPENMP) $(SRC) -o $(BIN)

fig6:
	python3 scripts/make_figure6_sfig6.py --output-dir results

sfig7:
	python3 scripts/make_sfig7.py --output-dir results

smoke: $(BIN)
	python3 scripts/run_validation.py --smoke-test --output results/smoke.csv
	python3 scripts/summarize_validation.py --input results/smoke.csv

validate-10k:
	python3 scripts/summarize_validation.py --input data/final_multiobjective_validation_10000.csv

clean:
	rm -rf bin results/smoke.csv

.PHONY: all fig6 sfig7 smoke validate-10k clean
