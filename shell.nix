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
    perf

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
    glfw
    glm
    glew

    # ---- Nix dev tooling ----
    nixpkgs-fmt

    # ---- Misc utilities ----
    git
    curl
    cacert
    jq
    python3
    python313Packages.glad
  ];

  shellHook = ''
    export CC=gcc
    export CXX=g++
    export LDFLAGS="-fuse-ld=mold"
    export CPM_SOURCE_CACHE="$PWD/.cpm-cache"
    export CLANGD_FLAGS="--compile-commands-dir=$PWD"

    export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [
      pkgs.stdenv.cc.cc.lib
      pkgs.mesa
      pkgs.vulkan-loader
      pkgs.gcc14.cc.lib
      pkgs.wayland
      pkgs.libffi
      pkgs.libffi.dev
      pkgs.wayland.dev
      pkgs.wayland-protocols
      pkgs.xorg.libxcb
      pkgs.xorg.libxcb.dev
      pkgs.libGL
      pkgs.libGLU
      pkgs.xorg.libX11
      pkgs.xorg.libXrandr
      pkgs.xorg.libXinerama
      pkgs.xorg.libXi
      pkgs.xorg.libXcursor
      pkgs.zlib
      pkgs.glfw
    ]}:$LD_LIBRARY_PATH
  '';
}
