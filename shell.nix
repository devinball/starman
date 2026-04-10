{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "engine-dev";

  packages = with pkgs; [
    # ---- Build system ----
    cmake
    ninja
    pkg-config

    # ---- Compilers ----
    gcc14
    clang_18
    llvmPackages_18.libcxxClang

    # ---- Linkers ----
    mold
    lld_18

    # ---- Static analysis / formatting ----
    llvmPackages_18.clang-tools
    cppcheck

    # ---- Debuggers / profilers ----
    gdb
    valgrind
    linuxPackages.perf

    # ---- System libraries SDL3 needs on Linux ----
    xorg.libX11
    xorg.libXext
    xorg.libXrandr
    xorg.libXi
    xorg.libXcursor
    xorg.libXinerama
    xorg.libXfixes
    xorg.libXScrnSaver
    xorg.libxcb
    xorg.xcbutil
    xorg.xcbutilwm
    xorg.xcbutilimage
    xorg.xcbutilkeysyms
    xorg.xcbutilrenderutil
    libGL
    libGLU
    mesa
    vulkan-headers
    vulkan-loader
    wayland
    wayland-protocols
    libxkbcommon
    libffi          # required by wayland-client
    dbus
    udev
    alsa-lib
    pipewire

    # ---- Nix dev tooling ----
    nixpkgs-fmt

    # ---- Misc utilities ----
    git
    curl
    cacert
    jq
    python3
  ];

  shellHook = ''
    export CC=gcc
    export CXX=g++

    export LDFLAGS="-fuse-ld=mold"
    export CPM_SOURCE_CACHE="$PWD/.cpm-cache"


    export LD_LIBRARY_PATH="${pkgs.libGL}/lib:${pkgs.mesa}/lib:${pkgs.vulkan-loader}/lib:${pkgs.gcc14.cc.lib}/lib:${pkgs.wayland}/lib:${pkgs.libffi}/lib:${pkgs.xorg.libX11}/lib:${pkgs.xorg.libXext}/lib:${pkgs.xorg.libXrandr}/lib:${pkgs.xorg.libXi}/lib:${pkgs.xorg.libXcursor}/lib:${pkgs.xorg.libxcb}/lib:${pkgs.libxkbcommon}/lib:$LD_LIBRARY_PATH"

    # pkg-config paths — libffi is needed by wayland-client
    export PKG_CONFIG_PATH="${pkgs.libffi.dev}/lib/pkgconfig:${pkgs.wayland.dev}/lib/pkgconfig:${pkgs.wayland-protocols}/share/pkgconfig:${pkgs.xorg.libxcb.dev}/lib/pkgconfig:${pkgs.vulkan-headers}/lib/pkgconfig:$PKG_CONFIG_PATH"

    export CLANGD_FLAGS="--compile-commands-dir=$PWD"

    echo ""
    echo "  engine dev shell"
    echo "  ─────────────────────────────────────────────"
    echo "  CC=$CC  CXX=$CXX"
    echo "  cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug"
    echo "  cmake --build build -j\$(nproc)"
    echo ""
    echo "  To use clang:  export CC=clang CXX=clang++"
    echo "  Sanitizers:    cmake -G Ninja -B build-asan -DENGINE_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug"
    echo ""
  '';
}
