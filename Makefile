# ============================================================
# Makefile: BRCA1-ONSET / CellSim Simulation Pipeline
# ============================================================

.PHONY: build rebuild test clean exp-smoke-all paper-figures \
	exp01-cs-fig2 exp01-cs-simulate exp01-cs-smoke exp01-fig2 exp01-simulate exp01-smoke \
	exp02-cs-sfig1 exp02-cs-simulate exp02-cs-smoke exp02-sfig1 exp02-simulate exp02-smoke \
	exp03-cs-fig3 exp03-cs-sfig4 exp03-cs-sfig4b exp03-cs-sfig4-all exp03-cs-smoke exp03-cs-abc-pilot exp03-cs-simulate exp03-fig3 exp03-sfig4 exp03-sfig4b exp03-smoke exp03-simulate \
	exp04-cs-fig4 exp04-cs-simulate exp04-cs-smoke exp04-fig4 exp04-simulate exp04-smoke \
	exp05-cs-fig5 exp05-cs-sfig5 exp05-cs-simulate exp05-cs-smoke exp05-fig5 exp05-sfig5 exp05-simulate exp05-smoke \
	exp06-cs-sfig2 exp06-cs-sfig3 exp06-cs-simulate exp06-cs-smoke exp06-sfig2 exp06-sfig3 \
	exp07-ts-fig6 exp07-ts-simulate exp07-ts-smoke exp07-fig6 exp07-simulate exp07-smoke \
	exp08-ts-sfig6 exp08-ts-smoke exp08-sfig6 exp08-smoke \
	exp09-ts-sfig7 exp09-ts-simulate exp09-ts-smoke exp09-sfig7 exp09-simulate exp09-smoke \
	bootstrap-quick bootstrap-paper calibration-paper calibration-pilot calibration-dry-run sensitivity-global-lhs sensitivity-unaccelerated

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
# Rebanadas Verticales de Experimentos (9 Experimentos Modulares)
# ============================================================

# ------------------------------------------------------------
# Experimento 01 (CS): Exploración preliminar y aceleración (Figura 2)
# ------------------------------------------------------------
exp01-cs-fig2:
	@echo "📊 Generando Figura 2 Canónica (PDF y PNG)..."
	python3 experiments/exp01_cs_exploratory_acceleration/plot_figure2.py

exp01-cs-simulate: build
	@echo "🔬 Ejecutando simulaciones estocásticas para Figura 2 (N=500)..."
	python3 experiments/exp01_cs_exploratory_acceleration/run_exploratory.py --generate-trajectories --n-runs 500

exp01-cs-smoke: build
	@echo "🧪 Verificando pipeline de simulación del Experimento 01..."
	python3 experiments/exp01_cs_exploratory_acceleration/run_exploratory.py --smoke-test

exp01-fig2: exp01-cs-fig2
exp01-simulate: exp01-cs-simulate
exp01-smoke: exp01-cs-smoke

# ------------------------------------------------------------
# Experimento 02 (CS): Análisis global de sensibilidad LHS/PRCC (Figura S1)
# ------------------------------------------------------------
exp02-cs-sfig1:
	@echo "📊 Generando Figura Suplementaria S1 (PDF y PNG)..."
	python3 experiments/exp02_cs_sensitivity_global/plot_sfig1.py

exp02-cs-smoke: build
	@echo "🧪 Verificando pipeline de sensibilidad del Experimento 02 (LHS smoke-test)..."
	python3 experiments/exp02_cs_sensitivity_global/run_lhs_sensitivity.py --smoke-test

exp02-cs-simulate: build
	@echo "🔬 Ejecutando campaña completa de sensibilidad global LHS (1,000 combinaciones x 200 simulaciones)..."
	python3 experiments/exp02_cs_sensitivity_global/run_lhs_sensitivity.py --n-samples 1000 --n-sim 200

exp02-sfig1: exp02-cs-sfig1
exp02-smoke: exp02-cs-smoke
exp02-simulate: exp02-cs-simulate

# ------------------------------------------------------------
# Experimento 03 (CS): Calibración Bayesiana ABC-SMC (Figuras 3 y S4)
# ------------------------------------------------------------
exp03-cs-fig3:
	@echo "📊 Generando Figura 3 (Performance ABC-SMC & Posterior Predictive)..."
	python3 experiments/exp03_cs_abc_smc_calibration/plot_figure3.py

exp03-cs-sfig4:
	@echo "📊 Generando Figura Suplementaria S4 Canónica (Posterior Corner Matrix 4x4)..."
	python3 experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py --variant 4a

exp03-cs-sfig4b:
	@echo "📊 Generando Figura Suplementaria S4b (Densidades 1D de los 6 Pares de Parámetros)..."
	python3 experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py --variant 4b

exp03-cs-sfig4-all:
	@echo "📊 Generando todas las figuras suplementarias S4 (S4 y S4b)..."
	python3 experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py --variant all

