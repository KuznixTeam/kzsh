# Kuznix Shell (ksh)

A bash-like shell written in C and C++.

## Features
- Command parsing and execution
- Extensible architecture
- Multiple build system support: Makefile, Meson, CMake
- Debian packaging

## Build Instructions

### Make
```sh
./configure
make
```

### Meson
```sh
meson setup build
ninja -C build
```

### CMake
```sh
cmake -S . -B build
cmake --build build
```

## Packaging
See the `debian/` folder for packaging scripts.
