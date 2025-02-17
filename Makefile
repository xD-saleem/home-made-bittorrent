buildproj:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/slim/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cd build && make && cd - && echo "successfully built"

buildMSproj:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G "Ninja" && cmake --build build && echo "successfully built"

run:
	./build/main

test:
		make buildproj && ./build/tests

buildrun:
		make buildproj && ./build/main

ci-build:
	 cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${{ github.workspace }}/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cd build && make && cd - && echo "successfully built"

ci-tests:
	echo "running tests" &&  ./build/tests


