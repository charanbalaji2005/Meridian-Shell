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
  "intro": {
    "id": "intro",
    "title": "Meridian Shell",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "quick-start",
        "text": "Quick Start",
        "level": 2
      },
      {
        "id": "what-is-meridian",
        "text": "What is Meridian?",
        "level": 2
      },
      {
        "id": "core-features",
        "text": "Core Features",
        "level": 2
      },
      {
        "id": "architecture",
        "text": "Architecture",
        "level": 2
      },
      {
        "id": "direct-image-rendering",
        "text": "Direct Image Rendering",
        "level": 2
      },
      {
        "id": "terminal-compatibility",
        "text": "Terminal Compatibility",
        "level": 2
      },
      {
        "id": "packaging",
        "text": "Packaging",
        "level": 2
      },
      {
        "id": "project-status",
        "text": "Project Status",
        "level": 2
      }
    ],
    "summary": "A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.",
    "body": "\n<div class=\"hero-section\">\n  <p class=\"hero-lead\">\n    A developer-focused cross-platform terminal platform built around real PTY sessions, terminal emulation, native graphics, GPU rendering, and extensible developer tooling.\n  </p>\n  <div class=\"hero-actions\">\n    <a href=\"#installation\" class=\"btn-primary\">Get Started</a>\n    <a href=\"#development-building\" class=\"btn-secondary\">Build from Source</a>\n    <a href=\"https://github.com/charanbalaji2005/Meridian-Shell\" target=\"_blank\" rel=\"noreferrer\" class=\"btn-outline\">\n      <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M15 22v-4a4.8 4.8 0 0 0-1-3.5c3 0 6-2 6-5.5.08-1.25-.27-2.48-1-3.5.28-1.15.28-2.35 0-3.5 0 0-1 0-3 1.5-2.64-.5-5.36-.5-8 0C6 2 5 2 5 2c-.3 1.15-.3 2.35 0 3.5A5.403 5.403 0 0 0 4 9c0 3.5 3 5.5 6 5.5-.39.49-.68 1.05-.85 1.65-.17.6-.22 1.23-.15 1.85v4\"/><path d=\"M9 18c-4.51 2-5-2-7-2\"/></svg>\n      GitHub\n    </a>\n  </div>\n</div>\n\n<div class=\"screenshot-container\">\n  <img src=\"./assets/meridian_screenshot.png\" alt=\"Meridian Terminal live session running on Fedora Linux\" class=\"meridian-live-screenshot\" />\n  <div class=\"screenshot-label\">\n    <span>Meridian Shell 2.0 on Fedora Linux (x86_64) running Hyprland with anime artwork header (Theme 2 / Ribbon Girl) and live Fastfetch hardware metrics.</span>\n  </div>\n</div>\n\n<h2 id=\"quick-start\">Quick Start</h2>\n<p>You can run or install Meridian Shell on any Linux system through local package archives or the universal turnkey installer.</p>\n\n<div class=\"note-box note-info\">\n  <div class=\"note-title\">📌 Important Notice on Package Repositories</div>\n  <p>Commands like <code>sudo dnf install meridian-terminal</code> require the Meridian repository to be enabled first. For immediate installation on a fresh machine without adding a repository, use the local package files or the universal curl installer below.</p>\n</div>\n\n<h3>1. Local Package Installation</h3>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Fedora / RHEL (Local RPM)</span></div><pre><code class=\"language-bash\">sudo dnf install ./meridian-terminal.rpm</code></pre></div>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Ubuntu / Debian (Local DEB)</span></div><pre><code class=\"language-bash\">sudo apt install ./meridian-terminal.deb</code></pre></div>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Arch Linux (Local Package)</span></div><pre><code class=\"language-bash\">sudo pacman -U ./meridian-terminal.pkg.tar.zst</code></pre></div>\n\n<h3>2. Universal Turnkey Installer</h3>\n<div class=\"code-block-wrapper\"><div class=\"code-header\"><span>Bash 1-Liner</span></div><pre><code class=\"language-bash\">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre></div>\n\n<h2 id=\"what-is-meridian\">What is Meridian?</h2>\n<p><strong>Meridian Shell</strong> is an open-source terminal emulator and developer platform designed to unify the terminal layer with native graphics, extensible developer intelligence, and real pseudoterminal multiplexing.</p>\n\n<h2 id=\"core-features\">Core Features</h2>\n<div class=\"feature-cards-grid\">\n  <div class=\"feat-card\"><div class=\"feat-icon\">⚡</div><div class=\"feat-title\">Real PTY Sessions</div><div class=\"feat-body\">Asynchronous POSIX openpty multiplexer with non-blocking I/O.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🖥️</div><div class=\"feat-title\">VT / ANSI Emulation</div><div class=\"feat-body\">Full escape sequence parsing, alternate screen buffers, 24-bit TrueColor.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🖼️</div><div class=\"feat-title\">Direct Raster Graphics</div><div class=\"feat-body\">Direct 32-bit RGBA inline image decoding with zero ASCII downsampling.</div></div>\n  <div class=\"feat-card\"><div class=\"feat-icon\">🪟</div><div class=\"feat-title\">Tabs & Panes</div><div class=\"feat-body\">Multi-pane splits, pane zooming, and persistent workspace layouts.</div></div>\n</div>\n\n<h2 id=\"architecture\">Architecture</h2>\n<div class=\"arch-diagram-block\"><pre><code class=\"language-text\">Meridian Application\n   ↓\nTerminal UI (Qt / Canvas)\n   ↓\nTerminal Core (PTY, VT Parser, ScreenBuffer, Input, Shell Engine)\n   ↓\nGraphics / Renderer (Text Renderer, GPU Renderer, Image Renderer)\n   ↓\nPlatform Layer (Linux, macOS, Windows)</code></pre></div>\n\n<h2 id=\"direct-image-rendering\">Direct Image Rendering</h2>\n<p>Executing <code>pic image.png</code> produces a <strong>direct full-color raster image</strong> on the terminal canvas.</p>\n<div class=\"flow-diagram\">\n  <div class=\"flow-box\">PNG / JPEG / WebP / BMP</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">graphics::ImageDecoder (stb_image)</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">32-bit RGBA Pixels</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box\">GPU Texture</div>\n  <div class=\"flow-sep\">→</div>\n  <div class=\"flow-box highlight-box\">ACTUAL IMAGE PIXELS ON SCREEN</div>\n</div>\n\n<h2 id=\"terminal-compatibility\">Terminal Compatibility</h2>\n<p>Meridian is tested against standard Linux CLI applications: <code>bash</code>, <code>zsh</code>, <code>fish</code>, <code>ssh</code>, <code>sudo</code>, <code>vim</code>, <code>nano</code>, <code>tmux</code>, <code>htop</code>, <code>git</code>, <code>python</code>, <code>node</code>, <code>docker</code>.</p>\n\n<h2 id=\"packaging\">Packaging</h2>\n<p>Available for Fedora (RPM), Debian/Ubuntu (.deb), Arch Linux (PKGBUILD), and Universal Linux tarballs.</p>\n\n<h2 id=\"project-status\">Project Status</h2>\n<p>PTY Core: <span class=\"status-tag status-impl\">IMPLEMENTED</span> | VT Engine: <span class=\"status-tag status-impl\">IMPLEMENTED</span> | Direct Images: <span class=\"status-tag status-impl\">IMPLEMENTED</span> | GPU Renderer: <span class=\"status-tag status-dev\">DEVELOPMENT</span></p>\n"
  },
  "installation": {
    "id": "installation",
    "title": "Installation & Build",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "dnf-rpm",
        "text": "Fedora & RHEL (DNF / RPM)",
        "level": 2
      },
      {
        "id": "apt-deb",
        "text": "Debian & Ubuntu (APT / DEB)",
        "level": 2
      },
      {
        "id": "arch-pkg",
        "text": "Arch Linux (Pacman / PKGBUILD)",
        "level": 2
      },
      {
        "id": "universal-script",
        "text": "Universal 1-Line Installer",
        "level": 2
      }
    ],
    "summary": "Comprehensive installation options across Linux distributions, macOS, and Windows.",
    "body": "\n<p>Meridian provides native packages for all major Linux distributions.</p>\n<h2 id=\"dnf-rpm\">Fedora & RHEL (DNF / RPM)</h2>\n<pre><code class=\"language-bash\">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>\n<h2 id=\"apt-deb\">Debian & Ubuntu (APT / DEB)</h2>\n<pre><code class=\"language-bash\">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>\n<h2 id=\"arch-pkg\">Arch Linux (Pacman / PKGBUILD)</h2>\n<pre><code class=\"language-bash\">cd packaging/arch && makepkg -si</code></pre>\n<h2 id=\"universal-script\">Universal 1-Line Installer</h2>\n<pre><code class=\"language-bash\">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>\n"
  },
  "quickstart": {
    "id": "quickstart",
    "title": "Quick Start",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "launch",
        "text": "Launching Meridian",
        "level": 2
      },
      {
        "id": "themes",
        "text": "Anime Theme Selection (Ctrl+P / pic)",
        "level": 2
      },
      {
        "id": "palette",
        "text": "Command Palette (Ctrl+Shift+P)",
        "level": 2
      }
    ],
    "summary": "Get started with Meridian Terminal in under one minute.",
    "body": "\n<h2 id=\"launch\">Launching Meridian</h2>\n<pre><code class=\"language-bash\">meridian          # Start interactive terminal environment\nmeridian-shell    # Start standalone shell engine</code></pre>\n<h2 id=\"themes\">Anime Theme Selection (Ctrl+P / pic)</h2>\n<pre><code class=\"language-bash\">pic set sharingan_eye   # Sasuke/Itachi Sharingan (Theme 0)\npic set ribbon_girl     # Ribbon Girl (Theme 2)\npic set random          # Rotate themes on startup</code></pre>\n<h2 id=\"palette\">Command Palette (Ctrl+Shift+P)</h2>\n<p>Press <code>Ctrl+Shift+P</code> to fuzzy-search across window splits, git inspections, error diagnoses, and themes.</p>\n"
  },
  "first-run": {
    "id": "first-run",
    "title": "First Run",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "paths",
        "text": "Default Directories & Paths",
        "level": 2
      },
      {
        "id": "env-setup",
        "text": "Environment Verification",
        "level": 2
      }
    ],
    "summary": "Initial configuration directories, assets, and PATH setup.",
    "body": "\n<p>Upon initial launch, Meridian automatically initializes configuration files:</p>\n<h2 id=\"paths\">Default Directories & Paths</h2>\n<ul>\n  <li><code>~/.config/meridian/config.json</code>: Main terminal configuration.</li>\n  <li><code>~/.config/meridian/gallery/</code>: Embedded anime wallpaper assets.</li>\n  <li><code>~/.config/meridian/keybindings.json</code>: User custom keymap overrides.</li>\n  <li><code>~/.local/share/meridian/history.db</code>: Rich history database.</li>\n</ul>\n<h2 id=\"env-setup\">Environment Verification</h2>\n<pre><code class=\"language-bash\">meridian --version</code></pre>\n"
  },
  "getting-started-config": {
    "id": "getting-started-config",
    "title": "Configuration",
    "category": "GETTING STARTED",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "overview",
        "text": "Configuration Overview",
        "level": 2
      },
      {
        "id": "sample",
        "text": "Sample config.json",
        "level": 2
      }
    ],
    "summary": "Overview of configuration options, live reload, and default values.",
    "body": "\n<p>Meridian uses standard JSON for configuration with automatic live reloading.</p>\n<h2 id=\"sample\">Sample config.json</h2>\n<pre><code class=\"language-json\">{\n  \"theme\": \"sharingan_eye\",\n  \"font_family\": \"FantasqueSansMNFM\",\n  \"font_size\": 14,\n  \"cursor_shape\": \"block\",\n  \"scrollback_lines\": 10000,\n  \"ai_enabled\": true\n}</code></pre>\n"
  },
  "terminal-emulation": {
    "id": "terminal-emulation",
    "title": "Terminal Emulation",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ansi-vt",
        "text": "ANSI & VT Sequences",
        "level": 2
      },
      {
        "id": "alt-screen",
        "text": "Alternate Screen Buffer",
        "level": 2
      },
      {
        "id": "truecolor",
        "text": "24-Bit TrueColor RGB",
        "level": 2
      }
    ],
    "summary": "VT100, XTerm, ANSI escapes, alternate screen buffers, and 24-bit TrueColor.",
    "body": "\n<p>Standard-compliant VT100/XTerm terminal emulation core.</p>\n<h2 id=\"ansi-vt\">ANSI & VT Sequences</h2>\n<p>Full support for cursor motion, scrolling regions, erase modes, and window titles.</p>\n<h2 id=\"alt-screen\">Alternate Screen Buffer</h2>\n<p>Full support for full-screen curses applications (vim, nano, tmux, htop).</p>\n<h2 id=\"truecolor\">24-Bit TrueColor RGB</h2>\n<p>Direct support for <code>\\033[38;2;R;G;Bm</code> providing 16.7M colors.</p>\n"
  },
  "terminal-shell": {
    "id": "terminal-shell",
    "title": "Shell",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ast-executor",
        "text": "AST Parser & Executor",
        "level": 2
      },
      {
        "id": "pipelines",
        "text": "Pipelines & Redirections",
        "level": 2
      }
    ],
    "summary": "Standalone POSIX-compliant AST execution engine, pipelines, and job control.",
    "body": "\n<p>Meridian includes an internal lexer, recursive-descent parser, and AST executor.</p>\n<h2 id=\"pipelines\">Pipelines & Redirections</h2>\n<pre><code class=\"language-bash\">cat log.txt | grep -i error | wc -l > errors.txt 2>&1</code></pre>\n"
  },
  "terminal-pty": {
    "id": "terminal-pty",
    "title": "PTY",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pty-overview",
        "text": "Linux openpty Architecture",
        "level": 2
      },
      {
        "id": "signals",
        "text": "Signal Handling & Resizing",
        "level": 2
      }
    ],
    "summary": "Asynchronous Linux openpty pseudoterminal multiplexer with non-blocking I/O.",
    "body": "\n<p>Meridian uses POSIX <code>openpty</code> to fork master/slave session pairs.</p>\n<h2 id=\"signals\">Signal Handling & Resizing</h2>\n<p>Propagates <code>SIGWINCH</code> window resize notifications and forwards <code>SIGINT</code>, <code>SIGTSTP</code>, and <code>SIGQUIT</code> directly to foreground process groups.</p>\n"
  },
  "terminal-commands": {
    "id": "terminal-commands",
    "title": "Commands",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "cli-cmds",
        "text": "Core Subcommands",
        "level": 2
      }
    ],
    "summary": "Meridian CLI commands, builtins, and subcommands.",
    "body": "\n<p>Meridian provides built-in commands for diagnostics, monitor, git status, and graphics:</p>\n<pre><code class=\"language-bash\">meridian ask \"find all pdf files\"\nmeridian diag \"cannot find -lutil\"\nmeridian monitor\nmeridian git\nmeridian pic image.png</code></pre>\n"
  },
  "terminal-ssh": {
    "id": "terminal-ssh",
    "title": "SSH",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ssh-overview",
        "text": "SSH Workspace Manager",
        "level": 2
      },
      {
        "id": "ssh-config",
        "text": "~/.ssh/config Integration",
        "level": 2
      }
    ],
    "summary": "SSH workspace manager, connection profiles, and remote session management.",
    "body": "\n<p>Meridian integrates with <code>~/.ssh/config</code> to provide a terminal workspace manager.</p>\n<h2 id=\"ssh-overview\">SSH Workspace Manager</h2>\n<pre><code class=\"language-bash\">meridian ssh              # List available SSH connection profiles\nmeridian ssh production   # Connect directly to production alias</code></pre>\n"
  },
  "terminal-keybindings": {
    "id": "terminal-keybindings",
    "title": "Keybindings",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "default-keys",
        "text": "Keybinding Table",
        "level": 2
      }
    ],
    "summary": "Default key shortcuts for tabs, panes, theme switcher, and command palette.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Shortcut</th><th>Action</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+P</code></td><td>Anime Gallery</td><td>Open theme gallery switcher</td></tr>\n    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td><td>Open fuzzy search command palette</td></tr>\n    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td><td>Split active terminal window vertically</td></tr>\n    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td><td>Split active terminal window horizontally</td></tr>\n    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td><td>Search across scrollback buffer & history</td></tr>\n  </tbody>\n</table>\n"
  },
  "terminal-clipboard": {
    "id": "terminal-clipboard",
    "title": "Clipboard",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "osc52",
        "text": "OSC 52 Clipboard Protocol",
        "level": 2
      }
    ],
    "summary": "OSC 52 remote clipboard synchronization, bracketed paste, and system clipboard.",
    "body": "\n<p>Meridian supports OSC 52 (<code>\\033]52;c;&lt;base64&gt;\\033\\\\</code>) for remote clipboard copy/paste over SSH sessions seamlessly.</p>\n"
  },
  "terminal-scrollback": {
    "id": "terminal-scrollback",
    "title": "Scrollback",
    "category": "TERMINAL",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "scrollback-overview",
        "text": "Scrollback Engine",
        "level": 2
      }
    ],
    "summary": "High-capacity scrollback buffer, search highlighting, and memory limits.",
    "body": "\n<p>Meridian stores up to 100,000 lines of scrollback history in a deque with fast substring and regex search.</p>\n"
  },
  "graphics-gpu": {
    "id": "graphics-gpu",
    "title": "GPU Renderer",
    "category": "GRAPHICS",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "damage-tracker",
        "text": "Damage Tracking Engine",
        "level": 2
      },
      {
        "id": "glyph-atlas",
        "text": "Glyph Texture Atlas",
        "level": 2
      }
    ],
    "summary": "Hardware-accelerated shader pipeline, damage tracking, and glyph texture atlas.",
    "body": "\n<p>Meridian tracks dirty screen rects to minimize draw calls and maximize battery life and framerate.</p>\n"
  },
  "graphics-inline-images": {
    "id": "graphics-inline-images",
    "title": "Inline Images",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pic-cmd",
        "text": "The pic Command",
        "level": 2
      }
    ],
    "summary": "Direct 32-bit RGBA inline image decoding and rendering via pic command.",
    "body": "\n<pre><code class=\"language-bash\">pic image.png       # Display raw image at (x=30, y=30)\npic --debug img.png # Inspect RGBA texture format and dimensions\npic --clear         # Remove active graphic</code></pre>\n"
  },
  "graphics-formats": {
    "id": "graphics-formats",
    "title": "Image Formats",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "formats",
        "text": "Format Support Matrix",
        "level": 2
      }
    ],
    "summary": "Supported image formats: PNG, JPEG, WebP, GIF, and BMP.",
    "body": "\n<p>Meridian uses <code>stb_image</code> to decode PNG, JPEG, WebP, GIF, and BMP directly into 32-bit RGBA pixel buffers.</p>\n"
  },
  "graphics-gif": {
    "id": "graphics-gif",
    "title": "GIF",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gif-playback",
        "text": "Animated GIF Engine",
        "level": 2
      }
    ],
    "summary": "Animated GIF playback and frame delay timing.",
    "body": "\n<p>Meridian decodes multi-frame GIF images and animates them at native frame delays on the canvas.</p>\n"
  },
  "graphics-kitty": {
    "id": "graphics-kitty",
    "title": "Kitty Graphics",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "kitty-chunking",
        "text": "Chunking & Safety",
        "level": 2
      }
    ],
    "summary": "Chunked 2048-byte transmission protocol with m=1/m=0 control frames.",
    "body": "\n<p>Automatically splits image payloads into 2048-byte safe chunks respecting the 4KB escape sequence limit.</p>\n"
  },
  "graphics-sixel": {
    "id": "graphics-sixel",
    "title": "Sixel",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "sixel-overview",
        "text": "DEC Sixel Engine",
        "level": 2
      }
    ],
    "summary": "DEC Sixel graphics protocol decoding and 256-color palette mapping.",
    "body": "\n<p>Decodes standard DEC Sixel bitmap escape sequences (<code>DCS q ... ST</code>) for compatibility with gnuplot and legacy tools.</p>\n"
  },
  "graphics-rendering": {
    "id": "graphics-rendering",
    "title": "Image Rendering",
    "category": "GRAPHICS",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "zero-ascii",
        "text": "Zero ASCII / Zero Half-Block Guarantee",
        "level": 2
      }
    ],
    "summary": "Detailed comparison between direct raster rendering and character-cell approximations.",
    "body": "\n<p>Meridian completely bypasses ASCII and half-block character approximations, rendering pure 32-bit RGBA hardware textures.</p>\n"
  },
  "interface-gui": {
    "id": "interface-gui",
    "title": "GUI",
    "category": "INTERFACE",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "canvas",
        "text": "Hardware Canvas & Windowing",
        "level": 2
      }
    ],
    "summary": "Qt6, Wayland, and X11 terminal canvas and window chrome.",
    "body": "\n<p>The GUI frontend integrates the core PTY and VT engine into modern desktop environments.</p>\n"
  },
  "interface-windows": {
    "id": "interface-windows",
    "title": "Windows",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "windows-overview",
        "text": "Window Management",
        "level": 2
      }
    ],
    "summary": "Multi-window support, transparency, and blur effects.",
    "body": "\n<p>Supports background transparency, acrylic/mica blur on supported compositors, and custom titles.</p>\n"
  },
  "interface-tabs": {
    "id": "interface-tabs",
    "title": "Tabs",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "tabs-shortcuts",
        "text": "Tab Shortcuts",
        "level": 2
      }
    ],
    "summary": "Tabbed terminal sessions, reordering, and shortcuts.",
    "body": "\n<p><code>Ctrl+Shift+T</code> to open new tabs, <code>Ctrl+Shift+W</code> to close tabs.</p>\n"
  },
  "interface-panes": {
    "id": "interface-panes",
    "title": "Panes",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "panes-split",
        "text": "Splitting Panes",
        "level": 2
      }
    ],
    "summary": "Multi-pane horizontal and vertical splits with persistent state.",
    "body": "\n<p><code>Ctrl+Shift+D</code> for vertical split, <code>Ctrl+Shift+E</code> for horizontal split, <code>Ctrl+Shift+Z</code> for zoom toggle.</p>\n"
  },
  "interface-themes": {
    "id": "interface-themes",
    "title": "Themes",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "theme-index",
        "text": "Complete Theme Index",
        "level": 2
      }
    ],
    "summary": "14 built-in anime picture themes and custom wallpaper configuration.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Index</th><th>ID</th><th>Theme</th></tr></thead>\n  <tbody>\n    <tr><td>0</td><td>sharingan_eye</td><td>Sasuke / Itachi Mangekyō Sharingan</td></tr>\n    <tr><td>1</td><td>sakura_girl</td><td>Sakura Blossom Anime Girl</td></tr>\n    <tr><td>2</td><td>ribbon_girl</td><td>Monochrome Anime Ribbon Girl</td></tr>\n    <tr><td>5</td><td>gojo_purple</td><td>Gojo Satoru: Hollow Purple (JJK)</td></tr>\n    <tr><td>6</td><td>sukuna_shrine</td><td>Sukuna: Malevolent Shrine (JJK)</td></tr>\n    <tr><td>7</td><td>naruto_rasengan</td><td>Naruto Uzumaki: Kurama Rasengan</td></tr>\n  </tbody>\n</table>\n"
  },
  "interface-powerline": {
    "id": "interface-powerline",
    "title": "Powerline",
    "category": "INTERFACE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "badges",
        "text": "Powerline Badges & Icons",
        "level": 2
      }
    ],
    "summary": "High-contrast Powerline status badges with Git branch detection.",
    "body": "\n<p>Renders real-time Git branch status, OS badges (󰌽 Linux), and window status with sub-millisecond latency.</p>\n"
  },
  "dev-architecture": {
    "id": "dev-architecture",
    "title": "Architecture",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "layers",
        "text": "Core Architecture Layers",
        "level": 2
      }
    ],
    "summary": "Architecture breakdown of PTY, VT parser, ScreenBuffer, and renderer layers.",
    "body": "\n<p>Meridian separates the UI from the underlying terminal emulation and PTY multiplexing engines.</p>\n"
  },
  "dev-pty-arch": {
    "id": "dev-pty-arch",
    "title": "PTY Architecture",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pty-internals",
        "text": "PTY Manager Internals",
        "level": 2
      }
    ],
    "summary": "POSIX openpty master/slave descriptor management and asynchronous I/O loops.",
    "body": "\n<p>Detailed design of non-blocking I/O polling, master/slave fd lifecycle, and POSIX termios flags.</p>\n"
  },
  "dev-screen-buffer": {
    "id": "dev-screen-buffer",
    "title": "Screen Buffer",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "screen-grid",
        "text": "Cell Grid Matrix",
        "level": 2
      }
    ],
    "summary": "2D cell matrix, damage tracking, and cursor management.",
    "body": "\n<p>Stores character codepoints, attributes, 24-bit TrueColor RGB, and hyperlink IDs per cell.</p>\n"
  },
  "dev-renderer": {
    "id": "dev-renderer",
    "title": "Renderer",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "render-pipeline",
        "text": "Render Pipeline",
        "level": 2
      }
    ],
    "summary": "Glyph caching, texture atlas, and GPU batching.",
    "body": "\n<p>Batches draw calls and optimizes glyph atlas caching for ultra-low latency rendering.</p>\n"
  },
  "dev-graphics-engine": {
    "id": "dev-graphics-engine",
    "title": "Graphics Engine",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "graphics-core",
        "text": "Graphics Subsystem",
        "level": 2
      }
    ],
    "summary": "stb_image decoding, aspect fitting, and image caching.",
    "body": "\n<p>Handles direct RGBA image decoding and manages texture lifecycles.</p>\n"
  },
  "dev-ai-engine": {
    "id": "dev-ai-engine",
    "title": "AI Engine",
    "category": "DEVELOPER",
    "status": "experimental",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "ask-engine",
        "text": "Intent Engine (ask)",
        "level": 2
      },
      {
        "id": "diag-engine",
        "text": "Compiler Diagnostics (diag)",
        "level": 2
      }
    ],
    "summary": "Offline natural language intent translation, compiler diagnostics, and autonomous coding agent.",
    "body": "\n<pre><code class=\"language-bash\">meridian ask \"find files larger than 1GB\"\nmeridian diag \"cannot find -lutil\"</code></pre>\n"
  },
  "dev-security": {
    "id": "dev-security",
    "title": "Security",
    "category": "DEVELOPER",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "risk-class",
        "text": "Risk Classification Interceptor",
        "level": 2
      }
    ],
    "summary": "Destructive command risk classification and credential leakage redactor.",
    "body": "\n<p>Classifies commands into Low, Medium, High, and Critical risk tiers and redacts API tokens from output.</p>\n"
  },
  "config-file": {
    "id": "config-file",
    "title": "Configuration File",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "schema",
        "text": "Schema Specification",
        "level": 2
      }
    ],
    "summary": "Specification of ~/.config/meridian/config.json options.",
    "body": "\n<pre><code class=\"language-json\">{\n  \"theme\": \"sharingan_eye\",\n  \"random_theme\": false,\n  \"font_size\": 14,\n  \"cursor_shape\": \"block\",\n  \"scrollback_lines\": 10000\n}</code></pre>\n"
  },
  "config-themes": {
    "id": "config-themes",
    "title": "Themes",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "custom-art",
        "text": "Custom Wallpaper Paths",
        "level": 2
      }
    ],
    "summary": "Configuring anime wallpaper themes and custom wallpapers.",
    "body": "\n<pre><code class=\"language-bash\">pic set /home/user/Pictures/wallpaper.png</code></pre>\n"
  },
  "config-keybindings": {
    "id": "config-keybindings",
    "title": "Keybindings",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "custom-keys",
        "text": "Keybinding JSON Format",
        "level": 2
      }
    ],
    "summary": "Customizing keyboard shortcuts via keybindings.json.",
    "body": "\n<p>Configure custom keybindings in <code>~/.config/meridian/keybindings.json</code>.</p>\n"
  },
  "config-profiles": {
    "id": "config-profiles",
    "title": "Profiles",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "profile-setup",
        "text": "Profile Configurations",
        "level": 2
      }
    ],
    "summary": "Setting up multiple terminal profiles (Shell, SSH, Containers).",
    "body": "\n<p>Define custom shell startup commands, environment variables, and working directories per profile.</p>\n"
  },
  "config-env": {
    "id": "config-env",
    "title": "Environment Variables",
    "category": "CONFIGURATION",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "env-vars",
        "text": "Environment Variable Reference",
        "level": 2
      }
    ],
    "summary": "Environment variables read and set by Meridian Terminal.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Variable</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>TERM</code></td><td>Set to <code>xterm-256color</code> or <code>meridian</code></td></tr>\n    <tr><td><code>COLORTERM</code></td><td>Set to <code>truecolor</code></td></tr>\n    <tr><td><code>MERIDIAN_CONFIG_HOME</code></td><td>Override config directory path</td></tr>\n  </tbody>\n</table>\n"
  },
  "pkg-fedora": {
    "id": "pkg-fedora",
    "title": "Fedora / RPM",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "dnf",
        "text": "DNF Installation",
        "level": 2
      }
    ],
    "summary": "Building and installing RPM packages with DNF and spec files.",
    "body": "\n<pre><code class=\"language-bash\">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>\n"
  },
  "pkg-debian": {
    "id": "pkg-debian",
    "title": "Ubuntu / Debian / DEB",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "dpkg",
        "text": "APT / DEB Installation",
        "level": 2
      }
    ],
    "summary": "Installing and building .deb packages for Ubuntu, Debian, and Linux Mint.",
    "body": "\n<pre><code class=\"language-bash\">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>\n"
  },
  "pkg-arch": {
    "id": "pkg-arch",
    "title": "Arch / AUR",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "makepkg",
        "text": "Arch PKGBUILD",
        "level": 2
      }
    ],
    "summary": "PKGBUILD packaging and AUR installation for Arch Linux and Manjaro.",
    "body": "\n<pre><code class=\"language-bash\">cd packaging/arch && makepkg -si</code></pre>\n"
  },
  "pkg-opensuse": {
    "id": "pkg-opensuse",
    "title": "openSUSE",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "zypper",
        "text": "Zypper Installation",
        "level": 2
      }
    ],
    "summary": "Installing RPM packages with Zypper on openSUSE Tumbleweed and Leap.",
    "body": "\n<pre><code class=\"language-bash\">sudo zypper install ./meridian-terminal.rpm</code></pre>\n"
  },
  "pkg-appimage": {
    "id": "pkg-appimage",
    "title": "AppImage",
    "category": "PACKAGING",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "appimage-run",
        "text": "Running AppImage",
        "level": 2
      }
    ],
    "summary": "Standalone self-contained AppImage package for any Linux distribution.",
    "body": "\n<pre><code class=\"language-bash\">chmod +x meridian-terminal.AppImage && ./meridian-terminal.AppImage</code></pre>\n"
  },
  "pkg-flatpak": {
    "id": "pkg-flatpak",
    "title": "Flatpak",
    "category": "PACKAGING",
    "status": "planned",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "flatpak-roadmap",
        "text": "Flatpak Roadmap",
        "level": 2
      }
    ],
    "summary": "Flatpak container package roadmap and sandbox permissions.",
    "body": "\n<p>Flatpak manifest with Host PTY and Wayland permissions is on the roadmap.</p>\n"
  },
  "pkg-snap": {
    "id": "pkg-snap",
    "title": "Snap",
    "category": "PACKAGING",
    "status": "planned",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "snap-roadmap",
        "text": "Snap Package Roadmap",
        "level": 2
      }
    ],
    "summary": "Snap packaging roadmap with classic confinement.",
    "body": "\n<p>Snap packaging with classic confinement is currently planned.</p>\n"
  },
  "pkg-macos": {
    "id": "pkg-macos",
    "title": "macOS",
    "category": "PACKAGING",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "macos-status",
        "text": "macOS Status",
        "level": 2
      }
    ],
    "summary": "macOS Darwin BSD PTY support, Homebrew formula, and DMG installer.",
    "body": "\n<p>macOS Darwin BSD PTY support and Homebrew formula are under active development.</p>\n"
  },
  "pkg-windows": {
    "id": "pkg-windows",
    "title": "Windows",
    "category": "PACKAGING",
    "status": "development",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "conpty",
        "text": "Windows ConPTY Core",
        "level": 2
      }
    ],
    "summary": "Windows 10/11 ConPTY pseudoterminal adapter and MSI installer.",
    "body": "\n<p>Windows Console PTY (ConPTY) adapter is currently in development.</p>\n"
  },
  "development-building": {
    "id": "development-building",
    "title": "Building",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "make",
        "text": "Make Targets",
        "level": 2
      }
    ],
    "summary": "Compiling Meridian Terminal with C++20, Make, and dependencies.",
    "body": "\n<pre><code class=\"language-bash\">make all -j$(nproc)\nmake test\nsudo ./install.sh</code></pre>\n"
  },
  "development-testing": {
    "id": "development-testing",
    "title": "Testing",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "run-tests",
        "text": "Running Tests",
        "level": 2
      }
    ],
    "summary": "Automated test suites covering PTY, VT parser, AST executor, and protocols.",
    "body": "\n<pre><code class=\"language-bash\">make test</code></pre>\n"
  },
  "development-debugging": {
    "id": "development-debugging",
    "title": "Debugging",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "telemetry",
        "text": "Telemetry Profiler",
        "level": 2
      }
    ],
    "summary": "GDB debugging, ASan, Valgrind, and Telemetry profiler.",
    "body": "\n<pre><code class=\"language-bash\">meridian --performance</code></pre>\n"
  },
  "development-contributing": {
    "id": "development-contributing",
    "title": "Contributing",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "pr-guide",
        "text": "Pull Request Guide",
        "level": 2
      }
    ],
    "summary": "How to report bugs, resolve visual glitches, add anime themes, and submit PRs.",
    "body": "\n<p>Open source contributions and bug fixes are warmly welcomed! Fork and submit a pull request on GitHub.</p>\n"
  },
  "development-release": {
    "id": "development-release",
    "title": "Release Process",
    "category": "DEVELOPMENT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "release-flow",
        "text": "Release Checklist",
        "level": 2
      }
    ],
    "summary": "Version tagging, packaging automation, and release checklist.",
    "body": "\n<p>Release workflow generates RPM, DEB, Arch, and offline tarballs automatically.</p>\n"
  },
  "ref-cli": {
    "id": "ref-cli",
    "title": "CLI Reference",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "subcommands-ref",
        "text": "Subcommands Index",
        "level": 2
      }
    ],
    "summary": "Complete command-line interface options and subcommands.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Command</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>meridian</code></td><td>Launch interactive terminal</td></tr>\n    <tr><td><code>meridian ssh [alias]</code></td><td>Manage & connect to SSH workspace</td></tr>\n    <tr><td><code>meridian plugins</code></td><td>List active extensible plugins</td></tr>\n    <tr><td><code>meridian --performance</code></td><td>Display GPU & PTY telemetry profiler</td></tr>\n    <tr><td><code>meridian pic &lt;file&gt;</code></td><td>Display direct inline raster image</td></tr>\n  </tbody>\n</table>\n"
  },
  "ref-config": {
    "id": "ref-config",
    "title": "Configuration Reference",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "keys-ref",
        "text": "Config Keys Table",
        "level": 2
      }
    ],
    "summary": "Complete schema documentation for config.json.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Key</th><th>Type</th><th>Description</th></tr></thead>\n  <tbody>\n    <tr><td><code>theme</code></td><td>string</td><td>Startup anime artwork theme</td></tr>\n    <tr><td><code>font_size</code></td><td>number</td><td>Font size in points</td></tr>\n    <tr><td><code>scrollback_lines</code></td><td>number</td><td>Max scrollback history lines</td></tr>\n  </tbody>\n</table>\n"
  },
  "ref-shortcuts": {
    "id": "ref-shortcuts",
    "title": "Keyboard Shortcuts",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "shortcuts-ref",
        "text": "Shortcut Reference",
        "level": 2
      }
    ],
    "summary": "Complete table of all default keyboard shortcuts.",
    "body": "\n<table class=\"doc-table\">\n  <thead><tr><th>Shortcut</th><th>Action</th></tr></thead>\n  <tbody>\n    <tr><td><code>Ctrl+P</code></td><td>Anime Theme Gallery</td></tr>\n    <tr><td><code>Ctrl+Shift+P</code></td><td>Command Palette</td></tr>\n    <tr><td><code>Ctrl+Shift+D</code></td><td>Split Vertical</td></tr>\n    <tr><td><code>Ctrl+Shift+E</code></td><td>Split Horizontal</td></tr>\n    <tr><td><code>Ctrl+Shift+F</code></td><td>Universal Search</td></tr>\n  </tbody>\n</table>\n"
  },
  "ref-env": {
    "id": "ref-env",
    "title": "Environment Variables",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "env-ref",
        "text": "Environment Variables",
        "level": 2
      }
    ],
    "summary": "Environment variables used and exported by Meridian.",
    "body": "\n<p><code>TERM=xterm-256color</code>, <code>COLORTERM=truecolor</code>, <code>MERIDIAN_CONFIG_HOME</code>.</p>\n"
  },
  "ref-faq": {
    "id": "ref-faq",
    "title": "FAQ",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "general-faq",
        "text": "General Questions",
        "level": 2
      }
    ],
    "summary": "Frequently asked questions about Meridian Shell.",
    "body": "\n<p><strong>Q: Does Meridian render real images or ASCII?</strong><br/>A: Meridian renders direct 32-bit RGBA hardware raster graphics with zero ASCII conversion.</p>\n"
  },
  "ref-troubleshooting": {
    "id": "ref-troubleshooting",
    "title": "Troubleshooting",
    "category": "REFERENCE",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "common-issues",
        "text": "Common Solutions",
        "level": 2
      }
    ],
    "summary": "Solutions for font rendering, PTY permissions, and graphical display issues.",
    "body": "\n<p>Ensure Nerd Fonts are installed for Powerline glyphs, and verify GPU drivers for hardware raster graphics.</p>\n"
  },
  "proj-github": {
    "id": "proj-github",
    "title": "GitHub",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "repo-links",
        "text": "GitHub Links",
        "level": 2
      }
    ],
    "summary": "Source code repository, issue tracker, and community discussions.",
    "body": "\n<p>Repository: <a href=\"https://github.com/charanbalaji2005/Meridian-Shell\" target=\"_blank\" style=\"color: #00E5FF;\">https://github.com/charanbalaji2005/Meridian-Shell</a></p>\n"
  },
  "proj-changelog": {
    "id": "proj-changelog",
    "title": "Changelog",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "v2-0",
        "text": "Meridian 2.0.0 (August 2026)",
        "level": 2
      }
    ],
    "summary": "Release history and major milestone changes.",
    "body": "\n<h2 id=\"v2-0\">Meridian 2.0.0 (August 2026)</h2>\n<ul>\n  <li>Direct 32-bit RGBA inline image decoding engine (zero ASCII).</li>\n  <li>OSC 8 Hyperlinks & OSC 52 Remote Clipboard.</li>\n  <li>OSC 7 Working Directory tracking & OSC 133 Semantic Prompts.</li>\n  <li>Native SSH Workspace Manager & Extensible Plugin Engine.</li>\n  <li>GPU Telemetry Profiler (meridian --performance).</li>\n</ul>\n"
  },
  "proj-license": {
    "id": "proj-license",
    "title": "License",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "gpl",
        "text": "GNU GPLv3 License",
        "level": 2
      }
    ],
    "summary": "Free and open-source GNU General Public License v3.0.",
    "body": "\n<p>Meridian Terminal is free and open-source software licensed under the <strong>GNU General Public License v3.0 or later</strong>.</p>\n"
  },
  "proj-contributing": {
    "id": "proj-contributing",
    "title": "Contributing",
    "category": "PROJECT",
    "status": "implemented",
    "lastUpdated": "August 25, 2026",
    "headings": [
      {
        "id": "welcome",
        "text": "Welcome Contributors!",
        "level": 2
      }
    ],
    "summary": "Open source contribution guide, bug fixes, and anime theme submissions.",
    "body": "\n<div class=\"note-box note-success\">\n  <div class=\"note-title\">🤝 Open Source Community Welcome!</div>\n  <p>Contributions of all kinds are warmly welcomed! Bug fixes, rendering improvements, new anime themes, and shell features are highly encouraged.</p>\n</div>\n"
  }
};
