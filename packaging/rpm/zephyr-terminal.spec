Name:           zephyr-terminal
Version:        2.6.0
Release:        1%{?dist}
Summary:        Fast, modern terminal emulator with tabs, splits and profiles

License:        GPL-3.0-or-later
URL:            https://github.com/charanbalaji2005/Meridian-Shell
# Built locally by packaging/rpm/build_rpm.sh, or from a release tag:
#   https://github.com/charanbalaji2005/Meridian-Shell/archive/v%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  cmake(Qt6Widgets) >= 6.4
BuildRequires:  cmake(Qt6Svg) >= 6.4
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
# Used by the test suite
BuildRequires:  bash
BuildRequires:  grep

Requires:       hicolor-icon-theme
# Native Wayland windows; without it Qt falls back to XWayland.
Recommends:     qt6-qtwayland
Suggests:       openssh-clients

Provides:       zephyr-shell = %{version}-%{release}

%description
Zephyr Terminal is a native Linux terminal emulator written in C++20 and Qt 6.
It offers tabs, nested split panes, scrollback search, shell and SSH profiles,
themes with transparency, a command palette, editable keybindings and session
restore. The package also ships the "zephyr" command-line toolkit and the
"zephyr-shell" interactive shell.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -G Ninja \
    -DZEPHYR_BUILD_GUI=ON \
    -DZEPHYR_BUILD_TESTS=ON \
    -DZEPHYR_INSTALL_DOCS=OFF \
    -DZEPHYR_INSTALL_LICENSE=OFF
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/org.zephyr_terminal.ZephyrTerminal.desktop
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/org.zephyr_terminal.ZephyrTerminal.metainfo.xml
%ctest

%files
%license LICENSE
%doc README.md CHANGELOG.md
%{_bindir}/zephyr-terminal
%{_bindir}/zephyr
%{_bindir}/zephyr-shell
%{_datadir}/applications/org.zephyr_terminal.ZephyrTerminal.desktop
%{_metainfodir}/org.zephyr_terminal.ZephyrTerminal.metainfo.xml
%{_datadir}/icons/hicolor/*/apps/zephyr-terminal.*
%{_datadir}/zephyr/

%changelog
* Mon Nov 16 2026 Charan Balaji - 2.6.0-1
- New Qt 6 graphical terminal (zephyr-terminal): tabs, split panes, search,
  profiles, SSH, command palette, settings and session restore
- Build with CMake; install desktop entry, AppStream metadata and icons

* Sat Aug 29 2026 Charan Balaji - 2.5.0-1
- Release 2.5 with prebuilt multi-distro installer, VS Code auto-detection,
  and stats/update engine

* Tue Aug 25 2026 Charan Balaji - 2.0.0-1
- Release 2.0 with TrueColor graphics, live Git badges, and unified canvas
