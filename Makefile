buildproj:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/slim/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cd build && make && cd -

build_test:
	echo "building" && cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/slim/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DBUILD_TEST=1 && cd build && make && cd -
run:
	./build/main
