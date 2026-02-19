rebuild:
	cmake -S . -B build -DUSE_CXXOPTS=OFF -DBUILD_TESTS=ON
	cmake --build build

.PHONY: build test all all-test cellSim run_all_scenarios single_cell_evolution interactive unit_tests clean install uninstall dist-package dist-deploy dist-run dist-run-seed clean-dist

# Alias para rebuild
build: rebuild

# Ejecutar solo tests
test:
	cmake --build build --target run_tests 2>/dev/null || cmake --build build --target unit_tests 2>/dev/null || (cd build && ctest --output-on-failure)

# Target para compilar y ejecutar tests (el que CLion debería usar)
all: rebuild test

# Alias alternativo
all-test: all

# ============================================================
# Targets para compilar aplicativos individuales
# ============================================================

# Compilar cellSim (ejecutable principal)
cellSim: build
	@echo "✅ cellSim compilado"
	@./build/cellSim

# Compilar y ejecutar run_all_scenarios (todos los escenarios)
run_all_scenarios: build
	@echo "✅ run_all_scenarios compilado"
	@./build/run_all_scenarios

# Compilar y ejecutar single_cell_evolution (célula única)
single_cell_evolution: build
	@echo "✅ single_cell_evolution compilado"
	@./build/single_cell_evolution

# Compilar y ejecutar interactive (modo interactivo)
interactive: build
	@echo "✅ interactive compilado"
	@./build/interactive

# Compilar y ejecutar cellSim_cli con configuración por defecto
cellSim_cli: build
	@echo "✅ cellSim_cli compilado"
	@./build/cellSim_cli --config configs/default.json --verbose

cellSim_cli_silent: build
	@echo "✅ cellSim_cli compilado"
	@./build/cellSim_cli --config configs/default.json

# ============================================================
# Targets para simulaciones múltiples con variabilidad
# ============================================================

# Ejecutar escenario default una vez
default: build
	@./cmake-build-debug/cellSim_cli --config configs/default.json

# Ejecutar N simulaciones con diferentes seeds (default N=10)
N ?= 50
multi: build
	@echo "🧬 Ejecutando $(N) simulaciones con diferentes seeds..."
	@mkdir -p traces/multi
	@for seed in $$(seq 1 $(N)); do \
		echo "  → Seed $$seed..."; \
		./cmake-build-debug/cellSim_cli --config configs/default.json --seed $$seed --output traces/multi/seed_$$seed 2>/dev/null; \
	done
	@echo ""
	@echo "📊 Resumen de años donde se excede el 10% de neoplásticas:"
	@echo "seed,tumor_threshold_year"
	@for seed in $$(seq 1 $(N)); do \
		year=$$(grep "tumor_threshold_year" traces/multi/seed_$$seed/default_run1_SUMMARY.csv 2>/dev/null | cut -d',' -f2); \
		echo "$$seed,$$year"; \
	done
	@echo ""
	@echo "📈 Estadísticas:"
	@years=""; for seed in $$(seq 1 $(N)); do \
		year=$$(grep "tumor_threshold_year" traces/multi/seed_$$seed/default_run1_SUMMARY.csv 2>/dev/null | cut -d',' -f2); \
		if [ "$$year" != "-1" ] && [ -n "$$year" ]; then years="$$years $$year"; fi; \
	done; \
	if [ -n "$$years" ]; then \
		echo "$$years" | tr ' ' '\n' | grep -v '^$$' | awk '{ sum += $$1; sumsq += $$1*$$1; n++ } END { if(n>0) { mean=sum/n; sd=sqrt(sumsq/n - mean*mean); printf "  Promedio: %.1f años\n  SD: %.1f años\n  N: %d\n", mean, sd, n } }'; \
	else \
		echo "  No hay datos válidos"; \
	fi

# Ejecutar simulaciones rápidas (5 seeds)
multi-quick: build
	@$(MAKE) multi N=5

# Ejecutar simulaciones extensas (20 seeds)
multi-full: build
	@$(MAKE) multi N=20

# Limpiar resultados de simulaciones múltiples
clean-multi:
	@rm -rf traces/multi
	@echo "🧹 Limpiados resultados de simulaciones múltiples"



# Compilar solo tests (sin ejecutar)
unit_tests: build
	@echo "✅ unit_tests compilado"
	@./build/tests/unit_tests

