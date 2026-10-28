# ============================================================
# Makefile: BRCA1-ONSET / CellSim Simulation Pipeline
# ============================================================

.PHONY: build rebuild test clean bootstrap-quick bootstrap-paper calibration-paper calibration-pilot calibration-dry-run sensitivity-global-lhs sensitivity-unaccelerated exp01-fig2 exp01-fig2-grid exp01-fig2-100years exp01-fig2-grid-100years exp01-fig2-paper exp01-simulate exp01-smoke exp02-sfig1 exp02-smoke exp02-simulate exp06-fig6 exp06-sfig7 exp06-smoke exp06-validate tissue-scale-build

# Compilación limpia y construcción con soporte OpenMP y tests
rebuild:
	cmake -S . -B build -DBUILD_TESTS=ON
	cmake --build build -j

build:
	cmake --build build -j

# Ejecución de la suite completa de tests unitarios
test:
	ctest --test-dir build --output-on-failure

clean:
	rm -rf build

# ============================================================
# Rebanadas Verticales de Experimentos y Figuras del Paper
# ============================================================

# Experimento 01: Exploración preliminar y aceleración tumoral (Figura 2)
exp01-fig2:
	@echo "📊 Generando Figura 2 Simulada (PDF y PNG)..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode simulated

exp01-fig2-grid:
	@echo "📊 Generando Figura 2 Extra (con nube gris de las 36 opciones del grid)..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode simulated --show-grid

exp01-fig2-grid-d001:
	@echo "📊 Generando Figura 2 Extra para d=0.01 (representativas + grid)..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode simulated --show-grid --data-suffix _d001 --output-name Figure2_extra_grid_d0.01

exp01-fig2-100years:
	@echo "📊 Generando Figura 2 extendida a 100 años (PDF y PNG)..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode simulated --max-age 100 --output-name Figure2_100years

exp01-fig2-grid-100years:
	@echo "📊 Generando Figura 2 Extra con grid completo extendido a 100 años..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode simulated --max-age 100 --show-grid

exp01-fig2-paper:
	@echo "📊 Generando Figura 2 del Paper (PDF y PNG)..."
	python3 experiments/exp01_exploratory_acceleration/plot_figure2.py --mode paper

exp01-simulate: build
	@echo "🔬 Ejecutando simulaciones estocásticas para Figura 2 (N=1000)..."
	python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --generate-trajectories --n-runs 1000

exp01-simulate-d001: build
	@echo "🔬 Ejecutando simulaciones estocásticas para d=0.01 (representativas + grid, N=1000)..."
	python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --generate-trajectories --division-rate 0.01 --file-suffix _d001 --n-runs 1000
	python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --generate-grid-trajectories --division-rate 0.01 --file-suffix _d001 --n-runs 1000

exp01-smoke: build
	@echo "🧪 Verificando pipeline de simulación del Experimento 01..."
	python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --smoke-test

# Experimento 02: Análisis global de sensibilidad LHS y PRCC (Figura Suplementaria S1)
exp02-sfig1:
	@echo "📊 Generando Figura Suplementaria S1 (PDF y PNG)..."
	python3 experiments/exp02_sensitivity_global/plot_sfig1.py

exp02-smoke: build
	@echo "🧪 Verificando pipeline de sensibilidad del Experimento 02 (LHS smoke-test)..."
	python3 experiments/exp02_sensitivity_global/run_lhs_sensitivity.py --smoke-test

exp02-simulate: build
	@echo "🔬 Ejecutando campaña completa de sensibilidad global LHS (1,000 combinaciones x 200 simulaciones)..."
	python3 experiments/exp02_sensitivity_global/run_lhs_sensitivity.py --n-samples 1000 --n-sim 200

# Experimento 03: Calibración Bayesiana ABC-SMC y Posterior Predictive (Figura 3 y Figura S4)
exp03-fig3:
	@echo "📊 Generando Figura 3 (Performance ABC-SMC & Posterior Predictive)..."
	python3 experiments/exp03_abc_smc_calibration/plot_figure3.py

