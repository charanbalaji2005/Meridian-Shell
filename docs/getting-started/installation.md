---
layout: default
title: "Installation & Build"
category: "GETTING STARTED"
status: "implemented"
---

<p>Meridian provides native packages for all major Linux distributions, as well as a standalone universal installer.</p>

<h2 id="dnf-rpm">Fedora & RHEL (DNF / RPM)</h2>
<p>To install a locally downloaded RPM package on Fedora 39, 40, 41, or 44:</p>
<pre><code class="language-bash"># Install local RPM archive
sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm

# Or using rpm directly
sudo rpm -Uvh ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<div class="note-box note-warning">
  <div class="note-title">⚠️ Repository Installation Requirement</div>
  <p>To install via <code>sudo dnf install meridian-terminal</code> without specifying a local file path, the official Meridian COPR / RPM repository must be added to <code>/etc/yum.repos.d/</code>.</p>
</div>

<h2 id="apt-deb">Ubuntu & Debian (APT / DEB)</h2>
<p>For Debian 12+, Ubuntu 22.04 LTS, Ubuntu 24.04 LTS, and Linux Mint:</p>
<pre><code class="language-bash">sudo apt update
sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>

<h2 id="arch-pkg">Arch Linux (Pacman / PKGBUILD)</h2>
<p>On Arch Linux, Manjaro, or EndeavourOS:</p>
<pre><code class="language-bash">cd packaging/arch
makepkg -si</code></pre>

<h2 id="opensuse-pkg">openSUSE (Zypper / RPM)</h2>
<p>On openSUSE Tumbleweed or Leap:</p>
<pre><code class="language-bash">sudo zypper install ./meridian-terminal.rpm</code></pre>

<h2 id="universal-script">Universal Turnkey Installer</h2>
<p>The universal installer detects your distribution, installs missing dependencies, compiles the latest release, and places binary launchers into <code>/usr/local/bin</code>:</p>
<pre><code class="language-bash"># Standard user installation (~/.local/bin)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash

# System-wide installation (/usr/local/bin)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre>

<h2 id="building-from-source">Compiling from Source</h2>
<p>Building Meridian from source requires a modern C++20 compiler (GCC 11+ or Clang 13+):</p>
<pre><code class="language-bash">git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell

# Build all binaries (terminal, shell, tests, demo)
make all -j$(nproc)

# Run test suite (133 tests)
make test

# Install to system
sudo make install</code></pre>

<h2 id="dependencies-table">Build & Runtime Dependencies</h2>
<table class="doc-table">
  <thead><tr><th>Package</th><th>Fedora Package</th><th>Debian/Ubuntu Package</th><th>Purpose</th></tr></thead>
  <tbody>
    <tr><td>C++20 Compiler</td><td><code>gcc-c++</code></td><td><code>g++</code></td><td>Core compilation</td></tr>
    <tr><td>POSIX Libutil</td><td><code>glibc-devel</code></td><td><code>libc6-dev</code></td><td>openpty and termios</td></tr>
    <tr><td>Make</td><td><code>make</code></td><td><code>make</code></td><td>Build automation</td></tr>
    <tr><td>Nerd Font</td><td><code>google-noto-sans-fonts</code></td><td><code>fonts-noto</code></td><td>Powerline & glyph rendering</td></tr>
  </tbody>
</table>

<h2 id="uninstalling">Uninstallation & Purge</h2>
<p>Meridian provides a clean uninstaller script that removes all binaries, desktop entries, and icons:</p>
<pre><code class="language-bash"># Standard uninstallation (preserves ~/.config/meridian configs)
sudo ./uninstall.sh

# Complete purge (removes binaries AND config/history databases)
sudo ./uninstall.sh --purge

# For DNF package installations
sudo dnf remove meridian-terminal</code></pre>