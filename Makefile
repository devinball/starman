#SHELL := /bin/sh
#export PATH := $(shell nix-shell shell.nix)

build:
	cmake -B build # >> build.log 2>&1
	cmake --build build # >> build.log 2>&1

run:
	cmake -B build # >> build.log 2>&1
	cmake --build build # >> build.log 2>&1
	nvidia-offload ./build/example

run-with-mango:
	cmake -B build # >> build.log 2>&1
	cmake --build build # >> build.log 2>&1
	nvidia-offload mangohud ./build/example

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	gdb ./build/example

clean:
	rm -rf build/
