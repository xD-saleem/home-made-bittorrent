b:
	echo "building" && \
	cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/slim/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && \
	cmake --build build -j$(nproc) && \
	echo "successfully built"

test:
		make b && ./build/tests

buildrun:
		make b && ./build/main

ci-build:
	cmake -G Ninja -B build -S . \
		-DCMAKE_TOOLCHAIN_FILE=${{ github.workspace }}/vcpkg/scripts/buildsystems/vcpkg.cmake \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build build --parallel $(nproc)

ci-tests:
	echo "running tests" &&  ./build/tests


