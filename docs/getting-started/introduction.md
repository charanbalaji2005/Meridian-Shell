---
layout: default
title: "Meridian Shell"
category: "GETTING STARTED"
status: "implemented"
---


<div class="hero-section">
  <p class="hero-lead">
    A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.
  </p>
  <div class="hero-actions">
    <a href="#installation" class="btn-primary">Get Started</a>
    <a href="#development-building" class="btn-secondary">Build from Source</a>
    <a href="https://github.com/charanbalaji2005/Meridian-Shell" target="_blank" rel="noreferrer" class="btn-outline">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 22v-4a4.8 4.8 0 0 0-1-3.5c3 0 6-2 6-5.5.08-1.25-.27-2.48-1-3.5.28-1.15.28-2.35 0-3.5 0 0-1 0-3 1.5-2.64-.5-5.36-.5-8 0C6 2 5 2 5 2c-.3 1.15-.3 2.35 0 3.5A5.403 5.403 0 0 0 4 9c0 3.5 3 5.5 6 5.5-.39.49-.68 1.05-.85 1.65-.17.6-.22 1.23-.15 1.85v4"/><path d="M9 18c-4.51 2-5-2-7-2"/></svg>
      GitHub
    </a>
  </div>
</div>

<div class="screenshot-container">
  <img src="./assets/meridian_screenshot.png" alt="Meridian Terminal live session running on Fedora Linux" class="meridian-live-screenshot" />
  <div class="screenshot-label">
    <span>Meridian Shell 2.0 on Fedora Linux (x86_64) running Hyprland with anime artwork header (Theme 2 / Ribbon Girl) and live Fastfetch hardware metrics.</span>
  </div>
</div>

<h2 id="quick-start">Quick Start</h2>
<p>You can run or install Meridian Shell on any Linux system through local package archives or the universal turnkey installer.</p>

<div class="note-box note-info">
  <div class="note-title">📌 Important Notice on Package Repositories</div>
  <p>Commands like <code>sudo dnf install meridian-terminal</code> require the Meridian repository to be enabled first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>
</div>

<h3>1. Local Package Installation</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Fedora / RHEL (Local RPM)</span></div><pre><code class="language-bash">sudo dnf install ./meridian-terminal.rpm</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Ubuntu / Debian (Local DEB)</span></div><pre><code class="language-bash">sudo apt install ./meridian-terminal.deb</code></pre></div>
<div class="code-block-wrapper"><div class="code-header"><span>Arch Linux (Local Package)</span></div><pre><code class="language-bash">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre></div>

<h3>2. Universal Turnkey Installer</h3>
<div class="code-block-wrapper"><div class="code-header"><span>Bash 1-Liner</span></div><pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre></div>

<h2 id="what-is-meridian">What is Meridian?</h2>
<p><strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing.</p>

<h2 id="core-features">Core Features</h2>
<div class="feature-cards-grid">
  <div class="feat-card"><div class="feat-icon">⚡</div><div class="feat-title">Real PTY Sessions</div><div class="feat-body">Asynchronous POSIX openpty multiplexer with non-blocking I/O.</div></div>
  <div class="feat-card"><div class="feat-icon">🖥️</div><div class="feat-title">VT / ANSI Emulation</div><div class="feat-body">Full escape sequence parsing, alternate screen buffers, 24-bit TrueColor.</div></div>
  <div class="feat-card"><div class="feat-icon">🖼️</div><div class="feat-title">Direct Raster Graphics</div><div class="feat-body">Direct 32-bit RGBA inline image decoding with zero ASCII downsampling.</div></div>
  <div class="feat-card"><div class="feat-icon">🪟</div><div class="feat-title">Tabs & Panes</div><div class="feat-body">Multi-pane splits, pane zooming, and persistent workspace layouts.</div></div>
</div>

<h2 id="architecture">Architecture</h2>
<div class="arch-diagram-block"><pre><code class="language-text">Meridian Application
   ↓
Terminal UI (Qt / Canvas)
   ↓
Terminal Core (PTY, VT Parser, ScreenBuffer, Input, Shell Engine)
   ↓
Graphics / Renderer (Text Renderer, GPU Renderer, Image Renderer)
   ↓
Platform Layer (Linux, macOS, Windows)</code></pre></div>

<h2 id="direct-image-rendering">Direct Image Rendering</h2>
<p>Executing <code>pic image.png</code> produces a <strong>direct full-color raster image</strong> on the terminal canvas.</p>
<div class="flow-diagram">
  <div class="flow-box">PNG / JPEG / WebP / BMP</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">graphics::ImageDecoder (stb_image)</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">32-bit RGBA Pixels</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">GPU Texture</div>
  <div class="flow-sep">→</div>
  <div class="flow-box highlight-box">ACTUAL IMAGE PIXELS ON SCREEN</div>
</div>

<h2 id="terminal-compatibility">Terminal Compatibility</h2>
<p>Meridian is tested against standard Linux CLI applications: <code>bash</code>, <code>zsh</code>, <code>fish</code>, <code>ssh</code>, <code>sudo</code>, <code>vim</code>, <code>nano</code>, <code>tmux</code>, <code>htop</code>, <code>git</code>, <code>python</code>, <code>node</code>, <code>docker</code>.</p>

<h2 id="packaging">Packaging</h2>
<p>Available for Fedora (RPM), Debian/Ubuntu (.deb), Arch Linux (PKGBUILD), and Universal Linux tarballs.</p>

<h2 id="project-status">Project Status</h2>
<p>PTY Core: <span class="status-tag status-impl">IMPLEMENTED</span> | VT Engine: <span class="status-tag status-impl">IMPLEMENTED</span> | Direct Images: <span class="status-tag status-impl">IMPLEMENTED</span> | GPU Renderer: <span class="status-tag status-dev">DEVELOPMENT</span></p>
