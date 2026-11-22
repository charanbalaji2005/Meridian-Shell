#!/usr/bin/env bash
# Build every package format this host can produce into dist/.
#
#   scripts/build_all_releases.sh            # auto-detect
#   scripts/build_all_releases.sh deb appimage
#
# Each format needs its own distro toolchain (see docs/packaging.md); CI
# builds them in matching containers (.github/workflows/packages.yml).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
mkdir -p dist

have() { command -v "$1" >/dev/null 2>&1; }
formats=("$@")
if [[ ${#formats[@]} -eq 0 ]]; then
    have rpmbuild && formats+=(rpm)
    have dpkg-buildpackage && formats+=(deb)
    have makepkg && formats+=(arch)
    have nix && formats+=(nix)
    (have qmake6 || have qtpaths6) && formats+=(appimage)
fi
[[ ${#formats[@]} -gt 0 ]] || { echo "No packaging toolchain found on this host." >&2; exit 1; }

failed=()
for f in "${formats[@]}"; do
    echo "=== Building $f ==="
    case "$f" in
        rpm)      bash packaging/rpm/build_rpm.sh || failed+=(rpm) ;;
        deb)      bash packaging/deb/build_deb.sh || failed+=(deb) ;;
        arch)     bash packaging/arch/build_pkg.sh || failed+=(arch) ;;
        nix)      nix build .#default --print-build-logs -o result && echo "Nix: $(readlink -f result)" || failed+=(nix) ;;
        appimage) bash packaging/appimage/build_appimage.sh || failed+=(appimage) ;;
        *) echo "Unknown format: $f" >&2; failed+=("$f") ;;
    esac
done

echo
ls -lh dist/ 2>/dev/null || true
if [[ ${#failed[@]} -gt 0 ]]; then
    echo "FAILED: ${failed[*]}" >&2
    exit 1
fi
echo "All requested formats built: ${formats[*]}"