exp03-sfig4:
	@echo "📊 Generando Figuras Suplementarias S4, S4b y S4c (Posterior Canónica, Ratios Cruzados & Auto-ratios)..."
	python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant all

exp03-sfig4b:
	@echo "📊 Generando Figura Suplementaria S4b (Ratios Cruzados Normalizados por Beta)..."
	python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant 4b

exp03-sfig4c:
	@echo "📊 Generando Figura Suplementaria S4c (Auto-ratios Normalizados por Beta)..."
	python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant 4c

exp03-smoke: build
	@echo "🧪 Verificando pipeline ABC-SMC del Experimento 03 (smoke-test rápido)..."
	python3 experiments/exp03_abc_smc_calibration/abc_smc.py --smoke-test

exp03-abc-pilot: build
	@echo "⚡ Ejecutando calibración ABC-SMC piloto (50 partículas x 4 generaciones)..."
	python3 experiments/exp03_abc_smc_calibration/abc_smc.py --particles 50 --generations 4 --n-sim 100 --workers 4

exp03-simulate: build
	@echo "🔬 Ejecutando calibración canónica ABC-SMC (250 partículas x 20 generaciones)..."
	python3 experiments/exp03_abc_smc_calibration/abc_smc.py --particles 250 --generations 20 --n-sim 300 --workers 8

# Experimento 04: Características Emergentes del Tumor y Linajes Clonales (Figura 4)
exp04-fig4:
	@echo "📊 Generando Figura 4 (Posterior Parameters, BRCA1 Composition & Founder Lineages)..."
	python3 experiments/exp04_emergent_features/plot_figure4.py

exp04-smoke: build
	@echo "🧪 Verificando pipeline de características emergentes del Experimento 04..."
	python3 experiments/exp04_emergent_features/run_emergent_features.py --smoke-test

exp04-simulate: build
	@echo "🔬 Simulando características emergentes del tumor a través de la población posterior..."
	python3 experiments/exp04_emergent_features/run_emergent_features.py

# Experimento 05: Intervención e Intercepción de Cáncer (Figura 5 y Figura S5)
exp05-fig5:
	@echo "📊 Generando Figura 5 (Cancer Interception, Timing & Dose-Response)..."
	python3 experiments/exp05_cancer_interception/plot_figure5_sfig5.py

exp05-sfig5:
	@echo "📊 Generando Figura Suplementaria S5 (D1-D2 Sub-additive Interaction)..."
	python3 experiments/exp05_cancer_interception/plot_figure5_sfig5.py

exp05-smoke: build
	@echo "🧪 Verificando pipeline de intercepción del Experimento 05..."
	python3 experiments/exp05_cancer_interception/run_interception.py --smoke-test

exp05-simulate: build
	@echo "🔬 Ejecutando simulación de cohortes de intercepción..."
	python3 experiments/exp05_cancer_interception/run_interception.py

# ============================================================
# Experimento 06: Modelo a Escala de Tejido (10^7 Células, Figuras 6 y S7)
# ============================================================

tissue-scale-build:
	@echo "🔨 Compilando simulador C++20 de escala de tejido (tissue_scale_sim)..."
	@$(MAKE) -C tissue_scale

exp06-fig6:
	@echo "📊 Generando Figura 6 (Penetrancia, Estado BRCA1 y Linajes Fundadores)..."
	python3 tissue_scale/scripts/make_figure6_sfig6.py --output-dir tissue_scale/results

exp06-simulate: tissue-scale-build
	@echo "🔬 Ejecutando simulación estocástica ad-hoc de tejidos para Figura 6 (Panels d y e)..."
	python3 tissue_scale/scripts/simulate_figure6_data.py

exp06-sfig7:
	@echo "📊 Generando Figura Suplementaria S7 (Ajuste y Trade-off de Modelo Homogéneo)..."
	python3 tissue_scale/scripts/make_sfig7.py --output-dir tissue_scale/results

exp06-smoke: tissue-scale-build
	@echo "🧪 Ejecutando smoke test rápido del simulador a escala de tejido (N=10)..."
	python3 tissue_scale/scripts/run_validation.py --smoke-test --output tissue_scale/results/smoke.csv
	python3 tissue_scale/scripts/summarize_validation.py --input tissue_scale/results/smoke.csv

