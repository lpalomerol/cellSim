rebuild:
	cmake -S . -B build -DUSE_CXXOPTS=OFF -DBUILD_TESTS=OFF
	cmake --build build