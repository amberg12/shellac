windows:
	cmake -B build-windows \
      -DCMAKE_TOOLCHAIN_FILE=./scripts/mingw-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release

.PHONY: windows