# 🚀 Meridian Terminal 2.0 — Cross-Platform Terminal & Developer Platform

[![CI/CD](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/ci.yml/badge.svg)](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/ci.yml)
[![Pages](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/pages.yml/badge.svg)](https://charanbalaji2005.github.io/Meridian-Shell/)
[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPLv3+-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: Linux | macOS | Windows](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-brightgreen.svg)](#)

**Meridian Terminal** is a high-performance, Kitty-class cross-platform terminal emulator, multiplexer, developer intelligence platform, and local AI agent built with **C++20 & Qt 6**.

📖 **Live Website & Documentation**: [https://charanbalaji2005.github.io/Meridian-Shell/](https://charanbalaji2005.github.io/Meridian-Shell/)

---

## 🎨 Vivid High-Contrast Powerline & Native Graphics

```text
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   .---.    *   .       charanbalaji@fedora                  │
│  /     \    .    *     ───────⭘───────                      │
│  \     /   .     .     󰌽 → Linux 7.1.5-201.fc44             │
│   '---'       *        󰨇 → Hyprland 0.56.1 (Wayland)        │
│   /\     /\     /\     󰞷 → zsh 5.9                          │
│  /  \   /  \   /  \     → meridian 2.0                      │
│ / /\ \ / /\ \ / /\ \   󰘚 → 5.77 GiB / 15.25 GiB              │
│~~~~~~~~~~~~~~~~~~~~~   󱑂 → 1 hour, 11 mins                  │
│  ~ ~  ~  ~  ~ ~ ~ ~    ───────⭘───────                      │
│                        ● ● ● ● ● ● ● ● ●                    │
│                                                             │
│  [Tue 25 Aug - 09:40]  [~/Downloads/meridian]  [ origin  main 6✸ 8●] 
│  [@charanbalaji]  echo "Welcome to Meridian Terminal"      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Offline Multi-OS Installation

Download the package once and install completely offline on any laptop or operating system:

### 1. Fedora / RHEL / CentOS / openSUSE (`dnf` / RPM)
```bash
# Build RPM from spec or download release asset:
rpmbuild -ba packaging/rpm/meridian-terminal.spec

# Install offline with DNF:
sudo dnf install ./dist/meridian-terminal.rpm
```

### 2. Ubuntu / Debian / Linux Mint (`apt` / DEB)
```bash
# Generate package:
./scripts/package_offline.sh

# Install offline with APT:
sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb
# or
sudo dpkg -i ./dist/meridian-terminal_2.0.0_amd64.deb
```

### 3. Arch Linux / Manjaro / EndeavourOS (`pacman`)
```bash
# Build and install:
cd packaging/arch && makepkg -si

# Or install from offline package file:
sudo pacman -U ./dist/meridian-terminal-2.0.0-1-x86_64.pkg.tar.zst
```

### 4. Universal Linux Portable (AppImage / Zero-Install)
```bash
# Extract and run without installation or root:
tar -xzf dist/Meridian-Terminal-2.0.0-x86_64.AppDir.tar.gz
./AppDir/AppRun
```

### 5. macOS (MacBooks - Intel & Apple Silicon M1/M2/M3/M4)
```bash
# Install via local Homebrew formula:
brew install --build-from-source packaging/macos/meridian-terminal.rb
```

### 6. Windows 10 / 11 (PowerShell & ConPTY)
```powershell
# Open PowerShell and run:
.\packaging\windows\install.ps1
```

### 7. Universal Turnkey Installer
```bash
# Build & install directly (system-wide):
sudo ./install.sh

# Or install for your user only (~/.local/bin, no root required):
./install.sh --user
```

---

## ⚡ Features at a Glance

* **🎨 Native Graphics Engine (`pic`)**:
  * `pic <file>` → TrueColor 24-bit half-blocks (`▀`/`▄`)
  * `pic <file> --ascii` → Grayscale ASCII density
  * `pic <file> --color-ascii` → Bright colored ASCII
  * `pic <file> --hybrid` → Hybrid background overlay
  * **`Ctrl+Shift+P`** → Interactive in-terminal image uploader and selector.
* **🛡️ Terminal-Native Safety & Risk Defense**: Real-time detection of destructive operations (`rm -rf /`, `curl | bash`, `dd`, `git push --force`, credential leaks).
* **🤖 Inline AI Engine (`ai`)**:
  * `ai "<query>"` → Natural language intent translation into safe shell commands.
  * `ai "<error message>"` → Automated compiler/runtime diagnosis and suggested fixes.
* **🪟 Multiplexer & Panes**: Splits (Horizontal/Vertical) and Zoom tabs.
* **⚡ Cross-Platform PTY Core**: Linux (`openpty`), macOS (BSD PTY), and Windows (`ConPTY`).

---

## 🧪 Building & Testing

```bash
# Compile all targets
make all -j$(nproc)

# Run full test suite (124 unit & integration tests)
make test

# Run manual PTY core tests
make manual-test

# Generate all offline distribution bundles in dist/
./scripts/build_all_releases.sh
```

---

## 📄 License

Meridian Terminal is free and open-source software licensed under the **GNU General Public License v3.0 or later** ([GPL-3.0-or-later](LICENSE)).
