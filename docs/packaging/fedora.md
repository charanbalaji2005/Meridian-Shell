# Fedora / RHEL (RPM)

Recipe: `packaging/rpm/zephyr-terminal.spec` (CMake + Ninja via `%cmake`).

```bash
sudo dnf install rpm-build rpmdevtools 'dnf-command(builddep)'
sudo dnf builddep packaging/rpm/zephyr-terminal.spec
bash packaging/rpm/build_rpm.sh          # -> dist/zephyr-terminal-2.6.0-1.<dist>.<arch>.rpm (+ .src.rpm)
sudo dnf install dist/zephyr-terminal-2.6.0-1.*.$(uname -m).rpm
```

`%check` validates the desktop entry (`desktop-file-validate`) and AppStream
metadata (`appstream-util validate-relax`) and runs the full test suite
(`%ctest`). Build dependencies use `cmake(Qt6Widgets)`/`cmake(Qt6Svg)`
virtual provides, so they resolve on Fedora and EPEL-enabled RHEL clones.

To build in a clean chroot: `mock -r fedora-41-x86_64 --rebuild dist/zephyr-terminal-2.6.0-1.*.src.rpm`.

Runtime: Qt libraries are picked up automatically from linked sonames;
`qt6-qtwayland` is `Recommends:` for native Wayland windows.
