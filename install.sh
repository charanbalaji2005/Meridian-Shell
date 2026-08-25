#!/usr/bin/env bash
# Meridian Terminal Turnkey Installer (Linux & macOS)
set -e

echo "=============================================="
echo "      Meridian Terminal 2.0 Installer         "
echo "=============================================="

PREFIX="${PREFIX:-/usr/local}"
USER_MODE=false

if [ "$1" = "--user" ] || [ "$EUID" -ne 0 ]; then
    if [ "$1" != "--user" ] && [ "$EUID" -ne 0 ]; then
        echo "Notice: Non-root user detected. Installing to ~/.local/bin."
    fi
    PREFIX="$HOME/.local"
    USER_MODE=true
fi

echo "-> Building Meridian Terminal..."
make all -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"

echo "-> Installing binaries to ${PREFIX}/bin..."
mkdir -p "${PREFIX}/bin"
cp -f build/meridian-shell "${PREFIX}/bin/meridian-shell"
cp -f build/meridian "${PREFIX}/bin/meridian"
chmod +x "${PREFIX}/bin/meridian-shell" "${PREFIX}/bin/meridian"

if [ "$USER_MODE" = false ] && [ -w /etc/shells ]; then
    if ! grep -Fxq "${PREFIX}/bin/meridian-shell" /etc/shells; then
        echo "-> Registering meridian-shell in /etc/shells..."
        echo "${PREFIX}/bin/meridian-shell" >> /etc/shells
    fi
fi

# Desktop Entry for Linux
if [ "$(uname)" = "Linux" ]; then
    DESKTOP_DIR="$HOME/.local/share/applications"
    if [ "$USER_MODE" = false ]; then
        DESKTOP_DIR="/usr/share/applications"
    fi
    mkdir -p "${DESKTOP_DIR}"
    cp -f meridian.desktop "${DESKTOP_DIR}/meridian.desktop"
    echo "-> Registered desktop launcher in ${DESKTOP_DIR}/meridian.desktop"
fi

echo ""
echo "=============================================="
echo "  Meridian Terminal successfully installed!   "
echo "=============================================="
echo "You can now run:"
echo "   meridian-shell     (Launch terminal session)"
echo "   meridian           (Launch developer CLI)"
echo ""
if [ "$USER_MODE" = true ]; then
    echo "Make sure ${PREFIX}/bin is in your PATH in ~/.bashrc or ~/.zshrc:"
    echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
