#!/usr/bin/env bash
# Build the Fedora/RHEL RPM from the current source tree.
#
#   sudo dnf install rpm-build rpmdevtools 'dnf-command(builddep)'
#   sudo dnf builddep packaging/rpm/zephyr-terminal.spec
#   packaging/rpm/build_rpm.sh            # -> dist/*.rpm
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SPEC="$ROOT/packaging/rpm/zephyr-terminal.spec"
VERSION="$(rpmspec -q --qf '%{version}\n' "$SPEC" | head -n1)"
TOP="$(mktemp -d)"
trap 'rm -rf "$TOP"' EXIT

mkdir -p "$TOP"/{SOURCES,SPECS,BUILD,RPMS,SRPMS}
bash "$ROOT/scripts/make_source_tarball.sh" "$TOP/SOURCES" "zephyr-terminal-$VERSION"
cp "$SPEC" "$TOP/SPECS/"

rpmbuild --define "_topdir $TOP" -ba "$TOP/SPECS/zephyr-terminal.spec"

mkdir -p "$ROOT/dist"
find "$TOP/RPMS" "$TOP/SRPMS" -name '*.rpm' -exec cp -v {} "$ROOT/dist/" \;
