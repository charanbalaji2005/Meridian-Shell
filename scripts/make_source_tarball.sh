#!/usr/bin/env bash
# Create a clean source tarball for packaging.
#
#   scripts/make_source_tarball.sh OUT_DIR TOP_DIR_NAME [TARBALL_NAME]
#
# Example: scripts/make_source_tarball.sh dist zephyr-terminal-2.6.0
#   -> dist/zephyr-terminal-2.6.0.tar.gz containing zephyr-terminal-2.6.0/...
#
# In a git checkout it packs the working tree's tracked + untracked
# (non-ignored) files, so local uncommitted changes are included; outside
# git it copies the tree minus build output.
set -euo pipefail

OUT_DIR="$1"
TOP="$2"
NAME="${3:-$TOP}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$OUT_DIR"
OUT="$(cd "$OUT_DIR" && pwd)/$NAME.tar.gz"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/$TOP"

if git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    (cd "$ROOT" && git ls-files -z --cached --others --exclude-standard |
        while IFS= read -r -d '' f; do [ -e "$f" ] && printf '%s\0' "$f"; done |
        tar --null -T - -cf -) | tar -C "$STAGE/$TOP" -xf -
else
    tar -C "$ROOT" \
        --exclude='./build' --exclude='./build-*' --exclude='./dist' --exclude='./result' \
        --exclude='./.git' --exclude='./debian' --exclude='*.AppImage' \
        -cf - . | tar -C "$STAGE/$TOP" -xf -
fi
tar -C "$STAGE" -czf "$OUT" "$TOP"
echo "Source tarball: $OUT"
