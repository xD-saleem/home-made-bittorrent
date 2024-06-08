buildproj:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/slim/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cd build && make && cd - && echo "successfully built"

run:
	./build/main

test:
		make buildproj && ./build/tests

buildrun:
		make buildproj && ./build/main

ci:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cd build && make && cd - && echo "successfully built"
