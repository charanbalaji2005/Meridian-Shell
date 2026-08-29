# 🚀 Meridian Terminal 2.5 — Modern Cross-Platform Terminal & Developer Platform

[![CI/CD](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/ci.yml/badge.svg)](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/charanbalaji2005/Meridian-Shell?color=brightgreen&label=release)](https://github.com/charanbalaji2005/Meridian-Shell/releases)
[![Pages](https://github.com/charanbalaji2005/Meridian-Shell/actions/workflows/pages.yml/badge.svg)](https://charanbalaji2005.github.io/Meridian-Shell/)
[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPLv3+-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: Linux | macOS | Windows](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-brightgreen.svg)](#)

**Meridian Terminal 2.5** is a high-performance cross-platform terminal emulator, standalone POSIX shell, and developer environment featuring native GPU raster graphics, live Git intelligence, interactive anime artwork galleries, multi-pane multiplexing, 1-click GitHub integration, and built-in offline AI assistance.

📖 **Live Documentation & Guides**: [https://charanbalaji2005.github.io/Meridian-Shell/](https://charanbalaji2005.github.io/Meridian-Shell/)

<p align="center">
  <img src="resources/images/meridian_screenshot.png" alt="Meridian Terminal 2.5 Screenshot" width="100%" style="border-radius: 8px; box-shadow: 0 4px 20px rgba(0,0,0,0.5);" />
</p>

---

## ⚡ 1-Line Universal Installer (Any Linux, WSL, or macOS)

```bash
# Recommended (Auto-detects OS, Architecture, and downloads native prebuilt release):
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```

> **For System-Wide Installation (installs to `/usr/local/bin` with `sudo`):**
> ```bash
> curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash
> ```

---

## 📦 Installation Commands by Operating System

### 1. 🐧 Ubuntu / Debian / Pop!_OS / Linux Mint

#### Option A: Native `.deb` Package (Recommended)
```bash
# Download and install latest Debian/Ubuntu package
curl -fsSLO https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal_2.5.0_amd64.deb
sudo apt install ./meridian-terminal_2.5.0_amd64.deb
```

#### Option B: APT Repository
```bash
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/packaging/deb/setup_apt.sh | sudo bash
sudo apt update
sudo apt install meridian-terminal
```

---

### 2. 🎩 Fedora / RHEL / CentOS / Rocky Linux / AlmaLinux

#### Option A: Native `.rpm` Package (Recommended)
```bash
# Direct DNF installation of prebuilt RPM
sudo dnf install https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal-2.5.0-1.fc44.x86_64.rpm
```

#### Option B: DNF Repository
```bash
# Enable Meridian repository and install
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/setup_dnf_repo.sh | sudo bash
sudo dnf install meridian-terminal
```

---

### 3. 🏹 Arch Linux / Manjaro

#### Option A: AUR (Arch User Repository)
```bash
yay -S meridian-terminal
# or:
paru -S meridian-terminal
```

#### Option B: Standalone Prebuilt Tarball
```bash
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```

---

### 4. 🦎 openSUSE / SLES

```bash
sudo zypper install https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/meridian-terminal-2.5.0-1.fc44.x86_64.rpm
```

---

### 5. 🏔️ Alpine Linux

```bash
# Install runtime dependencies and extract binary
apk add libstdc++ libgcc bash
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```

---

### 6. 🪟 Windows Subsystem for Linux (WSL / WSL2)

```bash
# Inside your Ubuntu/Debian/Fedora WSL terminal:
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash
```

> **WSL Display Modes:**
> - **Console / Terminal Shell**: Runs directly in Windows Terminal or VS Code (`meridian-shell`).
> - **Graphical Window (WSLg)**: If running Windows 11 with WSLg, `meridian` automatically launches its hardware-accelerated GUI window.

---

### 7. 🍏 macOS (Apple Silicon & Intel)

#### Option A: Homebrew Cask (Recommended)
```bash
brew tap charanbalaji2005/meridian https://github.com/charanbalaji2005/Meridian-Shell
brew install --cask meridian-terminal
```

#### Option B: Direct `.dmg` Disk Image
- **Apple Silicon (M1/M2/M3/M4/M5)**: [Download `Meridian-Terminal-2.5.0-macOS-arm64.dmg`](https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/Meridian-Terminal-2.5.0-macOS-arm64.dmg)
- **Intel x86_64**: [Download `Meridian-Terminal-2.5.0-macOS-x86_64.dmg`](https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v2.5.0/Meridian-Terminal-2.5.0-macOS-x86_64.dmg)

Drag **Meridian.app** to your `/Applications` folder.

---

### 8. 🪟 Windows Native (PowerShell)

```powershell
# In PowerShell (Run as Administrator):
irm https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/dist/install.ps1 | iex
```

---

## 🔑 Connecting to GitHub with Meridian (`meridian gh`)

Meridian features a built-in **1-Click Developer Experience GitHub Integration** that sets up SSH authentication, configures Port 443 fallback (bypassing network restrictions), and links repositories without asking for passwords.

```text
┌─── GitHub Integration Status ────────────────────────────────────────────────┐
│ Account : @charanbalaji2005
│ SSH     : ✓ Connected (Port 443)
│ GitHub  : ✓ Authenticated
│ Key     : ~/.ssh/id_ed25519
│ Remote  : git@github.com:charanbalaji2005/Meridian-Shell.git
└──────────────────────────────────────────────────────────────────────────────┘
```

### 1️⃣ Connect Your GitHub Account (1-Click)
```bash
meridian gh connect
```
- Automatically detects or generates an ED25519 SSH key (`~/.ssh/id_ed25519` or `~/.ssh/meridian_github_ed25519`) **without overwriting existing keys**.
- Configures `~/.ssh/config` to use **Port 443** so it works on all Wi-Fi and corporate networks.
- Tests the live connection and verifies write access.

### 2️⃣ Check Connection Status
```bash
meridian gh status
```

### 3️⃣ View Public SSH Key
```bash
meridian gh key
```

### 4️⃣ Create & Link a New Repository
```bash
meridian gh repo create my-project
```

### 5️⃣ Smart, Non-Blocking Git Hints
When you run `git init` in a new folder, Meridian displays non-blocking next steps to connect and push your project. You can toggle hints on/off anytime:
```bash
meridian gh hints on    # Enable hints
meridian gh hints off   # Disable hints
```

---

## 💻 VS Code Integration (`meridian vscode`)

Auto-detects your VS Code installation and configures Meridian Shell as a ready-to-use integrated terminal profile:

```bash
# Check current VS Code terminal status
meridian vscode status

# Set Meridian as default VS Code terminal
meridian vscode enable

# Reset back to standard bash/zsh
meridian vscode disable
```

---

## 🔄 Updating Meridian (`meridian update`)

Meridian includes a native update engine that checks GitHub Releases, verifies SHA256 checksums, and updates your installation:

```bash
# Check if an update is available
meridian update --check

# Download, verify, and apply the latest update
meridian update
```

---

## 📊 Anonymous Usage Statistics & Telemetry (`meridian stats`)

```bash
# View global usage overview
meridian stats

# View breakdown for a specific year
meridian stats --year 2026

# View yearly adoption growth rates
meridian stats --growth
```

### 🛡️ Privacy-First Telemetry
Meridian's telemetry is **strictly opt-in (disabled by default)** and collects zero personal data, commands, or file paths:

```bash
meridian telemetry status    # View privacy policy & current state
meridian telemetry enable    # Opt-in to anonymous metrics
meridian telemetry disable   # Opt-out completely (Zero network metrics)
```

---

## 🎨 Interactive Anime Artwork Themes

Choose from 14 embedded anime themes or render full-color raster images:

```bash
# Select startup anime wallpaper (0-13)
pic set 0       # Sharingan Eye
pic set 2       # Sakura Girl
pic set 4       # Cyberpunk Edgerunners
pic set 6       # Gojo Satoru
pic set random  # Random wallpaper on every launch

# Render any image inline inside terminal
pic /path/to/image.png
```

---

## 🛠️ Master Command Reference

| Subcommand | Description |
| :--- | :--- |
| `meridian` | Launch interactive GPU-accelerated Meridian Terminal |
| `meridian-shell` | Launch standalone C++20 POSIX shell |
| `meridian gh connect` | 1-Click GitHub SSH key setup & live authentication |
| `meridian gh status` | Inspect GitHub connection and remote repository |
| `meridian gh key` | View public SSH key |
| `meridian vscode enable` | Set Meridian as VS Code default integrated terminal |
| `meridian update` | Update to the latest release with SHA256 verification |
| `meridian stats` | Display anonymous platform metrics and yearly growth |
| `meridian telemetry status` | View opt-in telemetry configuration and privacy policy |
| `monitor` | Live CPU, RAM, Disk, Network metrics dashboard |
| `gitintel` | Visual Git branch divergence and staged/unstaged changes |
| `files [dir]` | Tree file explorer with Git badges |
| `ssh-mgr [alias]` | Native SSH workspace manager (`~/.ssh/config`) |
| `ask "<intent>"` | Translate natural language to safe shell commands |
| `diag "<error>"` | Offline compiler, runtime, and database error diagnostics |
| `palette [query]` | Command Palette with fuzzy search (`Ctrl+Shift+P`) |
| `search "<query>"` | Universal search across screen buffers and history (`Ctrl+Shift+F`) |
| `split [v\|h]` | Binary tree pane split (`Ctrl+Shift+D` / `Ctrl+Shift+E`) |
| `pic <file>` | Direct 32-bit RGBA hardware raster image viewer |
| `perf` | Live GPU framerate, PTY latency & telemetry profiler |

---

## 🤝 Contributing

Contributions of all kinds are warmly welcomed!

```bash
# Clone the repository
git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell

# Build all binaries & run test suite
make all -j$(nproc)
make test
```

Please feel free to open an **Issue** or submit a **Pull Request**.

---

## 📜 License

Dual-licensed under the **GPL-3.0-or-later** / **MIT License**. Copyright © 2025-2026 Charan Balaji and Meridian Contributors.
