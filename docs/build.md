---
layout: default
title: "Installation & Build"
category: "Getting Started"
---

# Build

## CMake (full build, used by every package)

Requirements: C++20 compiler (GCC 11+/Clang 14+), CMake ≥ 3.20, Ninja
(optional), Qt ≥ 6.4 with Widgets and Svg for the GUI.

| Distro | Build dependencies |
|---|---|
| Debian 12+ / Ubuntu 24.04+ | `build-essential cmake ninja-build qt6-base-dev qt6-svg-dev libgl-dev` |
| Fedora | `gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtsvg-devel` |
| Arch | `base-devel cmake ninja qt6-base qt6-svg` |
| Nix | `nix develop` |

```bash
cmake -S . -B build -G Ninja -DZEPHYR_BUILD_GUI=ON
cmake --build build
ctest --test-dir build --output-on-failure   # unit tests + PTY integration + manual_core_test.sh
./build/zephyr-terminal
sudo cmake --install build                    # /usr/local by default; DESTDIR= for staging
```

Options: `ZEPHYR_BUILD_GUI` (`AUTO`/`ON`/`OFF`), `ZEPHYR_BUILD_TESTS`,
`ZEPHYR_ENABLE_ASAN`, `ZEPHYR_WARNINGS_AS_ERRORS`, `ZEPHYR_INSTALL_DOCS`,
`ZEPHYR_INSTALL_LICENSE`.

Sanitizers: `cmake -B build-asan -DZEPHYR_ENABLE_ASAN=ON -DZEPHYR_BUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Debug`.

## Makefile (CLI, shell and tests — no Qt needed)

```bash
make all          # zephyr, zephyr-shell, zephyr_demo, zephyr_tests
make test         # run the test suite
make manual-test  # run tests/manual_core_test.sh against the built shell
make gui          # delegates to CMake to build zephyr-terminal
```

Use `BUILD=<dir>` to build elsewhere; tests find the shell via
`ZEPHYR_SHELL_BIN`, which the `test`/`manual-test` targets set for you.

## Packages

See [packaging.md](packaging.md).
