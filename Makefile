run:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	nvidia-offload mangohud ./build/example

build:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	