# Limpiar build directory
clean:
	@echo "🧹 Limpiando build directory..."
	@rm -rf build
	@echo "✅ Limpieza completada"

# ============================================================
# Targets para instalación y desinstalación
# ============================================================

# Directorio de instalación (personalizable con PREFIX=/ruta)
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Instalar binarios en el sistema
install: rebuild
	@echo "📦 Instalando binarios en $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@install -m 755 build/cellSim $(BINDIR)/cellSim
	@install -m 755 build/cellSim_cli $(BINDIR)/cellSim_cli
	@install -m 755 build/run_all_scenarios $(BINDIR)/run_all_scenarios
	@install -m 755 build/single_cell_evolution $(BINDIR)/single_cell_evolution
	@install -m 755 build/interactive $(BINDIR)/interactive
	@echo "✅ Instalación completada en $(BINDIR)"
	@echo "   Ahora puedes ejecutar: cellSim, cellSim_cli, etc. desde cualquier directorio"

# Desinstalar binarios del sistema
uninstall:
	@echo "🗑️  Desinstalando binarios de $(BINDIR)..."
	@rm -f $(BINDIR)/cellSim
	@rm -f $(BINDIR)/cellSim_cli
	@rm -f $(BINDIR)/run_all_scenarios
	@rm -f $(BINDIR)/single_cell_evolution
	@rm -f $(BINDIR)/interactive
	@echo "✅ Desinstalación completada"

# ============================================================
# Targets para distribución en servidores sin internet
# ============================================================

DIST_DIR ?= cellSim-dist
DIST_VERSION ?= 0.1.0
REMOTE_USER ?= user
REMOTE_HOST ?= server.com
REMOTE_PATH ?= /opt/cellSim

# Crear paquete distributable con todos los binarios y configs
dist-package: rebuild
	@echo "📦 Creando paquete distributable..."
	@mkdir -p $(DIST_DIR)/bin
	@mkdir -p $(DIST_DIR)/configs
	@mkdir -p $(DIST_DIR)/lib
	@cp build/cellSim $(DIST_DIR)/bin/
	@cp build/cellSim_cli $(DIST_DIR)/bin/
	@cp build/run_all_scenarios $(DIST_DIR)/bin/
	@cp build/single_cell_evolution $(DIST_DIR)/bin/
	@cp build/interactive $(DIST_DIR)/bin/
	@chmod +x $(DIST_DIR)/bin/*
	@cp -r configs/* $(DIST_DIR)/configs/
	@cp README.md $(DIST_DIR)/ 2>/dev/null || true
	@tar -czf cellSim-$(DIST_VERSION).tar.gz $(DIST_DIR)/
	@echo "✅ Paquete creado: cellSim-$(DIST_VERSION).tar.gz"
	@rm -rf $(DIST_DIR)

# Copiar paquete al servidor (requiere ruta válida)
dist-deploy: dist-package
	@echo "📤 Desplegando en servidor remoto..."
	@scp cellSim-$(DIST_VERSION).tar.gz $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_PATH)/
	@ssh $(REMOTE_USER)@$(REMOTE_HOST) "cd $(REMOTE_PATH) && tar -xzf cellSim-$(DIST_VERSION).tar.gz && mv cellSim-dist/* . && rmdir cellSim-dist && chmod +x bin/*"
	@echo "✅ Deployment completado en $(REMOTE_USER)@$(REMOTE_HOST):$(REMOTE_PATH)"

# Ejecutar cellSim_cli en el servidor remoto
dist-run:
	@echo "🚀 Ejecutando cellSim_cli en servidor remoto..."
	@ssh $(REMOTE_USER)@$(REMOTE_HOST) "$(REMOTE_PATH)/bin/cellSim_cli --config $(REMOTE_PATH)/configs/default.json --verbose"

# Ejecutar cellSim_cli en el servidor remoto con seed personalizado
dist-run-seed:
	@ssh $(REMOTE_USER)@$(REMOTE_HOST) "$(REMOTE_PATH)/bin/cellSim_cli --config $(REMOTE_PATH)/configs/default.json --seed $(SEED) --verbose"

# Limpiar archivos de distribución locales
clean-dist:
	@rm -rf $(DIST_DIR)
	@rm -f cellSim-*.tar.gz
	@echo "🧹 Limpiados archivos de distribución"

