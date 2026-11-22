#!/usr/bin/env bash
# ==============================================================================
# Zephyr Terminal Universal Uninstaller
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/uninstall.sh | sudo bash
#   or: ./uninstall.sh [--user] [--purge]
# ==============================================================================
set -e

echo "=============================================="
echo "      Zephyr Terminal 2.0 Uninstaller       "
echo "=============================================="

PURGE_CONFIG=false
for arg in "$@"; do
    if [ "$arg" = "--purge" ] || [ "$arg" = "-p" ]; then
        PURGE_CONFIG=true
    fi
done

# 1. Remove binary files
echo "-> Removing Zephyr binaries..."
rm -f /usr/local/bin/zephyr /usr/local/bin/zephyr-shell 2>/dev/null || true
rm -f /usr/bin/zephyr /usr/bin/zephyr-shell 2>/dev/null || true
rm -f "${HOME}/.local/bin/zephyr" "${HOME}/.local/bin/zephyr-shell" 2>/dev/null || true
# The GUI from /usr/local or ~/.local only; /usr/bin belongs to your distro package manager.
rm -f /usr/local/bin/zephyr-terminal "${HOME}/.local/bin/zephyr-terminal" 2>/dev/null || true

# 2. Remove desktop application launcher
echo "-> Removing desktop launchers..."
rm -f /usr/share/applications/zephyr.desktop 2>/dev/null || true
rm -f /usr/local/share/applications/zephyr.desktop 2>/dev/null || true
rm -f "${HOME}/.local/share/applications/zephyr.desktop" 2>/dev/null || true
# Launcher written by the installer or by Settings > Linux Integration.
rm -f /usr/local/share/applications/org.zephyr_terminal.ZephyrTerminal.desktop 2>/dev/null || true
rm -f "${HOME}/.local/share/applications/org.zephyr_terminal.ZephyrTerminal.desktop" 2>/dev/null || true

# 3. Remove shared gallery and data
echo "-> Removing shared application data..."
rm -rf /usr/local/share/zephyr /usr/share/zephyr 2>/dev/null || true

# 4. Remove from /etc/shells if writable
if [ -w /etc/shells ]; then
    echo "-> Cleaning /etc/shells entries..."
    sed -i '\#/usr/local/bin/zephyr#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/local/bin/zephyr-shell#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/bin/zephyr#d' /etc/shells 2>/dev/null || true
    sed -i '\#/usr/bin/zephyr-shell#d' /etc/shells 2>/dev/null || true
fi

# 5. Handle user configurations
if [ "$PURGE_CONFIG" = true ]; then
    echo "-> Purging configuration directory (~/.config/zephyr)..."
    rm -rf "${HOME}/.config/zephyr" 2>/dev/null || true
else
    echo "Notice: Configuration preserved at ~/.config/zephyr (run with --purge to remove)."
fi

echo ""
echo "=============================================="
echo " Zephyr Terminal successfully uninstalled!  "
echo "=============================================="
