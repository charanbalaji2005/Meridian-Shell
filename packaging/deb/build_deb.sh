#!/usr/bin/env bash
# Build the Debian/Ubuntu .deb from the current source tree.
#
# Requires Debian 12+ or Ubuntu 24.04+ (Qt >= 6.4):
#   sudo apt install build-essential devscripts equivs
#   sudo mk-build-deps -i -r -t 'apt-get -y' packaging/deb/debian/control
#   packaging/deb/build_deb.sh            # -> dist/*.deb
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VERSION="$(dpkg-parsechangelog -l "$ROOT/packaging/deb/debian/changelog" -S Version)"
UPSTREAM="${VERSION%-*}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Build in a pristine copy so the repo never gets a stray debian/ dir.
bash "$ROOT/scripts/make_source_tarball.sh" "$WORK" "zephyr-terminal-$UPSTREAM" "zephyr-terminal_$UPSTREAM.orig"
tar -C "$WORK" -xzf "$WORK/zephyr-terminal_$UPSTREAM.orig.tar.gz"
cp -r "$ROOT/packaging/deb/debian" "$WORK/zephyr-terminal-$UPSTREAM/debian"
chmod +x "$WORK/zephyr-terminal-$UPSTREAM/debian/rules"

(cd "$WORK/zephyr-terminal-$UPSTREAM" && dpkg-buildpackage -us -uc -b)

mkdir -p "$ROOT/dist"
cp -v "$WORK"/*.deb "$ROOT/dist/"
if command -v lintian >/dev/null; then
    lintian --info "$ROOT"/dist/zephyr-terminal_"$VERSION"_*.deb || true
fi
