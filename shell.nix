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
    mangohud

    # ---- System libraries SDL3 needs on Linux ----
    libX11
    libXext
    libXrandr
    libXi
    libXcursor
    libXinerama
    libXfixes
    libXScrnSaver
    libxcb
    xcbutil
    xcbutilwm
    xcbutilimage
    xcbutilkeysyms
    xcbutilrenderutil
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
    renderdoc
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
      pkgs.libxcb
      pkgs.libxcb.dev
      pkgs.libGL
      pkgs.libGLU
      pkgs.libX11
      pkgs.libXrandr
      pkgs.libXinerama
      pkgs.libXi
      pkgs.libXcursor
      pkgs.zlib
      pkgs.glfw
      pkgs.libxcb
      pkgs.libglvnd
    ]}:$LD_LIBRARY_PATH
  '';
}
