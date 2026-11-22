# Packaging Zephyr Terminal

All Linux packages are built from one set of CMake install rules, so every
format installs the same files:

| Path (under the prefix, usually `/usr`) | Content |
|---|---|
| `bin/zephyr-terminal` | Qt 6 GUI |
| `bin/zephyr`, `bin/zephyr-shell` | CLI toolkit and interactive shell |
| `share/applications/org.zephyr_terminal.ZephyrTerminal.desktop` | Desktop entry (`Terminal=false`, `X-TerminalArgExec=-e`) |
| `share/metainfo/org.zephyr_terminal.ZephyrTerminal.metainfo.xml` | AppStream metadata (software centers) |
| `share/icons/hicolor/{16..512}/apps/zephyr-terminal.png`, `scalable/apps/zephyr-terminal.svg` | Application icon |
| `share/zephyr/images/gallery/` | Artwork used by `zephyr pic` |
| `share/doc/zephyr-terminal/`, `share/licenses/zephyr-terminal/` | Docs / license (disable with `-DZEPHYR_INSTALL_DOCS=OFF`, `-DZEPHYR_INSTALL_LICENSE=OFF`) |

Build requirements everywhere: C++20 compiler, CMake ≥ 3.20, Qt ≥ 6.4
(Widgets, Svg). Runtime: Qt 6 base + svg; `qt6-wayland` recommended.

The desktop file id, the Wayland `app_id` (`QGuiApplication::desktopFileName`)
and the X11 `WM_CLASS` (`zephyr-terminal`) all match, so docks and task
switchers show the right icon.

## Formats

| Format | Recipe | Local build | Guide |
|---|---|---|---|
| Nix / NixOS | `flake.nix`, `nix/package.nix`, `nix/module.nix`, `default.nix` | `nix build` | [nixos.md](packaging/nixos.md) |
| Fedora RPM | `packaging/rpm/zephyr-terminal.spec` | `bash packaging/rpm/build_rpm.sh` | [fedora.md](packaging/fedora.md) |
| Debian / Ubuntu | `packaging/deb/debian/` | `bash packaging/deb/build_deb.sh` | [debian.md](packaging/debian.md) |
| Arch Linux | `packaging/arch/PKGBUILD` | `bash packaging/arch/build_pkg.sh` | [arch.md](packaging/arch.md) |
| AppImage | `packaging/appimage/build_appimage.sh` | same | [appimage.md](packaging/appimage.md) |

Local builds use `scripts/make_source_tarball.sh`, which packs the working
tree (including uncommitted changes) so you can test packaging before
committing. `scripts/build_all_releases.sh` runs every builder available on
the host.

## How packages are validated

`.github/workflows/packages.yml` (run on every push/PR via `ci.yml`, and on
tags via `release.yml`) builds each format in a matching environment and then
**installs the result and runs it**:

| Job | Environment | Checks |
|---|---|---|
| rpm | `fedora:latest` | `%check`: `desktop-file-validate`, `appstream-util validate-relax`, CTest; install with dnf; run `zephyr-terminal --version` and `zephyr-terminal -e sh -c 'echo ok'` (offscreen) |
| deb | `debian:bookworm`, `ubuntu:24.04` | `desktop-file-validate`, `appstreamcli validate`, CTest via `dh_auto_test`; install with apt; same run checks; lintian (informational) |
| arch | `archlinux:latest` | `check()`: validators + CTest; install with pacman; same run checks |
| nix | Ubuntu + Nix | `nix flake check`, `nix build`, run checks, `nix profile install`, and a build with `doCheck = true` |
| appimage | Ubuntu 24.04 | linuxdeploy bundle, `--version` smoke test, end-to-end `-e` run |

The `-e sh -c 'echo ok'` run is an end-to-end test: the GUI starts, spawns
the command on a real PTY, the command exits, the pane, tab and window close,
and the process must exit on its own within the timeout.

## Releasing

1. Bump the version in **all** of: `CMakeLists.txt` (`project(VERSION)`),
   `packaging/rpm/zephyr-terminal.spec` (`Version`, `%changelog`),
   `packaging/deb/debian/changelog`, `packaging/arch/PKGBUILD` (`pkgver`),
   `nix/package.nix` (`version`), `packaging/desktop/*.metainfo.xml` (`<release>`),
   and `CHANGELOG.md`.
2. Commit `flake.lock` (run `nix flake lock` once; CI uploads a generated one as an artifact).
3. Tag `vX.Y.Z` and push. `release.yml` builds and tests every package and
   publishes them with a source tarball and `SHA256SUMS` to GitHub Releases.
4. AUR: update `sha256sums` in the PKGBUILD from `SHA256SUMS`
   (`updpkgsums`), run `makepkg --printsrcinfo > .SRCINFO`, and push both to
   the AUR repository.

## Not maintained in this release

`packaging/flatpak/`, `packaging/snap/`, `packaging/macos/` and
`packaging/windows/` were left unchanged. They predate the GUI and have not
been validated; the GUI's PTY layer is Linux/POSIX-only.

## Why no MIME types or URL handlers

Terminal emulators don't open file types, so no MIME types are registered.
An `ssh://` URL handler was considered and deliberately left out: passing
untrusted URLs from a browser straight into `ssh` has a history of option- and
command-injection bugs. Saved SSH profiles are validated instead
(see `config::SshProfile::validate`).
