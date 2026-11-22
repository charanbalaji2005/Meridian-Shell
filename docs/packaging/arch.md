# Arch Linux (PKGBUILD / AUR)

Recipe: `packaging/arch/PKGBUILD`.

Build from your checkout (uses a tarball of the working tree):

```bash
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-svg desktop-file-utils appstream
bash packaging/arch/build_pkg.sh         # -> dist/zephyr-terminal-2.6.0-1-<arch>.pkg.tar.zst
sudo pacman -U dist/zephyr-terminal-2.6.0-1-*.pkg.tar.zst
```

Build the released version (downloads the `v2.6.0` tag):

```bash
cd packaging/arch && makepkg -si
```

`check()` runs `desktop-file-validate`, `appstreamcli validate` and CTest.
Optional dependencies: `qt6-wayland` (native Wayland), `openssh` (SSH profiles).

## AUR

Before publishing, replace `sha256sums=('SKIP')` with the release checksum
(`updpkgsums`), then `makepkg --printsrcinfo > .SRCINFO` and push `PKGBUILD`
and `.SRCINFO` to `ssh://aur@aur.archlinux.org/zephyr-terminal.git`.
