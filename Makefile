rebuild:
	cmake -S . -B build -DUSE_CXXOPTS=OFF -DBUILD_TESTS=ON
	cmake --build build

.PHONY: build test all all-test cellSim run_all_scenarios single_cell_evolution interactive unit_tests clean

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

# Compilar solo tests (sin ejecutar)
unit_tests: build
	@echo "✅ unit_tests compilado"
	@./build/tests/unit_tests

# Limpiar build directory
clean:
	@echo "🧹 Limpiando build directory..."
	@rm -rf build
	@echo "✅ Limpieza completada"

