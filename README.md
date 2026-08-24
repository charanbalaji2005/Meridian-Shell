# 🚀 Meridian 2.0 — Terminal + Developer Environment + AI Platform

**Meridian Terminal** is a high-performance, GPU-accelerated terminal emulator, multiplexer, developer intelligence platform, and local AI agent built with modern **C++20** for Linux.

```text
                    MERIDIAN 2.0 ARCHITECTURE
                               │
       ┌───────────────────────┼────────────────────────┐
       │                       │                        │
  Terminal Engine         AI Subsystem          Developer Tools
  • 144Hz GPU Quad Buffer • Intent Engine       • System & Net Monitor
  • Kitty / Sixel Graphics• AI Coding Agent     • Git Branch Intel
  • BSP Tree Multiplexer  • Error Diagnostics   • Universal Search
  • Session Recording     • Security & Risk     • Command Palette (Ctrl+Shift+P)
```

---

## 📦 Universal Installation

Anybody can install Meridian Terminal across any major Linux distribution:

### 1. Fedora / RHEL / openSUSE (`dnf` / RPM)
```bash
sudo dnf install meridian-terminal
# Or build RPM from spec:
rpmbuild -ba packaging/rpm/meridian-terminal.spec
```

### 2. Arch Linux / Manjaro (`pacman` / AUR)
```bash
# Via AUR helper:
paru -S meridian-terminal
# or
yay -S meridian-terminal

# Or manually via makepkg:
cd packaging/aur && makepkg -si
```

### 3. Ubuntu / Debian / Pop!_OS / Linux Mint (`apt` / DEB)
```bash
sudo apt install meridian-terminal
# Or build debian package:
cd packaging/deb && dpkg-buildpackage -us -uc -b
```

### 4. Universal Snap Package
```bash
sudo snap install meridian-terminal
```

### 5. Universal Flatpak (Flathub)
```bash
flatpak install flathub org.meridian_terminal.MeridianTerminal
```

### 6. One-Line Installer Script
```bash
# Install to ~/.local/bin (no root required)
./install.sh --user

# Or system-wide:
sudo ./install.sh
```

---

## ⚡ Interactive Command History Preview (Up / Down Navigation)

Meridian Shell features an **interactive graphical command preview overlay**:
- Press **`Up Arrow` (↑)**: Pops up an interactive graphical preview box above/below the prompt showing previous commands, their exit codes, durations, and directories.
- Press **`Down Arrow` (↓)**: Steps forward through history.
- Press **`Enter`**: Executes the selected command immediately.
- Press **`Tab`**: Auto-completes filenames and executables.

```text
┌─── Command History Preview (↑/↓ to navigate, Enter to run) ───┐
│ ▶ #24: git commit -m "Meridian 2.0 release"                   │
│   #23: docker compose up -d                                   │
│   #22: npm run dev                                            │
│   #21: cargo test --workspace                                 │
└───────────────────────────────────────────────────────────────┘
meridian:~/workspace$ git commit -m "Meridian 2.0 release"
```

---

## 🖥️ Graphical Workstation Interface (`meridian gui`)

Launch the visual developer workstation with split panes, tabs, and live telemetry:

```bash
meridian gui
```

- **Top Bar**: Active tabs (`[Tab 1: main]`, `[Tab 2: server]`, `[Tab 3: agent]`) with real-time CPU/RAM meters.
- **Split Panes**: Left side terminal session; Right side live `/proc` CPU/RAM/Disk/Network telemetry & file explorer.
- **Footer**: Live Git branch status (` main ↑0 ↓0 | ✓ Safe | 144Hz GPU Engine`).

---

## 🤖 Meridian AI & Developer CLI

```bash
# 1. Natural Language Intent Translation
meridian ask "find all python files modified in the last 2 days"
meridian ask "kill process running on port 3000"

# 2. Automatic Error Diagnostics & Suggested Fixes
meridian diag "Error: Cannot find module 'express'"
meridian diag "ModuleNotFoundError: No module named 'requests'"

# 3. Autonomous AI Coding & Repair Agent
meridian agent "Diagnose repository and run test suite"

# 4. Developer Intelligence Tools
meridian monitor           # Real-time CPU, RAM, Disk, Net & Top Processes
meridian git               # Git branch divergence, staged/unstaged changes
meridian files [dir]       # Tree file explorer with git status badges
meridian search "<query>"  # Universal search across screens, history & files
meridian palette [query]   # Fuzzy Command Palette (Ctrl+Shift+P)
meridian history           # Rich command history with exit codes & durations
```

---

## 🛠️ Building and Testing Locally

```bash
# Build all binaries (meridian, meridian-shell, meridian_demo, meridian_tests)
make all

# Run comprehensive test suite (124 tests, 376 assertions)
make test

# Run manual PTY and POSIX job control verification
make manual-test

# Install system-wide
sudo make install
```

---

## 🚀 Pushing to GitHub

To push this repository to GitHub:

```bash
# 1. Initialize and add files (respects .gitignore)
git init
git branch -m main
git add .

# 2. Create initial commit
git commit -m "feat: Meridian 2.0 - Terminal + Developer Environment + AI Platform"

# 3. Add your GitHub remote and push
git remote add origin git@github.com:<your-username>/meridian-terminal.git
git push -u origin main
```

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 or later (GPL-3.0-or-later)**. See the [LICENSE](LICENSE) file for details.
