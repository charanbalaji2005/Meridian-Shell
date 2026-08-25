#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal Universal 1-Line Installer for Any Linux System (Fedora, Ubuntu, Arch, etc.)
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash
# ==============================================================================
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

# Detect if running from within cloned repo or via curl pipe
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "-> Downloading Meridian Terminal source from GitHub..."
    WORK_DIR="$(mktemp -d /tmp/meridian-install-XXXXXX)"
    trap 'rm -rf "${WORK_DIR}"' EXIT

    # Ensure curl and tar are present
    if command -v dnf >/dev/null 2>&1; then
        echo "-> Checking dependencies on Fedora..."
        dnf install -y gcc-c++ make git tar gzip libutil-devel >/dev/null 2>&1 || true
    elif command -v apt-get >/dev/null 2>&1; then
        echo "-> Checking dependencies on Ubuntu/Debian..."
        apt-get update -qq && apt-get install -y -qq g++ make git tar gzip >/dev/null 2>&1 || true
    elif command -v pacman >/dev/null 2>&1; then
        echo "-> Checking dependencies on Arch Linux..."
        pacman -Sy --noconfirm base-devel git >/dev/null 2>&1 || true
    fi

    curl -fsSL https://github.com/charanbalaji2005/Meridian-Shell/archive/refs/heads/main.tar.gz | tar -xz -C "${WORK_DIR}"
    cd "${WORK_DIR}/Meridian-Shell-main"
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
    if ! grep -Fxq "${PREFIX}/bin/meridian" /etc/shells; then
        echo "${PREFIX}/bin/meridian" >> /etc/shells
    fi
fi

# Desktop Entry for Linux
if [ "$(uname)" = "Linux" ]; then
    DESKTOP_DIR="$HOME/.local/share/applications"
    if [ "$USER_MODE" = false ]; then
        DESKTOP_DIR="/usr/share/applications"
    fi
    mkdir -p "${DESKTOP_DIR}"
    cp -f meridian.desktop "${DESKTOP_DIR}/meridian.desktop" 2>/dev/null || true
    echo "-> Registered desktop launcher in ${DESKTOP_DIR}/meridian.desktop"
fi

echo ""
echo "=============================================="
echo "  Meridian Terminal successfully installed!   "
echo "=============================================="
echo "You can now run:"
echo "   meridian           (Launch Meridian Terminal)"
echo "   meridian-shell     (Launch standalone shell)"
echo ""
if [ "$USER_MODE" = true ]; then
    echo "Make sure ${PREFIX}/bin is in your PATH in ~/.bashrc or ~/.zshrc:"
    echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
