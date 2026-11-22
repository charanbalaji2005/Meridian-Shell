#!/usr/bin/env bash
# Build the Arch Linux package from the current source tree (not the tag).
#
#   sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-svg desktop-file-utils appstream
#   packaging/arch/build_pkg.sh           # -> dist/*.pkg.tar.zst
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PKGVER="$(sed -n 's/^pkgver=//p' "$ROOT/packaging/arch/PKGBUILD")"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

cp "$ROOT/packaging/arch/PKGBUILD" "$WORK/"
# makepkg uses a source file that is already present instead of downloading it.
bash "$ROOT/scripts/make_source_tarball.sh" "$WORK" "zephyr-terminal-$PKGVER"

(cd "$WORK" && makepkg --cleanbuild --force --noconfirm "$@")

mkdir -p "$ROOT/dist"
cp -v "$WORK"/*.pkg.tar.* "$ROOT/dist/"
