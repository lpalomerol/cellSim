rebuild:
	cmake -S . -B build -DUSE_CXXOPTS=OFF -DBUILD_TESTS=ON
	cmake --build build