exp03-cs-smoke: build
	@echo "🧪 Verificando pipeline ABC-SMC del Experimento 03 (smoke-test rápido)..."
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py --smoke-test

exp03-cs-abc-pilot: build
	@echo "⚡ Ejecutando calibración ABC-SMC piloto (50 partículas x 4 generaciones)..."
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py --particles 50 --generations 4 --n-sim 100 --workers 4

exp03-cs-simulate: build
	@echo "🔬 Ejecutando calibración canónica ABC-SMC (250 partículas x 20 generaciones)..."
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py --particles 250 --generations 20 --n-sim 300 --workers 8

exp03-fig3: exp03-cs-fig3
exp03-sfig4: exp03-cs-sfig4
exp03-sfig4b: exp03-cs-sfig4b
exp03-smoke: exp03-cs-smoke
exp03-simulate: exp03-cs-simulate

# ------------------------------------------------------------
# Experimento 04 (CS): Características Emergentes del Tumor (Figura 4)
# ------------------------------------------------------------
exp04-cs-fig4:
	@echo "📊 Generando Figura 4 (Posterior Parameters, BRCA1 Composition & Founder Lineages)..."
	python3 experiments/exp04_cs_emergent_features/plot_figure4.py

exp04-cs-smoke: build
	@echo "🧪 Verificando pipeline de características emergentes del Experimento 04..."
	python3 experiments/exp04_cs_emergent_features/run_emergent_features.py --smoke-test

exp04-cs-simulate: build
	@echo "🔬 Simulando características emergentes del tumor..."
	python3 experiments/exp04_cs_emergent_features/run_emergent_features.py

exp04-fig4: exp04-cs-fig4
exp04-smoke: exp04-cs-smoke
exp04-simulate: exp04-cs-simulate

# ------------------------------------------------------------
# Experimento 05 (CS): Intervención e Intercepción de Cáncer (Figuras 5 y S5)
# ------------------------------------------------------------
exp05-cs-fig5:
	@echo "📊 Generando Figura 5 (Cancer Interception, Timing & Dose-Response)..."
	python3 experiments/exp05_cs_cancer_interception/plot_figure5_sfig5.py

exp05-cs-sfig5:
	@echo "📊 Generando Figura Suplementaria S5 (D1-D2 Sub-additive Interaction)..."
	python3 experiments/exp05_cs_cancer_interception/plot_figure5_sfig5.py

exp05-cs-smoke: build
	@echo "🧪 Verificando pipeline de intercepción del Experimento 05..."
	python3 experiments/exp05_cs_cancer_interception/run_interception.py --smoke-test

exp05-cs-simulate: build
	@echo "🔬 Ejecutando simulación de cohortes de intercepción..."
	python3 experiments/exp05_cs_cancer_interception/run_interception.py --simulate

exp05-fig5: exp05-cs-fig5
exp05-sfig5: exp05-cs-sfig5
exp05-smoke: exp05-cs-smoke
exp05-simulate: exp05-cs-simulate

# ------------------------------------------------------------
# Experimento 06 (CS): Sensibilidad Estructural (Figuras S2 y S3)
# ------------------------------------------------------------
exp06-cs-sfig2:
	@echo "📊 Generando Figura Suplementaria S2 (Sensibilidad OAT de Parámetros Fijos)..."
	python3 experiments/exp06_cs_sensitivity_structural/plot_sfig2.py

exp06-cs-sfig3:
	@echo "📊 Generando Figura Suplementaria S3 (Sensibilidad Estructural Kappa y TP53)..."
	python3 experiments/exp06_cs_sensitivity_structural/plot_sfig3.py

exp06-cs-smoke: build
	@echo "🧪 Verificando pipeline de sensibilidad estructural (smoke test)..."
	python3 experiments/exp06_cs_sensitivity_structural/run_structural_sensitivity.py --smoke-test

exp06-cs-simulate: build
	@echo "🔬 Ejecutando simulaciones estocásticas para Figuras S2 y S3 (N=300)..."
	python3 experiments/exp06_cs_sensitivity_structural/run_structural_sensitivity.py --simulate --n-runs 300

exp06-sfig2: exp06-cs-sfig2
exp06-sfig3: exp06-cs-sfig3
exp06-smoke: exp06-cs-smoke
exp06-simulate: exp06-cs-simulate

# ------------------------------------------------------------
# Experimento 07 (TS): Validación Multiobjetivo a Escala de Tejido (Figura 6)
# ------------------------------------------------------------
exp07-ts-fig6:
	@echo "📊 Generando Figura 6 (5 paneles a-e, Escala de Tejido)..."
	python3 experiments/exp07_ts_multiobjective_cohort/plot_figure6.py

exp07-ts-smoke: build
	@echo "🧪 Verificando pipeline a escala de tejido del Experimento 07..."
	python3 experiments/exp07_ts_multiobjective_cohort/simulate_figure6_data.py --smoke-test

