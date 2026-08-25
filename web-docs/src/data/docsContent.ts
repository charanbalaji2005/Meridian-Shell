export interface DocArticle {
  id: string;
  title: string;
  category: string;
  status?: 'implemented' | 'development' | 'experimental';
  lastUpdated: string;
  headings: { id: string; text: string; level: number }[];
  summary: string;
  body: string;
}

export const DOCS_ARTICLES: Record<string, DocArticle> = {
  'intro': {
    id: 'intro',
    title: 'Meridian Terminal Documentation',
    category: 'Getting Started',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'High-performance Linux terminal emulator, developer environment, and local AI agent platform built with C++20.',
    headings: [
      { id: 'overview', text: 'Overview', level: 2 },
      { id: 'screenshot', text: 'Live Environment Showcase', level: 2 },
      { id: 'key-pillars', text: 'Core Architecture Pillars', level: 2 },
      { id: 'quick-commands', text: 'Quick Installation Overview', level: 2 },
    ],
    body: `
<p class="lead-text">
  <strong>Meridian Terminal</strong> is an advanced, high-performance Linux terminal emulator, developer environment, and local AI agent platform engineered in modern <strong>C++20</strong>. It features native hardware-accelerated raster graphics, direct inline image rendering, live Git intelligence, an interactive anime theme switcher, multi-pane multiplexing, and built-in offline developer tooling.
</p>

<div class="note-box note-info">
  <div class="note-title">🤝 Open Source Community & Contributors Welcome</div>
  <p>Contributions of all kinds are warmly welcomed! Whether you want to fix bugs, resolve visual glitches, contribute new anime artwork themes, implement new developer tooling, or improve POSIX shell compatibility, pull requests and issues are highly encouraged.</p>
</div>

<h2 id="screenshot">Live Environment Showcase</h2>
<p>Below is an authentic screenshot of Meridian Terminal running on Fedora Linux with Hyprland, displaying the direct anime artwork header, system specs, Powerline prompt, and Fastfetch hardware intelligence:</p>

<div class="screenshot-wrapper">
  <img src="./assets/meridian_screenshot.png" alt="Meridian Terminal 2.0 on Fedora Hyprland" class="doc-screenshot" />
  <div class="screenshot-caption">Figure 1: Meridian Terminal 2.0 with anime artwork header (theme 2 / ribbon_girl), live Fastfetch specs, and Powerline badges.</div>
</div>

<h2 id="key-pillars">Core Architecture Pillars</h2>
<div class="feature-grid">
  <div class="feature-card">
    <div class="feature-icon">🖼️</div>
    <div class="feature-name">Direct Raster Graphics</div>
    <div class="feature-desc">Zero-ASCII, zero half-block decoding. Direct 32-bit RGBA decoding into GPU textures and chunked Kitty Graphics protocol transmission.</div>
  </div>
  <div class="feature-card">
    <div class="feature-icon">⚡</div>
    <div class="feature-name">High-Performance PTY</div>
    <div class="feature-desc">Asynchronous Linux <code>openpty</code> pseudoterminal multiplexer with non-blocking I/O and sub-millisecond character throughput.</div>
  </div>
  <div class="feature-card">
    <div class="feature-icon">🤖</div>
    <div class="feature-name">Inline AI Assistance</div>
    <div class="feature-desc">Natural language shell translator (<code>ask</code>), automated compiler/database error diagnosis (<code>diag</code>), and autonomous agent loop.</div>
  </div>
  <div class="feature-card">
    <div class="feature-icon">🛡️</div>
    <div class="feature-name">Safety & Redaction Defense</div>
    <div class="feature-desc">Real-time risk classifier stopping destructive operations and automatic secret redactor masking tokens and passwords.</div>
  </div>
</div>

<h2 id="quick-commands">Quick Installation Overview</h2>
<p>Install Meridian immediately with a single command on any Linux distribution:</p>

<pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre>
`
  },

  'installation': {
    id: 'installation',
    title: 'Installation Guide',
    category: 'Getting Started',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Universal installation methods for Fedora, Ubuntu, Debian, Arch Linux, macOS, and offline air-gapped systems.',
    headings: [
      { id: 'fedora-dnf', text: 'Fedora & RHEL (sudo dnf install)', level: 2 },
      { id: 'universal-curl', text: 'Universal 1-Line Installer', level: 2 },
      { id: 'ubuntu-debian', text: 'Ubuntu & Debian (.deb)', level: 2 },
      { id: 'arch-linux', text: 'Arch Linux & Manjaro', level: 2 },
      { id: 'source-build', text: 'Compiling from Source', level: 2 },
      { id: 'offline-tarball', text: 'Air-Gapped Offline Installation', level: 2 },
    ],
    body: `
<p>Meridian Terminal can be installed across all major Linux distributions via native package managers, direct 1-line installers, prebuilt binary tarballs, or from source.</p>

<h2 id="fedora-dnf">Fedora & RHEL (sudo dnf install)</h2>
<p>On Fedora 39/40/41/42/44 and Red Hat Enterprise Linux, install directly using native <code>dnf</code>:</p>

<h3>Method A: Direct DNF 1-Liner (Fastest)</h3>
<pre><code class="language-bash">sudo dnf install https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>

<h3>Method B: Enable Meridian DNF Repository</h3>
<p>Setting up the repository enables automatic background updates whenever you run <code>sudo dnf update</code>:</p>
<pre><code class="language-bash"># 1. Enable repository in /etc/yum.repos.d/meridian.repo
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/setup_dnf_repo.sh | sudo bash

# 2. Install Meridian Terminal package
sudo dnf install meridian-terminal</code></pre>

<h2 id="universal-curl">Universal 1-Line Installer</h2>
<p>The universal turnkey script detects your distribution, installs compiler dependencies automatically if needed, builds the binary, registers desktop icons, and configures <code>/etc/shells</code>:</p>

<pre><code class="language-bash"># System-wide installation (/usr/local/bin)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash

# User-only installation (~/.local/bin, no root/sudo needed)
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>

<h2 id="ubuntu-debian">Ubuntu & Debian (.deb)</h2>
<p>Download and install the pre-packaged <code>.deb</code> file:</p>
<pre><code class="language-bash">sudo dpkg -i dist/meridian-terminal_2.0.0_amd64.deb
sudo apt-get install -f # Fix any missing dependencies</code></pre>

<h2 id="arch-linux">Arch Linux & Manjaro</h2>
<p>Build and install via <code>makepkg</code>:</p>
<pre><code class="language-bash">cd packaging/arch
makepkg -si</code></pre>

<h2 id="source-build">Compiling from Source</h2>
<p>Requirements: C++20 compiler (GCC 11+ or Clang 13+), <code>make</code>, <code>libutil</code>.</p>
<pre><code class="language-bash">git clone https://github.com/charanbalaji2005/Meridian-Shell.git
cd Meridian-Shell
make all -j$(nproc)
make test
sudo ./install.sh</code></pre>

<h2 id="offline-tarball">Air-Gapped Offline Installation</h2>
<p>For air-gapped computers or secure lab servers without internet access, generate the self-contained offline archive:</p>
<pre><code class="language-bash">./scripts/package_offline.sh
# Copy dist/meridian-terminal-2.0.0-linux-x86_64.tar.gz to target machine
tar -xzf meridian-terminal-2.0.0-linux-x86_64.tar.gz
cd meridian-terminal-2.0.0-linux-x86_64
sudo ./install.sh</code></pre>
`
  },

  'quickstart': {
    id: 'quickstart',
    title: 'Quick Start & First Run',
    category: 'Getting Started',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Get up and running with Meridian in under 60 seconds.',
    headings: [
      { id: 'launching', text: 'Launching Meridian', level: 2 },
      { id: 'anime-switcher', text: 'Switching Anime Artwork (Ctrl+P / pic)', level: 2 },
      { id: 'command-palette', text: 'Interactive Command Palette (Ctrl+Shift+P)', level: 2 },
      { id: 'powerline-prompt', text: 'High-Contrast Powerline Badges', level: 2 },
    ],
    body: `
<p>Once installed, Meridian can be launched from your terminal emulator, application launcher, or desktop menu.</p>

<h2 id="launching">Launching Meridian</h2>
<pre><code class="language-bash">meridian          # Launch full Meridian terminal environment
meridian-shell    # Launch standalone lightweight shell engine</code></pre>

<h2 id="anime-switcher">Switching Anime Artwork (Ctrl+P / pic)</h2>
<p>Meridian includes 14 high-resolution anime artwork themes. Press <code>Ctrl+P</code> or run <code>pic</code> to open the interactive theme gallery:</p>

<pre><code class="language-bash">pic set sharingan_eye   # Sasuke / Itachi Mangekyō Sharingan (Theme 0)
pic set sakura_girl     # Sakura Blossom Anime Girl (Theme 1)
pic set ribbon_girl     # Monochrome Anime Ribbon Girl (Theme 2)
pic set fan_girl        # Anime Girl with Fan (Theme 3)
pic set gojo_purple     # Gojo Satoru: Hollow Purple (Theme 5)
pic set sukuna_shrine   # Sukuna: Malevolent Shrine (Theme 6)
pic set naruto_rasengan # Naruto: Kurama Rasengan (Theme 7)
pic set random          # Automatically rotate themes on every startup
pic set /path/to/img.png # Set your own custom wallpaper permanently</code></pre>

<h2 id="command-palette">Interactive Command Palette (Ctrl+Shift+P)</h2>
<p>Press <code>Ctrl+Shift+P</code> or type <code>meridian palette</code> to access fuzzy-searchable commands across split panes, AI diagnoses, theme changes, Git branch inspections, and system performance metrics.</p>

<h2 id="powerline-prompt">High-Contrast Powerline Badges</h2>
<p>Meridian renders vivid 24-bit TrueColor Powerline badges with real-time Git branch detection (<code>󰌽 Linux</code>, <code>󰨇 Hyprland</code>, <code> origin  main 6✸ 8●</code>) and sub-millisecond latency.</p>
`
  },

  'lifecycle': {
    id: 'lifecycle',
    title: 'Updates & Uninstallation',
    category: 'Getting Started',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Commands to check versions, update to the latest release, and cleanly remove Meridian.',
    headings: [
      { id: 'version', text: 'Checking Version', level: 2 },
      { id: 'updating', text: 'Updating Meridian', level: 2 },
      { id: 'uninstalling', text: 'Uninstalling Meridian', level: 2 },
    ],
    body: `
<p>Meridian includes built-in commands to check version information, update binaries and assets from GitHub, and completely remove installed files.</p>

<h2 id="version">Checking Version</h2>
<pre><code class="language-bash">meridian --version
# or:
meridian -v</code></pre>
<p><strong>Output:</strong></p>
<pre><code class="language-text">Meridian Terminal 2.0.0 (x86_64-linux)
Copyright (c) 2025-2026 Charan Balaji and Meridian Contributors.</code></pre>

<h2 id="updating">Updating Meridian</h2>
<p>To upgrade to the latest GitHub release:</p>
<pre><code class="language-bash"># Built-in update command (auto-detects root vs user mode)
meridian update

# Or via direct 1-line curl:
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | sudo bash</code></pre>

<h2 id="uninstalling">Uninstalling Meridian</h2>
<p>To cleanly remove all Meridian binaries, desktop launchers, and shared data:</p>
<pre><code class="language-bash"># Built-in uninstaller
sudo meridian uninstall

# To remove all configuration and saved themes (~/.config/meridian) as well:
sudo meridian uninstall --purge

# Or via 1-line curl:
curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/uninstall.sh | sudo bash</code></pre>
`
  },

  'terminal': {
    id: 'terminal',
    title: 'Terminal & Shell Engine',
    category: 'Terminal & Shell',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'VT100/XTerm emulation, 24-bit TrueColor, POSIX AST execution engine, pipes, redirects, and job control.',
    headings: [
      { id: 'emulation', text: 'VT / XTerm Terminal Emulation', level: 2 },
      { id: 'shell-engine', text: 'POSIX Shell Engine & Parser', level: 2 },
      { id: 'job-control', text: 'Job Control & Background Tasks', level: 2 },
    ],
    body: `
<p>Meridian combines a standard-compliant VT100/XTerm terminal emulation core with an independent POSIX-compliant AST execution engine.</p>

<h2 id="emulation">VT / XTerm Terminal Emulation</h2>
<ul>
  <li>Full support for ANSI color escapes (16 standard, 256 color, and 24-bit TrueColor RGB).</li>
  <li>Alternate Screen Buffer for curses applications (<code>vim</code>, <code>htop</code>, <code>less</code>).</li>
  <li>UTF-8 multibyte character encoding with complete Nerd Font glyph support.</li>
</ul>

<h2 id="shell-engine">POSIX Shell Engine & Parser</h2>
<p>Meridian includes an internal lexer and recursive-descent parser capable of parsing complex shell pipelines:</p>
<ul>
  <li>Pipes (<code>|</code>) and chaining (<code>cmd1 | cmd2 | cmd3</code>)</li>
  <li>I/O Redirections (<code>&gt;</code>, <code>&gt;&gt;</code>, <code>&lt;</code>, <code>2&gt;&amp;1</code>)</li>
  <li>Logical operators (<code>&amp;&amp;</code>, <code>||</code>, <code>;</code>)</li>
  <li>Environment variable expansions (<code>$VAR</code>, <code>\${VAR}</code>, <code>$?</code>)</li>
</ul>

<h2 id="job-control">Job Control & Background Tasks</h2>
<pre><code class="language-bash">Ctrl+Z        # Suspend foreground job
bg            # Resume suspended job in background
fg            # Bring background job to foreground
jobs          # List all running child processes</code></pre>
`
  },

  'keybindings': {
    id: 'keybindings',
    title: 'Keybindings & Shortcuts',
    category: 'Terminal & Shell',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Keyboard shortcuts for command palette, theme switcher, splits, and session navigation.',
    headings: [
      { id: 'default-shortcuts', text: 'Default Keybindings', level: 2 },
      { id: 'custom-keys', text: 'Custom Keybinding Configuration', level: 2 },
    ],
    body: `
<p>Meridian provides fast keyboard shortcuts for instant navigation, splits, command searching, and theme switching.</p>

<h2 id="default-shortcuts">Default Keybindings</h2>
<table class="doc-table">
  <thead>
    <tr><th>Shortcut</th><th>Action</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr><td><code>Ctrl+P</code></td><td>Artwork Gallery</td><td>Open interactive anime theme gallery switcher</td></tr>
    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy-search action palette</td></tr>
    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active terminal window vertically</td></tr>
    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active terminal window horizontally</td></tr>
    <tr><td><code>Ctrl+Shift+Z</code></td><td>Zoom Toggle</td><td>Maximize/zoom active pane to full window</td></tr>
    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search across scrollback buffer & history</td></tr>
    <tr><td><code>Ctrl+L</code></td><td>Clear Screen</td><td>Clear terminal screen and redraw prompt</td></tr>
  </tbody>
</table>

<h2 id="custom-keys">Custom Keybinding Configuration</h2>
<p>Custom shortcuts can be defined in <code>~/.config/meridian/keybindings.json</code>.</p>
`
  },

  'raster-images': {
    id: 'raster-images',
    title: 'Direct Raster Image Pipeline',
    category: 'Graphics & Artwork',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Hardware-accelerated 32-bit RGBA inline image decoding and rendering bypassing ASCII converters.',
    headings: [
      { id: 'pipeline', text: 'Rendering Pipeline Architecture', level: 2 },
      { id: 'pic-command', text: 'The pic Command', level: 2 },
      { id: 'debug-specs', text: 'Inspecting Image Specs (pic --debug)', level: 2 },
    ],
    body: `
<p>Unlike traditional terminals that downsample images into character approximations, Meridian features a <strong>direct full-color raster graphics engine</strong>.</p>

<h2 id="pipeline">Rendering Pipeline Architecture</h2>
<div class="code-flow">
  <div class="flow-step">1. <code>pic file.png</code></div>
  <div class="flow-arrow">↓</div>
  <div class="flow-step">2. <code>graphics::ImageDecoder</code> (stb_image: PNG, JPEG, WebP, BMP, GIF)</div>
  <div class="flow-arrow">↓</div>
  <div class="flow-step">3. 32-bit RGBA8888 Pixel Buffer</div>
  <div class="flow-arrow">↓</div>
  <div class="flow-step">4. GPU Hardware Texture Transmission (Kitty / Sixel / RGBA Direct)</div>
  <div class="flow-arrow">↓</div>
  <div class="flow-step">5. <strong>ACTUAL FULL-COLOR IMAGE PIXELS ON SCREEN</strong></div>
</div>

<h2 id="pic-command">The pic Command</h2>
<pre><code class="language-bash">pic tanjiro.png       # Render image at x=30, y=30 with aspect containment (max 220x220)
pic --clear           # Clear active raster graphic from canvas</code></pre>

<h2 id="debug-specs">Inspecting Image Specs (pic --debug)</h2>
<pre><code class="language-bash">meridian pic --debug resources/images/gallery/sharingan_eye.png</code></pre>
<p><strong>Output:</strong></p>
<pre><code class="language-text">IMAGE MODE: RASTER
SOURCE: resources/images/gallery/sharingan_eye.png
SOURCE SIZE: 268x148
FORMAT: PNG
PIXEL FORMAT: RGBA8888
RENDERER: GPU
TEXTURE: 268x148
DISPLAY: 220x121</code></pre>
`
  },

  'anime-gallery': {
    id: 'anime-gallery',
    title: 'Anime Gallery & Themes',
    category: 'Graphics & Artwork',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: '14 built-in anime picture themes and custom wallpaper configuration.',
    headings: [
      { id: 'theme-list', text: 'Complete Theme Index', level: 2 },
      { id: 'cli-usage', text: 'CLI & Keyboard Controls', level: 2 },
    ],
    body: `
<p>Meridian includes an embedded high-resolution anime art gallery installed to <code>~/.config/meridian/gallery/</code>.</p>

<h2 id="theme-list">Complete Theme Index</h2>
<table class="doc-table">
  <thead>
    <tr><th>Index</th><th>Theme ID</th><th>Character / Source</th><th>Type</th></tr>
  </thead>
  <tbody>
    <tr><td><code>0</code></td><td><code>sharingan_eye</code></td><td>Sasuke / Itachi Mangekyō Sharingan</td><td>Raw Raster Image</td></tr>
    <tr><td><code>1</code></td><td><code>sakura_girl</code></td><td>Sakura Blossom Anime Girl</td><td>Raw Raster Image</td></tr>
    <tr><td><code>2</code></td><td><code>ribbon_girl</code></td><td>Monochrome Anime Ribbon Girl</td><td>Raw Raster Image</td></tr>
    <tr><td><code>3</code></td><td><code>fan_girl</code></td><td>Anime Girl with Fan</td><td>Raw Raster Image</td></tr>
    <tr><td><code>4</code></td><td><code>itachi_sharingan</code></td><td>Itachi Uchiha (Naruto)</td><td>TrueColor Art</td></tr>
    <tr><td><code>5</code></td><td><code>gojo_purple</code></td><td>Gojo Satoru: Hollow Purple (JJK)</td><td>TrueColor Art</td></tr>
    <tr><td><code>6</code></td><td><code>sukuna_shrine</code></td><td>Ryomen Sukuna: Malevolent Shrine (JJK)</td><td>TrueColor Art</td></tr>
    <tr><td><code>7</code></td><td><code>naruto_rasengan</code></td><td>Naruto Uzumaki: Kurama Rasengan</td><td>TrueColor Art</td></tr>
    <tr><td><code>8</code></td><td><code>rengoku_flames</code></td><td>Kyojuro Rengoku: Flame Hashira</td><td>TrueColor Art</td></tr>
    <tr><td><code>9</code></td><td><code>ultra_instinct</code></td><td>Goku: Mastered Ultra Instinct (DBS)</td><td>TrueColor Art</td></tr>
    <tr><td><code>10</code></td><td><code>chainsaw_man</code></td><td>Denji & Power (Chainsaw Man)</td><td>TrueColor Art</td></tr>
    <tr><td><code>11</code></td><td><code>cyberpunk</code></td><td>Lucy (Cyberpunk: Edgerunners)</td><td>TrueColor Art</td></tr>
    <tr><td><code>12</code></td><td><code>synthwave</code></td><td>Retro Synthwave Cyber Sun</td><td>TrueColor Art</td></tr>
    <tr><td><code>13</code></td><td><code>ghibli</code></td><td>Studio Ghibli Anime Meadow</td><td>TrueColor Art</td></tr>
  </tbody>
</table>

<h2 id="cli-usage">CLI & Keyboard Controls</h2>
<pre><code class="language-bash">pic set 2            # Set Theme 2 (ribbon_girl) permanently
pic set random       # Rotate through all themes on every terminal launch
pic show sakura_girl # Preview Sakura Girl without saving
pic set ~/art.png    # Use your own custom image file</code></pre>
`
  },

  'ai-assistant': {
    id: 'ai-assistant',
    title: 'Intent & Error Diagnostics',
    category: 'AI & Intelligence',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Natural language intent translation (ask) and instant compiler/database diagnostics (diag).',
    headings: [
      { id: 'intent-ask', text: 'Natural Language Intent Translation (ask)', level: 2 },
      { id: 'diagnostics-diag', text: 'Automated Error Diagnostics (diag)', level: 2 },
    ],
    body: `
<p>Meridian includes an offline, privacy-first AI intelligence engine built directly into the binary.</p>

<h2 id="intent-ask">Natural Language Intent Translation (ask)</h2>
<p>Translates human language into verified, executable shell pipelines:</p>
<pre><code class="language-bash">meridian ask "find all pdf files modified in the last 7 days larger than 10MB"
meridian ask "compress the src folder into a tar.gz archive"
meridian ask "find which process is using port 8080"</code></pre>

<h2 id="diagnostics-diag">Automated Error Diagnostics (diag)</h2>
<p>Parses stack traces from GCC, Clang, Rust, Python, and PostgreSQL to deliver instant fixes:</p>
<pre><code class="language-bash"># Diagnose last failed command automatically:
meridian diag

# Or diagnose specific error output:
meridian diag "g++: error: cannot find -lutil"</code></pre>
`
  },

  'ai-safety': {
    id: 'ai-safety',
    title: 'Safety Defense & Redaction',
    category: 'AI & Intelligence',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Real-time destructive command blocking and automatic credential leakage redactor.',
    headings: [
      { id: 'risk-tiers', text: 'Destructive Command Interceptor', level: 2 },
      { id: 'secret-masking', text: 'Credential & Token Masking', level: 2 },
    ],
    body: `
<p>Meridian intercepts high-risk commands and masks sensitive secrets before they enter history files or terminal screen buffers.</p>

<h2 id="risk-tiers">Destructive Command Interceptor</h2>
<ul>
  <li>🔴 <strong>Critical (Blocked)</strong>: <code>rm -rf /</code>, <code>:(){ :|:& };:</code> (fork bombs), <code>dd of=/dev/sda</code>, untrusted curl pipes.</li>
  <li>🟡 <strong>High (Confirmation)</strong>: <code>sudo rm -rf</code>, <code>git push --force</code>, partition modifications.</li>
  <li>🟢 <strong>Low / Safe</strong>: Standard read, build, and navigation operations.</li>
</ul>

<h2 id="secret-masking">Credential & Token Masking</h2>
<p>Automatically detects and redacts AWS Keys (<code>AKIA...</code>), Bearer tokens, private RSA keys, and database passwords from screen output.</p>
`
  },

  'architecture': {
    id: 'architecture',
    title: 'PTY & Core Architecture',
    category: 'Developer & Core',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Linux openpty multiplexer, ScreenBuffer cell matrix, damage tracking, and glyph atlas.',
    headings: [
      { id: 'pty-multiplexer', text: 'Linux PTY Pseudoterminal Engine', level: 2 },
      { id: 'screen-buffer', text: 'ScreenBuffer & Damage Tracking', level: 2 },
      { id: 'glyph-atlas', text: 'Glyph Atlas & Font Rasterizer', level: 2 },
    ],
    body: `
<p>Meridian is engineered as a clean modular C++20 codebase structured into independent core layers.</p>

<h2 id="pty-multiplexer">Linux PTY Pseudoterminal Engine</h2>
<p>Uses POSIX <code>openpty</code> to fork master/slave pairs, handling non-blocking asynchronous I/O and window resize signaling (<code>SIGWINCH</code>).</p>

<h2 id="screen-buffer">ScreenBuffer & Damage Tracking</h2>
<p>Maintains a 2D matrix of cell attributes (character code, foreground/background RGB, bold, italic, underline) and tracks dirty row bounding boxes to minimize frame blitting overhead.</p>

<h2 id="glyph-atlas">Glyph Atlas & Font Rasterizer</h2>
<p>Renders high-density TrueColor characters and Nerd Font symbols with sub-pixel alignment and caching.</p>
`
  },

  'dev-tools': {
    id: 'dev-tools',
    title: 'Developer Tooling & Workspaces',
    category: 'Developer & Core',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Built-in real-time system monitor, Git intelligence, tree file explorer, and workspace multiplexing.',
    headings: [
      { id: 'monitor', text: 'System Resource Monitor (monitor)', level: 2 },
      { id: 'git-intel', text: 'Git Intelligence (git)', level: 2 },
      { id: 'file-explorer', text: 'Tree File Explorer (files)', level: 2 },
      { id: 'workspace-multiplexer', text: 'Persistent Workspace Multiplexer', level: 2 },
    ],
    body: `
<p>Meridian includes an integrated suite of developer tools that can be invoked via CLI or from the Command Palette (<code>Ctrl+Shift+P</code>).</p>

<h2 id="monitor">System Resource Monitor (monitor)</h2>
<pre><code class="language-bash">meridian monitor      # Display real-time CPU, RAM, Disk, Network, and Process tree</code></pre>

<h2 id="git-intel">Git Intelligence (git)</h2>
<pre><code class="language-bash">meridian git          # Inspect branch divergence, ahead/behind status, staged/unstaged changes</code></pre>

<h2 id="file-explorer">Tree File Explorer (files)</h2>
<pre><code class="language-bash">meridian files [dir]  # Interactive directory tree with Git badges and MIME icons</code></pre>

<h2 id="workspace-multiplexer">Persistent Workspace Multiplexer</h2>
<pre><code class="language-bash">meridian workspace save backend-dev   # Save current window splits & layout
meridian workspace open backend-dev   # Restore saved development environment
meridian workspace list               # List all saved workspaces</code></pre>
`
  },

  'contributing': {
    id: 'contributing',
    title: 'Contributing Guidelines',
    category: 'Community & Status',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'How to report bugs, resolve visual glitches, add anime themes, and contribute to Meridian.',
    headings: [
      { id: 'welcome', text: 'Welcome Contributors!', level: 2 },
      { id: 'how-to-help', text: 'Ways to Contribute', level: 2 },
      { id: 'pull-requests', text: 'Pull Request Workflow', level: 2 },
      { id: 'testing-code', text: 'Testing Your Changes', level: 2 },
    ],
    body: `
<div class="note-box note-success">
  <div class="note-title">🌟 Open Source Community Note</div>
  <p><strong>Contributors of all experience levels are warmly welcomed!</strong> We appreciate all forms of contributions—from reporting typos and fixing visual glitches on different terminal emulators to adding anime artwork themes, writing unit tests, and implementing new shell features.</p>
</div>

<h2 id="how-to-help">Ways to Contribute</h2>
<ul>
  <li>🐛 <strong>Bug Fixes & Glitches</strong>: Report or fix rendering anomalies, font clipping, terminal resizing issues, or PTY edge cases.</li>
  <li>🎨 <strong>Anime Artwork & Themes</strong>: Add new raw anime wallpapers to <code>resources/images/gallery/</code> and register them in <code>ArtGallery</code>.</li>
  <li>⚡ <strong>Performance Improvements</strong>: Optimize glyph rendering, damage blitting, or memory footprints.</li>
  <li>📖 <strong>Documentation</strong>: Improve documentation pages, tutorials, or packaging recipes.</li>
</ul>

<h2 id="pull-requests">Pull Request Workflow</h2>
<ol>
  <li>Fork the repository: <code>https://github.com/charanbalaji2005/Meridian-Shell</code></li>
  <li>Create a feature branch: <code>git checkout -b feat/my-new-feature</code></li>
  <li>Make your changes and verify with <code>make test</code></li>
  <li>Submit a Pull Request with a clear description of your changes.</li>
</ol>

<h2 id="testing-code">Testing Your Changes</h2>
<p>Meridian includes an integrated test suite with 126 unit and integration tests:</p>
<pre><code class="language-bash">make all -j$(nproc)
make test
# Verify that all 126 tests and 400+ assertions pass cleanly!</code></pre>
`
  },

  'status-matrix': {
    id: 'status-matrix',
    title: 'Component Status Matrix',
    category: 'Community & Status',
    status: 'implemented',
    lastUpdated: 'August 25, 2026',
    summary: 'Transparent and credible status breakdown of all Meridian subsystems.',
    headings: [
      { id: 'matrix', text: 'Subsystem Status Overview', level: 2 },
      { id: 'legend', text: 'Status Legend', level: 2 },
    ],
    body: `
<p>Meridian maintains a transparent and honest status report of every component in the codebase.</p>

<h2 id="legend">Status Legend</h2>
<ul>
  <li><span class="status-badge status-implemented">🟢 Implemented</span>: Production-ready, fully tested, and passing all regression suites.</li>
  <li><span class="status-badge status-development">🟡 In Development</span>: Core engine functional; active refinement and cross-platform porting underway.</li>
  <li><span class="status-badge status-experimental">🧪 Experimental</span>: Functional preview with evolving APIs and heuristics.</li>
</ul>

<h2 id="matrix">Subsystem Status Overview</h2>
<table class="doc-table">
  <thead>
    <tr><th>Component</th><th>Status</th><th>Description</th></tr>
  </thead>
  <tbody>
    <tr><td><strong>Linux PTY Core</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>POSIX <code>openpty</code>, asynchronous non-blocking session multiplexer</td></tr>
    <tr><td><strong>POSIX Shell & AST Executor</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Lexer, parser, pipes, redirections, job control (<code>Ctrl+Z</code>, <code>bg</code>, <code>fg</code>)</td></tr>
    <tr><td><strong>Direct Raster Graphics Engine</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Native stb_image 32-bit RGBA decoding, aspect containment, zero ASCII</td></tr>
    <tr><td><strong>Kitty Graphics Protocol</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Chunked 2048-byte safe frame transmission (<code>m=1</code>/<code>m=0</code>)</td></tr>
    <tr><td><strong>Anime Art Gallery</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>14 built-in themes with interactive <code>Ctrl+P</code> / <code>pic set</code> selector</td></tr>
    <tr><td><strong>Natural Language Intent (ask)</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Local prompt-to-command translation with parameter extraction</td></tr>
    <tr><td><strong>Error Diagnostics (diag)</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Automated compiler/runtime/database error diagnostics and fixes</td></tr>
    <tr><td><strong>Safety & Secret Redaction</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Risk classification (Low/Medium/High/Critical) and credential masking</td></tr>
    <tr><td><strong>System Monitor (monitor)</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Live CPU, RAM, Disk, Network, Process tree inspection</td></tr>
    <tr><td><strong>Git Intelligence (git)</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Real-time branch divergence, staged/unstaged changes, Powerline badges</td></tr>
    <tr><td><strong>File Explorer (files)</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Interactive tree view with git status and MIME glyphs</td></tr>
    <tr><td><strong>Command Palette</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Fuzzy-searchable action center (<code>Ctrl+Shift+P</code>)</td></tr>
    <tr><td><strong>Workspace Multiplexer</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Multi-pane splits (horizontal/vertical), zoom tabs, session state save/load</td></tr>
    <tr><td><strong>Fedora RPM & DNF Repo</strong></td><td><span class="status-badge status-implemented">🟢 Implemented</span></td><td>Native <code>sudo dnf install</code> support and <code>setup_dnf_repo.sh</code></td></tr>
    <tr><td><strong>Autonomous AI Agent (agent)</strong></td><td><span class="status-badge status-experimental">🧪 Experimental</span></td><td>Multi-step goal execution with self-correcting retry loop</td></tr>
    <tr><td><strong>macOS BSD PTY Core</strong></td><td><span class="status-badge status-development">🟡 In Development</span></td><td>macOS Darwin BSD PTY support</td></tr>
    <tr><td><strong>Windows ConPTY Core</strong></td><td><span class="status-badge status-development">🟡 In Development</span></td><td>Windows 10/11 ConPTY pseudoterminal adapter</td></tr>
  </tbody>
</table>
`
  }
};
