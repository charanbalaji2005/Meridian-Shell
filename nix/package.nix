# Nix derivation for Zephyr Terminal.
#
# Builds the Qt 6 GUI (zephyr-terminal), the CLI (zephyr) and the shell
# (zephyr-shell) with CMake, and installs the desktop entry, AppStream
# metadata and icons. Used by flake.nix and default.nix.
{
  lib,
  stdenv,
  cmake,
  ninja,
  qt6,
  doCheck ? false,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "zephyr-terminal";
  version = "2.6.0";

  # Only what the build needs, so unrelated edits (docs, CI) don't trigger rebuilds.
  src = lib.fileset.toSource {
    root = ../.;
    fileset = lib.fileset.unions [
      ../CMakeLists.txt
      ../src
      ../tests
      ../resources
      ../packaging/desktop
      ../LICENSE
      ../README.md
      ../CHANGELOG.md
    ];
  };

  nativeBuildInputs = [
    cmake
    ninja
    qt6.wrapQtAppsHook
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtsvg
  ] ++ lib.optionals stdenv.hostPlatform.isLinux [ qt6.qtwayland ];

  cmakeFlags = [
    (lib.cmakeFeature "ZEPHYR_BUILD_GUI" "ON")
    (lib.cmakeBool "ZEPHYR_BUILD_TESTS" finalAttrs.doCheck)
  ];

  # The test suite spawns real PTYs and shells and writes under /tmp, which
  # the Nix build sandbox does not provide on every system. It runs in CI
  # outside the sandbox; enable it here with `.override { doCheck = true; }`.
  inherit doCheck;
  preCheck = ''
    export HOME=$TMPDIR
  '';

  meta = {
    description = "Fast, modern terminal emulator with tabs, splits, profiles and a built-in shell";
    homepage = "https://github.com/charanbalaji2005/Meridian-Shell";
    license = lib.licenses.gpl3Plus;
    mainProgram = "zephyr-terminal";
    platforms = lib.platforms.linux;
  };
})
