export interface DocArticle {
  id: string;
  title: string;
  category: string;
  status?: 'implemented' | 'development' | 'experimental' | 'planned';
  lastUpdated: string;
  headings: { id: string; text: string; level: number }[];
  summary: string;
  body: string;
}

export const DOCS_ARTICLES: Record<string, DocArticle> = {
  'intro': {
    id: 'intro',
    title: 'Meridian Shell',
    category: 'GETTING STARTED',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.',
    headings: [
      { id: 'quick-start', text: 'Quick Start', level: 2 },
      { id: 'what-is-meridian', text: 'What is Meridian?', level: 2 },
      { id: 'core-features', text: 'Core Features', level: 2 },
      { id: 'architecture', text: 'Architecture', level: 2 },
      { id: 'direct-image-rendering', text: 'Direct Image Rendering', level: 2 },
      { id: 'terminal-compatibility', text: 'Terminal Compatibility', level: 2 },
      { id: 'packaging', text: 'Packaging', level: 2 },
      { id: 'project-status', text: 'Project Status', level: 2 },
    ],
    body: `
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
  <p>Commands like <code>sudo dnf install meridian-terminal</code> or <code>sudo apt install meridian-terminal</code> require the Meridian repository to be enabled on your system first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>
</div>

<h3>1. Local Package Installation</h3>

<div class="code-block-wrapper">
  <div class="code-header"><span>Fedora / RHEL (Local RPM)</span></div>
  <pre><code class="language-bash">sudo dnf install ./meridian-terminal.rpm</code></pre>
</div>

<div class="code-block-wrapper">
  <div class="code-header"><span>Ubuntu / Debian (Local DEB)</span></div>
  <pre><code class="language-bash">sudo apt install ./meridian-terminal.deb</code></pre>
</div>

<div class="code-block-wrapper">
  <div class="code-header"><span>Arch Linux (Local Package)</span></div>
  <pre><code class="language-bash">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre>
</div>

<h3>2. Universal Turnkey Installer (Any Linux)</h3>
<div class="code-block-wrapper">
  <div class="code-header"><span>Bash 1-Liner</span></div>
  <pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>
</div>

<h3>3. Future / Public Repository Installation</h3>
<p>To enable the automated DNF repository for seamless <code>sudo dnf update</code> management:</p>
<div class="code-block-wrapper">
  <div class="code-header"><span>Fedora DNF Repository Setup</span></div>
  <pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/setup_dnf_repo.sh | sudo bash
sudo dnf install meridian-terminal</code></pre>
</div>

<h2 id="what-is-meridian">What is Meridian?</h2>
<p>
  <strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing.
</p>
<p>
  Instead of treating terminal graphics as an afterthought or relying on character-cell ASCII approximations, Meridian is engineered around a <strong>unified PTY, terminal emulation, graphics, and GPU-rendering pipeline</strong>. It is built in modern C++20 for speed, low latency, and deterministic resource utilization.
</p>

<h2 id="core-features">Core Features</h2>
<div class="feature-cards-grid">
  <div class="feat-card">
    <div class="feat-icon">⚡</div>
    <div class="feat-title">Real PTY Sessions</div>
    <div class="feat-body">Asynchronous POSIX <code>openpty</code> session multiplexer with non-blocking I/O and process lifecycle control.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🖥️</div>
    <div class="feat-title">VT / ANSI Emulation</div>
    <div class="feat-body">Standard-compliant escape sequence parsing, alternate screen buffers, cursor management, and 24-bit TrueColor RGB.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🐚</div>
    <div class="feat-title">Shell Integration</div>
    <div class="feat-body">Standalone POSIX AST executor supporting pipelines, redirects, background jobs, variable expansion, and job control.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🎮</div>
    <div class="feat-title">GPU Rendering</div>
    <div class="feat-body">Hardware-accelerated glyph atlas and damage tracking engine blitting only modified screen regions.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🖼️</div>
    <div class="feat-title">Direct Raster Graphics</div>
    <div class="feat-body">Direct 32-bit RGBA inline image decoding with zero ASCII or half-block downsampling.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🎨</div>
    <div class="feat-title">Inline Graphics Protocols</div>
    <div class="feat-body">Chunked Kitty Graphics Protocol (2048-byte frames) and DEC Sixel bitmap compatibility.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🪟</div>
    <div class="feat-title">Tabs & Panes</div>
    <div class="feat-body">Vertical/horizontal split multiplexing, pane zooming, and persistent workspace layout save/restore.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">✨</div>
    <div class="feat-title">Powerline Interface</div>
    <div class="feat-body">Vivid high-contrast status segments with real-time Git branch detection and sub-millisecond redraws.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🌐</div>
    <div class="feat-title">Cross-Platform Core</div>
    <div class="feat-body">Engineered for Linux (openpty), macOS (BSD PTY), and Windows (ConPTY).</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🔑</div>
    <div class="feat-title">Security & Redaction</div>
    <div class="feat-body">Real-time destructive command blocking and automatic API key/bearer token secret redaction.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">🤖</div>
    <div class="feat-title">AI & Developer Tools</div>
    <div class="feat-body">Offline intent translation (<code>ask</code>), automated compiler/runtime diagnostics (<code>diag</code>), and system monitor.</div>
  </div>
  <div class="feat-card">
    <div class="feat-icon">📦</div>
    <div class="feat-title">Multi-OS Packaging</div>
    <div class="feat-body">RPM (.spec), Debian (.deb), Arch (PKGBUILD), Homebrew, and offline air-gapped tarballs.</div>
  </div>
</div>

<h2 id="architecture">Architecture</h2>
<p>Meridian is organized into modular layers separating the UI, terminal state machine, graphics pipelines, and platform bindings:</p>

<div class="arch-diagram-block">
<pre><code class="language-text">Meridian Application
   ↓
Terminal UI (Qt / Canvas)
   ↓
Terminal Core
├── PTY Manager (POSIX openpty / ConPTY)
├── VT Parser (ANSI / XTerm Escapes)
├── Screen Buffer (Cell Matrix & Styles)
├── Input Controller (Keymaps & Shortcuts)
└── Shell Engine (Lexer, Parser & AST Executor)
   ↓
Graphics / Renderer
├── Text Renderer (Glyph Atlas & TrueColor)
├── GPU Renderer (Damage Tracker & Shader Pipeline)
└── Image Renderer (Kitty Protocol, Sixel & RGBA Direct)
   ↓
Platform Layer
├── Linux (x86_64, aarch64)
├── macOS (BSD PTY)
└── Windows (ConPTY)</code></pre>
</div>

<h2 id="direct-image-rendering">Direct Image Rendering</h2>
<p>
  A foundational design tenet of Meridian is that executing <code>pic image.png</code> produces a <strong>direct, authentic full-color raster image</strong> on the terminal canvas.
</p>

<div class="flow-diagram">
  <div class="flow-box">PNG / JPEG / WebP / BMP</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">graphics::ImageDecoder (stb_image)</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">32-bit RGBA Pixel Buffer</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">GPU Hardware Texture</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">TerminalImage Object</div>
  <div class="flow-sep">→</div>
  <div class="flow-box">Image Renderer</div>
  <div class="flow-sep">→</div>
  <div class="flow-box highlight-box">ACTUAL IMAGE PIXELS ON SCREEN</div>
</div>

<div class="note-box note-warning">
  <div class="note-title">⚠️ Zero Character-Cell Conversion</div>
  <p>The image pipeline does <strong>not</strong> convert graphics into:</p>
  <ul style="margin: 6px 0 0 16px;">
    <li>ASCII or colored ASCII text</li>
    <li>Unicode block or half-block (<code>▀</code>/<code>▄</code>) characters</li>
    <li>Braille patterns</li>
    <li>Character-cell background color approximations</li>
  </ul>
</div>

<h2 id="terminal-compatibility">Terminal Compatibility</h2>
<p>
  Meridian is tested against standard Linux CLI applications, shells, and terminal multiplexers to guarantee robust compatibility:
</p>

<table class="doc-table">
  <thead>
    <tr><th>Category</th><th>Target Programs</th><th>Supported Capabilities</th></tr>
  </thead>
  <tbody>
    <tr><td><strong>Interactive Shells</strong></td><td><code>bash</code>, <code>zsh</code>, <code>fish</code></td><td>Prompt redraws, syntax highlighting, completions, history</td></tr>
    <tr><td><strong>Remote & Privileged</strong></td><td><code>ssh</code>, <code>sudo</code>, <code>su</code></td><td>Password prompts, raw PTY pass-through, SIGINT forwarding</td></tr>
    <tr><td><strong>Full-Screen TUI</strong></td><td><code>vim</code>, <code>neovim</code>, <code>nano</code></td><td>Alternate screen buffer, mouse tracking, 24-bit RGB colors</td></tr>
    <tr><td><strong>Multiplexers & Monitors</strong></td><td><code>tmux</code>, <code>top</code>, <code>htop</code>, <code>btop</code></td><td>Cursor addressability, window resizing (SIGWINCH), bold/italic</td></tr>
    <tr><td><strong>Dev Runtimes & Tools</strong></td><td><code>git</code>, <code>python</code>, <code>node</code>, <code>docker</code></td><td>Interactive pagers (<code>less</code>), REPL sessions, streaming logs</td></tr>
  </tbody>
</table>

<h2 id="packaging">Packaging</h2>
<table class="doc-table">
  <thead>
    <tr><th>Platform</th><th>Target Format</th><th>Implementation Status</th></tr>
  </thead>
  <tbody>
    <tr><td><strong>Fedora / RHEL / CentOS</strong></td><td>RPM Package (<code>.spec</code>) & DNF Repository</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td></tr>
    <tr><td><strong>Ubuntu / Debian / Mint</strong></td><td>Debian Package (<code>.deb</code>)</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td></tr>
    <tr><td><strong>Arch Linux / Manjaro</strong></td><td>PKGBUILD & Arch Archive (<code>.pkg.tar.zst</code>)</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td></tr>
    <tr><td><strong>Universal Linux</strong></td><td>Standalone Offline Tarball (<code>.tar.gz</code>)</td><td><span class="status-tag status-impl">IMPLEMENTED</span></td></tr>
    <tr><td><strong>Universal Linux (Container)</strong></td><td>AppImage / Flatpak / Snap</td><td><span class="status-tag status-plan">PLANNED</span></td></tr>
    <tr><td><strong>macOS (Apple Silicon / Intel)</strong></td><td>Homebrew Formula (<code>.rb</code>) & DMG</td><td><span class="status-tag status-dev">DEVELOPMENT</span></td></tr>
    <tr><td><strong>Windows 10 / 11</strong></td><td>PowerShell Installer & ConPTY Adapter</td><td><span class="status-tag status-dev">DEVELOPMENT</span></td></tr>
  </tbody>
</table>

<h2 id="project-status">Project Status</h2>
<p>
  Meridian maintains an honest and verifiable record of component maturity. Status levels:
  <span class="status-tag status-impl">IMPLEMENTED</span> Production-ready & tested;
  <span class="status-tag status-dev">DEVELOPMENT</span> Functional core under active refinement;
  <span class="status-tag status-exp">EXPERIMENTAL</span> Preview feature;
  <span class="status-tag status-plan">PLANNED</span> Designed on roadmap.
</p>

<table class="doc-table">
  <thead>
    <tr><th>Subsystem</th><th>Status</th><th>Notes</th></tr>
  </thead>
  <tbody>
    <tr><td><strong>PTY / Shell Engine</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>POSIX <code>openpty</code>, AST parser, job control, exit code tracking</td></tr>
    <tr><td><strong>VT / Screen Buffer</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>24-bit TrueColor, alternate screen buffer, UTF-8 Nerd Fonts</td></tr>
    <tr><td><strong>Raster Graphics (stb_image)</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Native PNG, JPEG, WebP, BMP direct 32-bit RGBA decoding</td></tr>
    <tr><td><strong>Kitty Graphics Protocol</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>2048-byte safe chunked transmission (<code>m=1</code>/<code>m=0</code>)</td></tr>
    <tr><td><strong>Anime Artwork Gallery</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>14 built-in themes with interactive <code>Ctrl+P</code> / <code>pic set</code></td></tr>
    <tr><td><strong>AI Intent & Diagnostics</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Local <code>ask</code> intent engine and <code>diag</code> compiler diagnostic rulebook</td></tr>
    <tr><td><strong>Security & Redaction</strong></td><td><span class="status-tag status-impl">IMPLEMENTED</span></td><td>Risk classification (Low/Med/High/Critical) and credential masking</td></tr>
    <tr><td><strong>GUI / Windowing</strong></td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>Qt6 / Wayland / X11 rendering backend</td></tr>
    <tr><td><strong>GPU Hardware Renderer</strong></td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>Damage-tracked shader blitter and texture cache</td></tr>
    <tr><td><strong>Autonomous Agent Loop</strong></td><td><span class="status-tag status-exp">EXPERIMENTAL</span></td><td>Multi-step goal execution with self-correcting retry loop</td></tr>
    <tr><td><strong>macOS PTY</strong></td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>Darwin BSD pseudoterminal integration</td></tr>
    <tr><td><strong>Windows ConPTY</strong></td><td><span class="status-tag status-dev">DEVELOPMENT</span></td><td>Windows Console PTY API bridge</td></tr>
  </tbody>
</table>
`
  },

  'installation': {
    id: 'installation',
    title: 'Installation & Build',
    category: 'GETTING STARTED',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Comprehensive installation options across Linux distributions, macOS, and Windows.',
    headings: [
      { id: 'fedora-rpm', text: 'Fedora & RHEL (DNF / RPM)', level: 2 },
      { id: 'debian-ubuntu', text: 'Debian & Ubuntu (APT / DEB)', level: 2 },
      { id: 'arch-linux', text: 'Arch Linux (Pacman / PKGBUILD)', level: 2 },
      { id: 'universal-script', text: 'Universal Turnkey Installer', level: 2 },
      { id: 'building-source', text: 'Building from Source', level: 2 },
    ],
    body: `
<p>Meridian Shell provides native packages for major Linux package managers as well as turnkey build scripts.</p>

<h2 id="fedora-rpm">Fedora & RHEL (DNF / RPM)</h2>
<h3>Option A: Local RPM Package</h3>
<pre><code class="language-bash">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<h3>Option B: Enable Repository</h3>
<pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/setup_dnf_repo.sh | sudo bash
sudo dnf install meridian-terminal</code></pre>

<h2 id="debian-ubuntu">Debian & Ubuntu (APT / DEB)</h2>
<pre><code class="language-bash">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>

<h2 id="arch-linux">Arch Linux (Pacman / PKGBUILD)</h2>
<pre><code class="language-bash">cd packaging/arch
makepkg -si</code></pre>

<h2 id="universal-script">Universal Turnkey Installer</h2>
<pre><code class="language-bash"># System-wide installation
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash

# User-mode installation (~/.local/bin, no root required)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>

<h2 id="building-source">Building from Source</h2>
<pre><code class="language-bash">git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell
make all -j$(nproc)
make test
sudo ./install.sh</code></pre>
`
  },

  'quickstart': {
    id: 'quickstart',
    title: 'Quick Start',
    category: 'GETTING STARTED',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Launch your first session, customize anime artwork, and explore shortcuts.',
    headings: [
      { id: 'first-launch', text: 'First Launch', level: 2 },
      { id: 'theme-selection', text: 'Anime Theme Selection (Ctrl+P / pic)', level: 2 },
      { id: 'palette', text: 'Command Palette (Ctrl+Shift+P)', level: 2 },
    ],
    body: `
<p>Get started with Meridian Terminal in under one minute.</p>

<h2 id="first-launch">First Launch</h2>
<pre><code class="language-bash">meridian          # Start interactive terminal environment
meridian-shell    # Start standalone shell engine</code></pre>

<h2 id="theme-selection">Anime Theme Selection (Ctrl+P / pic)</h2>
<p>Press <code>Ctrl+P</code> or run <code>pic</code> to choose from 14 anime themes:</p>
<pre><code class="language-bash">pic set sharingan_eye   # Sasuke/Itachi Sharingan (Theme 0)
pic set sakura_girl     # Sakura Blossom Girl (Theme 1)
pic set ribbon_girl     # Ribbon Girl (Theme 2)
pic set fan_girl        # Fan Girl (Theme 3)
pic set gojo_purple     # Gojo: Hollow Purple (Theme 5)
pic set random          # Rotate themes on every terminal launch</code></pre>

<h2 id="palette">Command Palette (Ctrl+Shift+P)</h2>
<p>Press <code>Ctrl+Shift+P</code> to fuzzy-search actions across window splits, git inspections, error diagnoses, and theme switching.</p>
`
  },

  'first-run': {
    id: 'first-run',
    title: 'First Run',
    category: 'GETTING STARTED',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Initial terminal configuration, Powerline badge setup, and default paths.',
    headings: [
      { id: 'directories', text: 'Default Directories', level: 2 },
      { id: 'environment', text: 'PATH & Environment Setup', level: 2 },
    ],
    body: `
<p>On initial launch, Meridian sets up default configuration files and asset caches.</p>

<h2 id="directories">Default Directories</h2>
<ul>
  <li><code>~/.config/meridian/config.json</code>: Main configuration file.</li>
  <li><code>~/.config/meridian/gallery/</code>: Anime wallpaper assets.</li>
  <li><code>~/.config/meridian/keybindings.json</code>: User custom shortcuts.</li>
  <li><code>~/.local/share/meridian/history.db</code>: Rich command history database.</li>
</ul>

<h2 id="environment">PATH & Environment Setup</h2>
<p>Ensure <code>~/.local/bin</code> is in your PATH in <code>~/.bashrc</code> or <code>~/.zshrc</code>:</p>
<pre><code class="language-bash">export PATH="$HOME/.local/bin:$PATH"</code></pre>
`
  },

  'getting-started-config': {
    id: 'getting-started-config',
    title: 'Configuration',
    category: 'GETTING STARTED',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Configure themes, font sizes, keybindings, and PTY behaviors.',
    headings: [
      { id: 'json-config', text: 'Configuration Structure', level: 2 },
      { id: 'example-config', text: 'Example config.json', level: 2 },
    ],
    body: `
<p>Meridian uses standard JSON for configuration with automatic live reloading.</p>

<h2 id="json-config">Configuration Structure</h2>
<pre><code class="language-json">{
  "theme": "sharingan_eye",
  "random_theme": false,
  "font_family": "FantasqueSansMNFM",
  "font_size": 14,
  "cursor_shape": "block",
  "cursor_blink": true,
  "scrollback_lines": 10000,
  "ai_enabled": true,
  "secret_redaction": true
}</code></pre>
`
  },

  'terminal-emulation': {
    id: 'terminal-emulation',
    title: 'Terminal Emulation',
    category: 'TERMINAL',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'VT100, XTerm, and ANSI escape sequences, 24-bit TrueColor, and alternate screen buffers.',
    headings: [
      { id: 'ansi-vt', text: 'ANSI & VT Sequence Support', level: 2 },
      { id: 'alternate-screen', text: 'Alternate Screen Buffer', level: 2 },
      { id: 'truecolor', text: '24-Bit TrueColor RGB', level: 2 },
    ],
    body: `
<p>Meridian provides a standards-compliant terminal emulation state machine.</p>

<h2 id="ansi-vt">ANSI & VT Sequence Support</h2>
<ul>
  <li>CSI sequences (Cursor positioning, erasing, scrolling, modes).</li>
  <li>OSC sequences (Window titles, clipboard access, palette modifications).</li>
  <li>DecPrivateModes (Mouse tracking, bracketed paste, application cursor keys).</li>
</ul>

<h2 id="alternate-screen">Alternate Screen Buffer</h2>
<p>Full support for full-screen curses applications (<code>vim</code>, <code>nano</code>, <code>tmux</code>, <code>htop</code>) seamlessly restoring the primary scrollback buffer on exit.</p>

<h2 id="truecolor">24-Bit TrueColor RGB</h2>
<p>Direct support for <code>\\033[38;2;R;G;Bm</code> (foreground) and <code>\\033[48;2;R;G;Bm</code> (background) providing 16.7 million distinct colors.</p>
`
  },

  'terminal-shell': {
    id: 'terminal-shell',
    title: 'Shell',
    category: 'TERMINAL',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Independent POSIX-compliant AST execution engine, pipelines, redirections, and job control.',
    headings: [
      { id: 'ast-executor', text: 'AST Parser & Executor', level: 2 },
      { id: 'pipelines', text: 'Pipelines & Redirections', level: 2 },
      { id: 'jobs', text: 'Job Control', level: 2 },
    ],
    body: `
<p>Meridian includes an embedded POSIX shell engine capable of executing commands without relying on an external <code>bash</code> or <code>sh</code> binary.</p>

<h2 id="ast-executor">AST Parser & Executor</h2>
<p>The parser converts token streams into an Abstract Syntax Tree (AST) supporting sequential, logical, and asynchronous execution.</p>

<h2 id="pipelines">Pipelines & Redirections</h2>
<pre><code class="language-bash">cat log.txt | grep -i error | wc -l > errors_count.txt 2>&1</code></pre>

<h2 id="jobs">Job Control</h2>
<pre><code class="language-bash">Ctrl+Z    # Suspend foreground process
bg        # Continue in background
fg        # Bring to foreground
jobs      # List active jobs</code></pre>
`
  },

  'graphics-gpu': {
    id: 'graphics-gpu',
    title: 'GPU Renderer',
    category: 'GRAPHICS',
    status: 'development',
    lastUpdated: 'August 25, 2026',
    summary: 'Hardware-accelerated shader pipeline, damage tracking, and glyph texture atlas.',
    headings: [
      { id: 'damage-tracking', text: 'Damage Tracking Engine', level: 2 },
      { id: 'glyph-caching', text: 'Glyph Texture Atlas', level: 2 },
    ],
    body: `
<p>The Meridian rendering pipeline optimizes throughput by tracking cell damage regions.</p>

<h2 id="damage-tracking">Damage Tracking Engine</h2>
<p>Calculates minimum bounding boxes around modified terminal cells, avoiding full-screen redraws when only small text updates occur.</p>

<h2 id="glyph-caching">Glyph Texture Atlas</h2>
<p>Pre-renders character glyphs and Nerd Font symbols into a shared GPU texture atlas for single-draw-call blitting.</p>
`
  },

  'graphics-inline-images': {
    id: 'graphics-inline-images',
    title: 'Inline Images',
    category: 'GRAPHICS',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Direct full-color raster rendering for PNG, JPEG, WebP, GIF, and BMP.',
    headings: [
      { id: 'pic-command', text: 'The pic Command', level: 2 },
      { id: 'specs', text: 'Aspect Ratio & Scaling', level: 2 },
    ],
    body: `
<p>Display full-color images directly inside your terminal canvas:</p>

<h2 id="pic-command">The pic Command</h2>
<pre><code class="language-bash">pic tanjiro.png       # Display raw image at (x=30, y=30)
pic --debug img.png   # Inspect RGBA texture format and dimensions
pic --clear           # Remove active image from terminal canvas</code></pre>

<h2 id="specs">Aspect Ratio & Scaling</h2>
<p>Images automatically scale using contain fitting (default max width 220px, max height 220px) preserving exact source aspect ratios without distortion.</p>
`
  },

  'dev-architecture': {
    id: 'dev-architecture',
    title: 'Architecture',
    category: 'DEVELOPER',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Detailed architecture breakdown of the PTY, VT parser, ScreenBuffer, and renderer layers.',
    headings: [
      { id: 'core-layers', text: 'Subsystem Breakdown', level: 2 },
      { id: 'data-flow', text: 'Terminal Data Flow', level: 2 },
    ],
    body: `
<p>Meridian is built with a clean separation of concerns across 5 modular layers:</p>

<h2 id="core-layers">Subsystem Breakdown</h2>
<ul>
  <li><strong>PTY Layer (<code>src/platform/</code>)</strong>: Manages OS pseudoterminal descriptors and non-blocking I/O.</li>
  <li><strong>VT Parser (<code>src/core/vt/</code>)</strong>: Tokenizes and executes ANSI/XTerm escape sequences.</li>
  <li><strong>Screen Buffer (<code>src/core/vt/</code>)</strong>: 2D grid storing character codes, TrueColor attributes, and cursor state.</li>
  <li><strong>Shell Engine (<code>src/shell/</code>)</strong>: Lexer, parser, builtins, and AST command execution engine.</li>
  <li><strong>Graphics & AI (<code>src/core/graphics/</code>, <code>src/ai/</code>)</strong>: Image decoders, Kitty protocol, intent engine, and diagnostics.</li>
</ul>
`
  },

  'dev-ai-engine': {
    id: 'dev-ai-engine',
    title: 'AI Engine',
    category: 'DEVELOPER',
    status: 'experimental',
    lastUpdated: 'August 25, 2026',
    summary: 'Offline natural language intent translation, compiler diagnostics, and autonomous coding agent.',
    headings: [
      { id: 'intent-translation', text: 'Natural Language Intent (ask)', level: 2 },
      { id: 'error-diagnostics', text: 'Compiler Diagnostics (diag)', level: 2 },
      { id: 'agent-loop', text: 'Autonomous Coding Agent (agent)', level: 2 },
    ],
    body: `
<p>Meridian integrates local AI capabilities directly into the terminal without mandatory cloud dependencies.</p>

<h2 id="intent-translation">Natural Language Intent (ask)</h2>
<pre><code class="language-bash">meridian ask "find all python files modified in last 3 days"</code></pre>

<h2 id="error-diagnostics">Compiler Diagnostics (diag)</h2>
<pre><code class="language-bash">meridian diag "undefined reference to pthread_create"</code></pre>

<h2 id="agent-loop">Autonomous Coding Agent (agent)</h2>
<pre><code class="language-bash">meridian agent "find all memory leaks in tests/ and fix them"</code></pre>
`
  },

  'config-file': {
    id: 'config-file',
    title: 'Configuration File',
    category: 'CONFIGURATION',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Detailed specification of ~/.config/meridian/config.json options.',
    headings: [
      { id: 'schema', text: 'Config Schema & Keys', level: 2 },
    ],
    body: `
<p>Meridian loads settings from <code>~/.config/meridian/config.json</code> at startup.</p>

<h2 id="schema">Config Schema & Keys</h2>
<table class="doc-table">
  <thead>
    <tr><th>Key</th><th>Type</th><th>Default</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr><td><code>theme</code></td><td>string</td><td><code>"sharingan_eye"</code></td><td>Default anime artwork theme</td></tr>
    <tr><td><code>random_theme</code></td><td>boolean</td><td><code>false</code></td><td>Rotate themes on startup</td></tr>
    <tr><td><code>font_size</code></td><td>number</td><td><code>14</code></td><td>Terminal font point size</td></tr>
    <tr><td><code>cursor_shape</code></td><td>string</td><td><code>"block"</code></td><td><code>"block"</code>, <code>"beam"</code>, or <code>"underline"</code></td></tr>
    <tr><td><code>scrollback_lines</code></td><td>number</td><td><code>10000</code></td><td>Max lines in scrollback buffer</td></tr>
  </tbody>
</table>
`
  },

  'pkg-fedora': {
    id: 'pkg-fedora',
    title: 'Fedora / RPM',
    category: 'PACKAGING',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Building and installing RPM packages with DNF and spec files.',
    headings: [
      { id: 'dnf-install', text: 'Installing via DNF', level: 2 },
      { id: 'rpmbuild', text: 'Building RPM with rpmbuild', level: 2 },
    ],
    body: `
<p>Meridian provides official RPM spec files for Fedora, RHEL, and openSUSE.</p>

<h2 id="dnf-install">Installing via DNF</h2>
<pre><code class="language-bash"># Install local RPM package
sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<h2 id="rpmbuild">Building RPM with rpmbuild</h2>
<pre><code class="language-bash">rpmbuild -ba packaging/rpm/meridian-terminal.spec</code></pre>
`
  },

  'development-building': {
    id: 'development-building',
    title: 'Building',
    category: 'DEVELOPMENT',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Compiling Meridian Terminal with C++20, Make, and dependencies.',
    headings: [
      { id: 'prerequisites', text: 'Prerequisites', level: 2 },
      { id: 'make-targets', text: 'Make Targets', level: 2 },
    ],
    body: `
<p>Meridian is compiled using standard C++20 and Make.</p>

<h2 id="prerequisites">Prerequisites</h2>
<ul>
  <li><strong>Compiler</strong>: GCC 11+ or Clang 13+ with C++20 support.</li>
  <li><strong>Build Tools</strong>: GNU Make, <code>libutil-devel</code> (or <code>glibc-devel</code> on Linux).</li>
</ul>

<h2 id="make-targets">Make Targets</h2>
<pre><code class="language-bash">make all -j$(nproc)    # Build all binaries (meridian, meridian-shell, tests)
make test              # Run full automated test suite
make clean             # Remove build artifacts
sudo ./install.sh      # Install binaries and assets system-wide</code></pre>
`
  },

  'development-testing': {
    id: 'development-testing',
    title: 'Testing',
    category: 'DEVELOPMENT',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Automated test suites covering PTY, VT parsing, AST executor, and graphics.',
    headings: [
      { id: 'running-tests', text: 'Running Tests', level: 2 },
    ],
    body: `
<p>Meridian includes 126 unit and integration tests verifying all core subsystems:</p>
<pre><code class="language-bash">make test</code></pre>
<p><strong>Test Suites Covered:</strong></p>
<ul>
  <li><code>test_pty_manager</code>: Linux openpty lifecycle, signals, and non-blocking I/O.</li>
  <li><code>test_screen_buffer</code>: Cell grids, damage regions, and scroll operations.</li>
  <li><code>test_shell_executor</code>: POSIX pipelines, redirects, and job control.</li>
  <li><code>test_graphics</code>: Kitty graphics chunks and stb_image RGBA decoding.</li>
</ul>
`
  },

  'ref-cli': {
    id: 'ref-cli',
    title: 'CLI Reference',
    category: 'REFERENCE',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Complete command-line interface options and subcommands.',
    headings: [
      { id: 'subcommands', text: 'Meridian Subcommands', level: 2 },
    ],
    body: `
<h2 id="subcommands">Meridian Subcommands</h2>
<table class="doc-table">
  <thead>
    <tr><th>Command</th><th>Arguments</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr><td><code>meridian</code></td><td>None</td><td>Launch interactive Meridian terminal environment</td></tr>
    <tr><td><code>meridian update</code></td><td>None</td><td>Update Meridian to latest GitHub release</td></tr>
    <tr><td><code>meridian uninstall</code></td><td><code>[--purge]</code></td><td>Remove binaries, desktop files, and configs</td></tr>
    <tr><td><code>meridian pic</code></td><td><code>&lt;file&gt;</code></td><td>Display raw full-color inline raster image</td></tr>
    <tr><td><code>meridian pic set</code></td><td><code>&lt;id|name|path&gt;</code></td><td>Set permanent startup anime theme</td></tr>
    <tr><td><code>meridian ask</code></td><td><code>"&lt;intent&gt;"</code></td><td>Translate human description to safe shell command</td></tr>
    <tr><td><code>meridian diag</code></td><td><code>"&lt;error&gt;"</code></td><td>Diagnose compiler or runtime error</td></tr>
    <tr><td><code>meridian monitor</code></td><td>None</td><td>Display real-time CPU, RAM, Disk, and Process metrics</td></tr>
    <tr><td><code>meridian git</code></td><td>None</td><td>Inspect Git branch divergence and staged changes</td></tr>
    <tr><td><code>meridian --version</code></td><td>None</td><td>Display version and copyright info</td></tr>
  </tbody>
</table>
`
  },

  'proj-license': {
    id: 'proj-license',
    title: 'License',
    category: 'PROJECT',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Free and open-source GNU General Public License v3.0 or later.',
    headings: [
      { id: 'gplv3', text: 'GNU GPL-3.0-or-later', level: 2 },
    ],
    body: `
<h2 id="gplv3">GNU GPL-3.0-or-later</h2>
<p>
  Meridian Terminal is free and open-source software released under the terms of the <strong>GNU General Public License v3.0 or later</strong>.
</p>
<p>
  You are free to use, modify, distribute, and contribute back to the project under the terms of the GPLv3 license.
</p>
`
  },

  'proj-contributing': {
    id: 'proj-contributing',
    title: 'Contributing',
    category: 'PROJECT',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Guidelines for community contributions, bug fixes, visual glitches, and artwork.',
    headings: [
      { id: 'welcome', text: 'Welcome Contributors!', level: 2 },
      { id: 'guidelines', text: 'How to Submit Contributions', level: 2 },
    ],
    body: `
<div class="note-box note-success">
  <div class="note-title">🤝 Open Source Community Welcome!</div>
  <p>Contributions of all kinds are warmly welcomed! Whether you want to fix bugs, resolve visual glitches, add anime artwork themes, implement new developer tooling, or improve POSIX shell compatibility, pull requests and issues are highly encouraged.</p>
</div>

<h2 id="guidelines">How to Submit Contributions</h2>
<ol>
  <li>Fork the repository on GitHub: <code>https://github.com/charanbalaji2005/Meridian-Shell</code></li>
  <li>Create your feature branch: <code>git checkout -b feat/my-improvement</code></li>
  <li>Run all unit tests to ensure no regressions: <code>make test</code></li>
  <li>Open a Pull Request with a clear description of your changes.</li>
</ol>
`
  }
};