exp06-validate:
	@echo "📋 Resumiendo métricas del dataset congelado de 10.000 tejidos..."
	python3 tissue_scale/scripts/summarize_validation.py --input tissue_scale/data/final_multiobjective_validation_10000.csv


# ============================================================
# Targets Directos de Bootstrapping (C++ Monte Carlo)
# ============================================================

# Simulación rápida (N=50 tejidos virtuales, ~1-2 segundos) con los parámetros del paper
bootstrap-quick: build
	@echo "🧬 Ejecutando simulación rápida de bootstrapping (N=50 tejidos, modo acelerado)..."
	./build/run_bootstrapping \
		--n-runs 50 \
		--n-cells 500 \
		--brca1-rate 0.0457 \
		--low-delta 0.104 \
		--high-delta 0.392 \
		--neoplastic-div-rate 0.159 \
		--big-bang \
		--weighted-sse \
		--verbose

# Simulación canónica (N=1,000 tejidos virtuales, evaluación posterior de penetrancia)
bootstrap-paper: build
	@echo "🧬 Ejecutando evaluación completa de penetrancia (N=1,000 tejidos, parámetros posteriores)..."
	./build/run_bootstrapping \
		--n-runs 1000 \
		--n-cells 500 \
		--brca1-rate 0.0457 \
		--low-delta 0.104 \
		--high-delta 0.392 \
		--neoplastic-div-rate 0.159 \
		--big-bang \
		--weighted-sse \
		--output bootstrapping_results.csv

# ============================================================
# Targets de Calibración y Sensibilidad Activos
# ============================================================

# Smoke test instantáneo de verificación del pipeline
calibration-dry-run: build
	@echo "🧪 Verificando pipeline ABC-SMC (--dry-run)..."
	python3 scripts/experiments/exp_01_abc_smc_global.py --dry-run --workers 2

# Análisis de sensibilidad piloto rápido (50 partículas x 4 generaciones)
calibration-pilot: build
	@echo "⚡ Iniciando Análisis de Sensibilidad ABC-SMC Piloto (50 partículas x 4 generaciones)..."
	@mkdir -p results/20260928_draft_map/sensitivity_abc_pilot
	python3 scripts/experiments/exp_01_abc_smc_global.py \
		--particles 50 \
		--generations 4 \
		--n-sim 100 \
		--n-reps 1 \
		--workers 4 \
		--output-dir results/20260928_draft_map/sensitivity_abc_pilot

# Calibración canónica de referencia (250 partículas x 20 generaciones)
calibration-paper: build
	@echo "🔬 Calibración canónica de referencia..."
	@mkdir -p results/20260928_draft_map/exp_01_abc_smc_global
	python3 scripts/experiments/exp_01_abc_smc_global.py \
		--particles 250 \
		--generations 20 \
		--n-sim 300 \
		--n-reps 1 \
		--workers 4 \
		--output-dir results/20260928_draft_map/exp_01_abc_smc_global

# Global Sensitivity Analysis LHS (1,000 combinaciones) -> Fig. S1
sensitivity-global-lhs: build
	@echo "🌐 Ejecutando Global Sensitivity Analysis (1,000 LHS parameter combinations)..."
	@mkdir -p results/20260928_draft_map/exp_02_global_sensitivity
	python3 scripts/experiments/exp_02_global_sensitivity_prcc.py \
		--n-samples 1000 \
		--n-sim 200 \
		--workers 4 \
		--output-dir results/20260928_draft_map/exp_02_global_sensitivity

# Contraste de ablación sin aceleración tumoral (nu = d, Fig. 2)
sensitivity-unaccelerated: build
	@echo "📉 Evaluando modelo sin aceleración tumoral (locked division nu = d)..."
	@mkdir -p results/20260928_draft_map/sensitivity_unaccelerated
	python3 scripts/experiments/exp_01_abc_smc_locked_division.py \
		--particles 100 \
		--generations 4 \
		--n-sim 100 \
		--n-reps 1 \
		--workers 4 \
		--output-dir results/20260928_draft_map/sensitivity_unaccelerated
