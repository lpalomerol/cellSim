# ============================================================
# Makefile: BRCA1-ONSET / CellSim Simulation Pipeline
# ============================================================

.PHONY: build rebuild test clean calibration-paper calibration-pilot calibration-dry-run sensitivity-global-lhs sensitivity-unaccelerated

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
