run:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	nvidia-offload mangohud ./build/example

test:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	env __NV_PRIME_RENDER_OFFLOAD=1 \
		__EGL_VENDOR_LIBRARY_FILENAMES=/run/opengl-driver/share/glvnd/egl_vendor.d/10_nvidia.json \
		mangohud ./build/example

xtest:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	nvidia-offload env WAYLAND_DISPLAY="" DISPLAY=:0 mangohud ./build/example

compile:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug # > build.log
	cmake --build build # >> build.log 2>&1
	gdb ./build/example
	
gfx-test:
	nvidia-offload mangohud , glxgears
