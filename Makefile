#SHELL := /bin/sh
#export PATH := $(shell nix-shell shell.nix)

run:
	cmake -B build # > build.log
	cmake --build build # >> build.log 2>&1
	nvidia-offload ./build/example

execute:
	nvidia-offload mangohud ./build/example

run-mango:
	cmake -B build # > build.log
	cmake --build build # >> build.log 2>&1
	nvidia-offload mangohud ./build/example

compile:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	gdb ./build/example
	
gfx-test:
	nvidia-offload mangohud , glxgears

doc:
	qrenderdoc &
	renderdoccmd capture -d . build/example

clean:
	rm -r build/