# Packaging

Every Linux package is built from the same CMake install rules
(`cmake --install`), so binaries, the desktop entry, AppStream metadata and
icons land in the same places everywhere.

| Directory / file | Format | Build command (from the repo root) |
|---|---|---|
| `rpm/zephyr-terminal.spec` | Fedora / RHEL RPM | `packaging/rpm/build_rpm.sh` |
| `deb/debian/` | Debian 12+ / Ubuntu 24.04+ `.deb` | `packaging/deb/build_deb.sh` |
| `arch/PKGBUILD` | Arch Linux (and AUR) | `packaging/arch/build_pkg.sh` |
| `appimage/build_appimage.sh` | Portable AppImage | `packaging/appimage/build_appimage.sh` |
| `../flake.nix`, `../nix/` | Nix flake, NixOS module | `nix build` |
| `desktop/` | `.desktop` entry + AppStream metainfo | installed by CMake |
| `install.sh` | Source installer (CLI + shell) | see `docs/getting-started/installation.md` |

`scripts/build_all_releases.sh` runs whichever of these the current host
supports and writes results to `dist/`. CI builds all of them in matching
distro containers (`.github/workflows/packages.yml`).

Not maintained as part of this release (left as they were, unverified):
`flatpak/`, `snap/`, `macos/`, `windows/`.

Full instructions: [`docs/packaging.md`](../docs/packaging.md).