exp07-ts-simulate: build
	@echo "🔬 Ejecutando simulación estocástica completa de tejidos para Figura 6..."
	python3 experiments/exp07_ts_multiobjective_cohort/simulate_figure6_data.py

exp07-fig6: exp07-ts-fig6
exp07-smoke: exp07-ts-smoke
exp07-simulate: exp07-ts-simulate

# ------------------------------------------------------------
# Experimento 08 (TS): Restricción Homogénea a Escala de Tejido (Figura S6)
# ------------------------------------------------------------
exp08-ts-sfig6:
	@echo "📊 Generando Figura Suplementaria S6 (Trade-off de Restricción Homogénea)..."
	python3 experiments/exp08_ts_homogeneous_restraint/plot_sfig6.py

exp08-ts-smoke:
	@echo "🧪 Verificando generación de Figura S6..."
	python3 experiments/exp08_ts_homogeneous_restraint/plot_sfig6.py

exp08-sfig6: exp08-ts-sfig6
exp08-smoke: exp08-ts-smoke

# ------------------------------------------------------------
# Experimento 09 (TS): Relajación de Checkpoints por Driver Somático (Figura S7)
# ------------------------------------------------------------
exp09-ts-sfig7:
	@echo "📊 Generando Figura Suplementaria S7 (Heatmaps de Relajación de Checkpoints)..."
	python3 experiments/exp09_ts_checkpoint_relaxation/plot_sfig7.py

exp09-ts-smoke: build
	@echo "🧪 Verificando grid de relajación de checkpoints (smoke test)..."
	python3 experiments/exp09_ts_checkpoint_relaxation/run_checkpoint_grid.py --smoke-test --output /tmp/sfig7_smoke.csv

exp09-ts-simulate: build
	@echo "🔬 Ejecutando grid completo de 24 condiciones para Figura S7 (N=3,000 por celda)..."
	python3 experiments/exp09_ts_checkpoint_relaxation/run_checkpoint_grid.py --n-sim 3000

exp09-sfig7: exp09-ts-sfig7
exp09-smoke: exp09-ts-smoke
exp09-simulate: exp09-ts-simulate

# ============================================================
# Smoke-Test Global de Todos los Experimentos (< 30s)
# ============================================================
exp-smoke-all: build exp01-cs-smoke exp02-cs-smoke exp03-cs-smoke exp04-cs-smoke exp05-cs-smoke exp06-cs-smoke exp07-ts-smoke exp08-ts-smoke exp09-ts-smoke
	@echo "✅ Todos los smoke tests (exp01 a exp09) se completaron con éxito."

# ============================================================
# Sincronización Centralizada de Figuras del Paper
# ============================================================
paper-figures:
	@echo "📊 Sincronizando y exportando todas las figuras canónicas del paper..."
	python3 results/paper_figures/export_all_figures.py

# ============================================================
# Targets Directos de Bootstrapping (C++ Monte Carlo)
# ============================================================

bootstrap-quick: build
	@echo "🧬 Ejecutando simulación rápida de bootstrapping (N=50 tejidos)..."
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

bootstrap-paper: build
	@echo "🧬 Ejecutando evaluación completa de penetrancia (N=1,000 tejidos)..."
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

calibration-dry-run: build
	@echo "🧪 Verificando pipeline ABC-SMC (--smoke-test)..."
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py --smoke-test --workers 2

calibration-pilot: build
	@echo "⚡ Iniciando Análisis de Sensibilidad ABC-SMC Piloto (50 partículas x 4 generaciones)..."
	@mkdir -p results/20260928_draft_map/sensitivity_abc_pilot
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py \
		--particles 50 \
		--generations 4 \
		--n-sim 100 \
		--workers 4 \
		--output-dir results/20260928_draft_map/sensitivity_abc_pilot

calibration-paper: build
	@echo "🔬 Calibración canónica de referencia..."
	@mkdir -p results/20260928_draft_map/exp_01_abc_smc_global
	python3 experiments/exp03_cs_abc_smc_calibration/abc_smc.py \
		--particles 250 \
		--generations 20 \
		--n-sim 300 \
		--workers 8 \
		--output-dir results/20260928_draft_map/exp_01_abc_smc_global

sensitivity-global-lhs: build
	@echo "🌐 Ejecutando Global Sensitivity Analysis (1,000 LHS parameter combinations)..."
	@mkdir -p results/20260928_draft_map/exp_02_global_sensitivity
	python3 experiments/exp02_cs_sensitivity_global/run_lhs_sensitivity.py \
		--n-samples 1000 \
		--n-sim 200 \
		--workers 8 \
		--output-dir results/20260928_draft_map/exp_02_global_sensitivity

sensitivity-unaccelerated: build
	@echo "📉 Evaluando modelo sin aceleración tumoral..."
	@mkdir -p results/20260928_draft_map/sensitivity_unaccelerated
	python3 experiments/exp01_cs_exploratory_acceleration/run_exploratory.py \
		--generate-trajectories \
		--output-dir results/20260928_draft_map/sensitivity_unaccelerated
