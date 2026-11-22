#!/usr/bin/env bash
# Build a portable Zephyr-Terminal-<version>-<arch>.AppImage.
#
# Bundles the Qt 6 runtime (LGPL, dynamically linked) with linuxdeploy and
# linuxdeploy-plugin-qt. Build on the OLDEST distro you want to support:
# the AppImage runs on systems with the same or newer glibc. Needs Qt >= 6.4,
# e.g. Ubuntu 24.04 / Debian 12:
#   sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-svg-dev \
#                    libgl-dev qt6-wayland file wget desktop-file-utils
#   packaging/appimage/build_appimage.sh        # -> dist/*.AppImage
#
# linuxdeploy only publishes rolling "continuous" builds. For reproducible
# builds, download them once, verify, and point at your copies:
#   LINUXDEPLOY=/path/linuxdeploy-x86_64.AppImage \
#   LINUXDEPLOY_QT=/path/linuxdeploy-plugin-qt-x86_64.AppImage packaging/appimage/build_appimage.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ARCH="${ARCH:-$(uname -m)}"
VERSION="$(sed -n 's/^ *VERSION \([0-9.]*\)$/\1/p' "$ROOT/CMakeLists.txt" | head -n1)"
WORK="${WORK:-$ROOT/build/appimage}"
APPDIR="$WORK/AppDir"
TOOLS="$WORK/tools"
mkdir -p "$TOOLS" "$ROOT/dist"
rm -rf "$APPDIR" "$WORK/build"

fetch() { # name url
    local dest="$TOOLS/$1"
    if [[ ! -x "$dest" ]]; then
        echo "Downloading $1"
        wget -q -O "$dest" "$2"
        chmod +x "$dest"
    fi
    echo "$dest"
}
LINUXDEPLOY="${LINUXDEPLOY:-$(fetch linuxdeploy-$ARCH.AppImage \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage)}"
LINUXDEPLOY_QT="${LINUXDEPLOY_QT:-$(fetch linuxdeploy-plugin-qt-$ARCH.AppImage \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$ARCH.AppImage)}"
# linuxdeploy discovers plugins on PATH as linuxdeploy-plugin-<name>.
ln -sf "$LINUXDEPLOY_QT" "$TOOLS/linuxdeploy-plugin-qt"
export PATH="$TOOLS:$PATH"

cmake -S "$ROOT" -B "$WORK/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DZEPHYR_BUILD_GUI=ON \
    -DZEPHYR_BUILD_TESTS=OFF
cmake --build "$WORK/build"
DESTDIR="$APPDIR" cmake --install "$WORK/build"

DESKTOP="$APPDIR/usr/share/applications/org.zephyr_terminal.ZephyrTerminal.desktop"
ICON="$APPDIR/usr/share/icons/hicolor/256x256/apps/zephyr-terminal.png"
desktop-file-validate "$DESKTOP"

# Bundle the Wayland platform plugin too when it is installed, so the
# AppImage runs natively on Wayland (Qt falls back to XCB otherwise).
QT_PLUGIN_DIR="$(qtpaths6 --plugin-dir 2>/dev/null || qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
if [[ -n "$QT_PLUGIN_DIR" && -e "$QT_PLUGIN_DIR/platforms/libqwayland-generic.so" ]]; then
    export EXTRA_PLATFORM_PLUGINS="libqwayland-generic.so;libqwayland-egl.so"
fi
export QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake)}"
export OUTPUT="Zephyr-Terminal-$VERSION-$ARCH.AppImage"
# The tools are AppImages themselves; extract-and-run works without FUSE (CI, containers).
export APPIMAGE_EXTRACT_AND_RUN=1

(cd "$WORK" && "$LINUXDEPLOY" --appdir "$APPDIR" \
    --desktop-file "$DESKTOP" --icon-file "$ICON" \
    --executable "$APPDIR/usr/bin/zephyr-terminal" \
    --executable "$APPDIR/usr/bin/zephyr" \
    --executable "$APPDIR/usr/bin/zephyr-shell" \
    --plugin qt --output appimage)

mv -f "$WORK/$OUTPUT" "$ROOT/dist/"
echo "Built dist/$OUTPUT"
# Smoke test: the bundled binary starts and reports its version (no display needed).
QT_QPA_PLATFORM=offscreen "$ROOT/dist/$OUTPUT" --version
