# Debian / Ubuntu (.deb)

Recipe: `packaging/deb/debian/` (debhelper 13, `--buildsystem=cmake+ninja`).
Requires Qt ≥ 6.4: Debian 12 (bookworm) or newer, Ubuntu 24.04 or newer.

```bash
sudo apt install build-essential devscripts equivs
sudo mk-build-deps -i -r -t 'apt-get -y' packaging/deb/debian/control
bash packaging/deb/build_deb.sh          # -> dist/zephyr-terminal_2.6.0-1_<arch>.deb
sudo apt install ./dist/zephyr-terminal_2.6.0-1_*.deb
```

`build_deb.sh` builds in a temporary copy of the tree, so no `debian/`
directory is left in the repository. The test step validates the desktop
entry and AppStream metadata and runs CTest (skip with
`DEB_BUILD_OPTIONS=nocheck`). Hardening flags are enabled
(`hardening=+all`). If `lintian` is installed its report is printed.

The package does not register the `x-terminal-emulator` alternative: Debian
policy requires such terminals to support `-T <title>`, which Zephyr does
not implement yet. Launchers using `xdg-terminal-exec` find Zephyr through
its desktop entry.
