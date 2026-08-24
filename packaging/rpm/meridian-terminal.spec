Name:           meridian-terminal
Version:        0.1.0
Release:        1%{?dist}
Summary:        Independently engineered Linux terminal emulator with built-in shell and local AI

License:        GPL-3.0-or-later
URL:            https://github.com/meridian-terminal/meridian-terminal
Source0:        https://github.com/meridian-terminal/meridian-terminal/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ >= 11
BuildRequires:  make
BuildRequires:  glibc-devel
BuildRequires:  libutil-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

Requires:       hicolor-icon-theme

%description
Meridian Terminal is an independently engineered Linux terminal emulator (C++20)
featuring a high-performance ANSI/VT engine, an optional standalone Meridian
Shell with full POSIX job control, and an offline local-only Meridian AI assistant.
No external network connection is required.

%prep
%autosetup -n %{name}-%{version}

%build
%make_build all

%check
./build/meridian_tests
./tests/manual_core_test.sh

%install
%make_install PREFIX=%{_prefix}

%files
%license LICENSE
%doc README.md CHANGELOG.md
%{_bindir}/meridian
%{_bindir}/meridian-shell
%{_bindir}/meridian_demo
%{_datadir}/applications/org.meridian_terminal.MeridianTerminal.desktop
%{_datadir}/icons/hicolor/scalable/apps/meridian-terminal.svg
%{_datadir}/metainfo/org.meridian_terminal.MeridianTerminal.metainfo.xml
%{_datadir}/doc/meridian-terminal/

%changelog
* Tue Aug 25 2026 Charan Balaji <charan@example.com> - 0.1.0-1
- Initial release of Meridian Terminal, Meridian Shell, and local Meridian AI.

