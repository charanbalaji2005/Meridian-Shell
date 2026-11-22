#!/usr/bin/env bash
# ==============================================================================
# Zephyr Terminal 2.5 — macOS DMG Packaging Script
# Generates native .app bundle and distributable .dmg disk image
# ==============================================================================
set -euo pipefail

APP_VERSION="2.5.0"
ARCH="$(uname -m)"
OUTPUT_DIR="dist"
APP_NAME="Zephyr.app"
DMG_NAME="Zephyr-Terminal-${APP_VERSION}-macOS-${ARCH}.dmg"

echo "==> Building macOS Application Bundle: ${APP_NAME} (${ARCH})..."

# 1. Compile binaries if not present
if [ ! -f "build/zephyr" ] || [ ! -f "build/zephyr-shell" ]; then
    make all -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
fi

# 2. Assemble .app bundle structure
BUNDLE_DIR="${OUTPUT_DIR}/${APP_NAME}"
CONTENTS_DIR="${BUNDLE_DIR}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"

rm -rf "${BUNDLE_DIR}"
mkdir -p "${MACOS_DIR}" "${RESOURCES_DIR}"

cp -f build/zephyr "${MACOS_DIR}/zephyr"
cp -f build/zephyr-shell "${MACOS_DIR}/zephyr-shell"
chmod +x "${MACOS_DIR}/zephyr"*

cp -f packaging/macos/Info.plist "${CONTENTS_DIR}/Info.plist"

if [ -d "resources/images/gallery" ]; then
    mkdir -p "${RESOURCES_DIR}/gallery"
    cp -rf resources/images/gallery/* "${RESOURCES_DIR}/gallery/" 2>/dev/null || true
fi

if [ -f "resources/icons/zephyr-terminal.icns" ]; then
    cp -f resources/icons/zephyr-terminal.icns "${RESOURCES_DIR}/zephyr-terminal.icns"
elif [ -f "resources/icons/zephyr-terminal.svg" ]; then
    cp -f resources/icons/zephyr-terminal.svg "${RESOURCES_DIR}/zephyr-terminal.svg"
fi

echo "==> Application bundle successfully created at ${BUNDLE_DIR}"

# 3. Build DMG Disk Image (if hdiutil is available on macOS)
if command -v hdiutil >/dev/null 2>&1; then
    echo "==> Creating Apple Disk Image (${DMG_NAME})..."
    DMG_TEMP_DIR="$(mktemp -d /tmp/zephyr-dmg-XXXXXX)"
    cp -R "${BUNDLE_DIR}" "${DMG_TEMP_DIR}/"
    ln -s /Applications "${DMG_TEMP_DIR}/Applications"

    rm -f "${OUTPUT_DIR}/${DMG_NAME}"
    hdiutil create -volname "Zephyr Terminal ${APP_VERSION}" \
                   -srcfolder "${DMG_TEMP_DIR}" \
                   -ov -format UDZO \
                   "${OUTPUT_DIR}/${DMG_NAME}"

    rm -rf "${DMG_TEMP_DIR}"
    echo "==> Successfully generated: ${OUTPUT_DIR}/${DMG_NAME}"
else
    echo "==> Creating macOS ZIP archive fallback (${OUTPUT_DIR}/Zephyr-Terminal-${APP_VERSION}-macOS-${ARCH}.zip)..."
    cd "${OUTPUT_DIR}"
    zip -r -q "Zephyr-Terminal-${APP_VERSION}-macOS-${ARCH}.zip" "${APP_NAME}"
    cd ..
    echo "==> Successfully generated ZIP bundle"
fi

