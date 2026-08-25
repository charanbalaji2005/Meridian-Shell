export interface DocSection {
  id: string;
  title: string;
  category: string;
  content: string;
  headings: { id: string; text: string; level: number }[];
  status?: 'implemented' | 'development' | 'experimental';
}

export interface NavCategory {
  title: string;
  icon: string;
  items: { id: string; title: string; status?: 'implemented' | 'development' | 'experimental' }[];
}

export const NAV_STRUCTURE: NavCategory[] = [
  {
    title: 'Getting Started',
    icon: 'Compass',
    items: [
      { id: 'intro', title: 'Introduction', status: 'implemented' },
      { id: 'installation', title: 'Installation Guide', status: 'implemented' },
      { id: 'quickstart', title: 'Quick Start & First Run', status: 'implemented' },
      { id: 'lifecycle', title: 'Updates & Uninstallation', status: 'implemented' },
    ],
  },
  {
    title: 'Terminal & Shell',
    icon: 'Terminal',
    items: [
      { id: 'emulation', title: 'Terminal Emulation (VT/XTerm)', status: 'implemented' },
      { id: 'shell-engine', title: 'POSIX Shell & Builtins', status: 'implemented' },
      { id: 'history-search', title: 'Rich History & Universal Search', status: 'implemented' },
      { id: 'keybindings', title: 'Keybindings & Shortcuts', status: 'implemented' },
    ],
  },
  {
    title: 'Graphics & Artwork',
    icon: 'Image',
    items: [
      { id: 'raster-images', title: 'Direct Raster Image Pipeline', status: 'implemented' },
      { id: 'kitty-protocol', title: 'Kitty Graphics Protocol', status: 'implemented' },
      { id: 'anime-gallery', title: 'Anime Gallery & Custom Art', status: 'implemented' },
    ],
  },
  {
    title: 'AI & Intelligence',
    icon: 'Sparkles',
    items: [
      { id: 'ai-intent', title: 'Natural Language Intent (ask)', status: 'implemented' },
      { id: 'ai-diagnostics', title: 'Automated Error Diagnostics (diag)', status: 'implemented' },
      { id: 'ai-agent', title: 'Autonomous Coding Agent (agent)', status: 'experimental' },
      { id: 'ai-security', title: 'Safety Defense & Secret Redactor', status: 'implemented' },
    ],
  },
  {
    title: 'Developer Tooling',
    icon: 'Wrench',
    items: [
      { id: 'dev-monitor', title: 'System Resource Monitor', status: 'implemented' },
      { id: 'dev-git', title: 'Git Intelligence & Badges', status: 'implemented' },
      { id: 'dev-files', title: 'File Tree Explorer', status: 'implemented' },
      { id: 'dev-palette', title: 'Command Palette (Ctrl+Shift+P)', status: 'implemented' },
      { id: 'dev-workspace', title: 'Workspace Multiplexer & Panes', status: 'implemented' },
    ],
  },
  {
    title: 'Architecture & Core',
    icon: 'Layers',
    items: [
      { id: 'arch-pty', title: 'Linux PTY Manager', status: 'implemented' },
      { id: 'arch-buffer', title: 'ScreenBuffer & Damage Tracker', status: 'implemented' },
      { id: 'arch-glyph', title: 'Glyph Atlas & TrueColor Engine', status: 'implemented' },
      { id: 'arch-security', title: 'Encrypted Credential Store', status: 'implemented' },
    ],
  },
  {
    title: 'Packaging & Releases',
    icon: 'Package',
    items: [
      { id: 'pkg-fedora', title: 'Fedora / RHEL (DNF & RPM)', status: 'implemented' },
      { id: 'pkg-debian', title: 'Ubuntu / Debian (APT & DEB)', status: 'implemented' },
      { id: 'pkg-arch', title: 'Arch Linux (Pacman & AUR)', status: 'implemented' },
      { id: 'pkg-macos-win', title: 'macOS & Windows', status: 'development' },
    ],
  },
  {
    title: 'Community & Status',
    icon: 'Users',
    items: [
      { id: 'contributing', title: 'Contributing Guidelines', status: 'implemented' },
      { id: 'status-matrix', title: 'Component Status Matrix', status: 'implemented' },
    ],
  },
];
