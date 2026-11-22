# AppImage

Script: `packaging/appimage/build_appimage.sh` — builds with CMake, installs
into an AppDir and bundles Qt with
[linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) +
linuxdeploy-plugin-qt.

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-svg-dev \
                 qt6-wayland libgl-dev qmake6 qt6-base-dev-tools file wget desktop-file-utils
bash packaging/appimage/build_appimage.sh   # -> dist/Zephyr-Terminal-2.6.0-x86_64.AppImage
```

- **Compatibility**: an AppImage runs on distros with the same or newer
  glibc than the build host. Building on Ubuntu 24.04 (the oldest common
  distro with Qt ≥ 6.4) gives glibc 2.39.
- **Wayland**: if `qt6-wayland` is installed at build time, the Wayland
  platform plugin is bundled; otherwise the AppImage uses XWayland.
- **Licensing**: Qt is LGPL and dynamically linked inside the AppImage,
  which is permitted; Zephyr itself is GPL-3.0-or-later.
- **Reproducibility**: linuxdeploy only publishes rolling `continuous`
  builds. Pin them by downloading once and setting `LINUXDEPLOY=` and
  `LINUXDEPLOY_QT=` to your verified copies.
- **No FUSE?** Run with `APPIMAGE_EXTRACT_AND_RUN=1` or `--appimage-extract`.

The AppImage launches the GUI. The bundled CLI tools are inside
(`--appimage-extract` then `squashfs-root/usr/bin/zephyr`). To add the
AppImage to your menu use **Settings → Linux Integration → Install
launcher for my user**, which points the launcher at the `.AppImage` file
(via `$APPIMAGE`) rather than its temporary mount.
