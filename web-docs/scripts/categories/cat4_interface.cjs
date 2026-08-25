module.exports = function(addArt) {
  addArt('interface-gui', 'GUI', 'INTERFACE', 'development',
    'Qt6, Wayland, and X11 terminal canvas and window chrome.',
    [
      { id: 'gui-arch', text: 'GUI Windowing Architecture', level: 2 },
      { id: 'wayland-x11', text: 'Wayland & X11 Compositor Integration', level: 2 },
      { id: 'scaling-dpi', text: 'HiDPI & Fractional Scaling', level: 2 }
    ],
    `
<h2 id="gui-arch">GUI Windowing Architecture</h2>
<p>Meridian's GUI layer embeds the high-speed PTY engine and OpenGL/Vulkan canvas into native desktop window chrome, providing responsive rendering and native platform integration.</p>

<h2 id="wayland-x11">Wayland & X11 Compositor Integration</h2>
<p>Supports fractional scaling, client-side decorations (CSD), server-side decorations (SSD), and sub-pixel glyph rendering across GNOME, KDE Plasma, and Hyprland.</p>

<h2 id="scaling-dpi">HiDPI & Fractional Scaling</h2>
<p>Under Wayland <code>wp-fractional-scale-v1</code>, Meridian dynamically scales glyph texture atlases to exact fractional physical pixel dimensions (125%, 150%, 175%), completely avoiding blurry bitmap bilinear interpolation.</p>
`
  );

  addArt('interface-windows', 'Windows', 'INTERFACE', 'implemented',
    'Multi-window support, transparency, and blur effects.',
    [
      { id: 'transparency', text: 'Window Transparency & Background Blur', level: 2 },
      { id: 'multi-window', text: 'Multi-Window Session Management', level: 2 },
      { id: 'geometry-persistence', text: 'Window Geometry & State Persistence', level: 2 }
    ],
    `
<h2 id="transparency">Window Transparency & Background Blur</h2>
<p>Configure background opacity (<code>0.0</code> to <code>1.0</code>) and enable compositor blur in <code>~/.config/meridian/config.json</code>:</p>
<pre><code class="language-json">{
  "opacity": 0.92,
  "blur": true
}</code></pre>

<h2 id="multi-window">Multi-Window Session Management</h2>
<p>Press <code>Ctrl+Shift+N</code> to open a new independent window sharing the same session daemon.</p>

<h2 id="geometry-persistence">Window Geometry & State Persistence</h2>
<p>Window position, dimensions, maximized state, and split layouts are automatically serialized and restored across application relaunches.</p>
`
  );

  addArt('interface-tabs', 'Tabs', 'INTERFACE', 'implemented',
    'Tabbed terminal sessions, reordering, and shortcuts.',
    [
      { id: 'tabs-workflow', text: 'Tabbed Workflow', level: 2 },
      { id: 'tabs-shortcuts-spec', text: 'Tab Shortcuts Reference', level: 2 },
      { id: 'cwd-tab-titles', text: 'Dynamic CWD & Process Titles', level: 2 }
    ],
    `
<h2 id="tabs-workflow">Tabbed Workflow</h2>
<p>Tabs maintain independent PTY sessions, working directories, and scrollback histories.</p>

<h2 id="tabs-shortcuts-spec">Tab Shortcuts Reference</h2>
<ul>
  <li><code>Ctrl+Shift+T</code>: New Tab in current directory.</li>
  <li><code>Ctrl+Shift+W</code>: Close current Tab.</li>
  <li><code>Ctrl+PageUp</code> / <code>Ctrl+PageDown</code>: Switch between tabs sequentially.</li>
  <li><code>Alt+1..9</code>: Jump directly to Tab N.</li>
</ul>

<h2 id="cwd-tab-titles">Dynamic CWD & Process Titles</h2>
<p>Tab headers update dynamically in response to OSC 7 directory tracking and foreground process execution (e.g., <code>vim: src/main.cpp</code> or <code>cargo: build</code>).</p>
`
  );

  addArt('interface-panes', 'Panes', 'INTERFACE', 'implemented',
    'Multi-pane horizontal and vertical splits with persistent state.',
    [
      { id: 'pane-tree-arch', text: 'Binary Pane Tree Architecture', level: 2 },
      { id: 'pane-shortcuts-spec', text: 'Pane Splitting & Zoom Shortcuts', level: 2 },
      { id: 'pane-resizing', text: 'Pane Resizing & Layout Persistence', level: 2 }
    ],
    `
<h2 id="pane-tree-arch">Binary Pane Tree Architecture</h2>
<p><code>workspace::PaneTree</code> models terminal splits as an n-ary tree of leaves, recalculating fractional layout rectangles on resize.</p>

<h2 id="pane-shortcuts-spec">Pane Splitting & Zoom Shortcuts</h2>
<ul>
  <li><code>Ctrl+Shift+D</code>: Split active pane vertically.</li>
  <li><code>Ctrl+Shift+E</code>: Split active pane horizontally.</li>
  <li><code>Ctrl+Shift+Z</code>: Toggle zoom on active pane.</li>
  <li><code>Alt+Arrows</code>: Navigate focus between adjacent panes.</li>
</ul>

<h2 id="pane-resizing">Pane Resizing & Layout Persistence</h2>
<p>Drag pane divider borders with the mouse or use <code>Ctrl+Shift+Alt+Arrows</code> to adjust split ratios in 5% increments.</p>
`
  );

  addArt('interface-themes', 'Themes', 'INTERFACE', 'implemented',
    '14 built-in anime picture themes and custom wallpaper configuration.',
    [
      { id: 'anime-gallery', text: 'Embedded Anime Theme Gallery', level: 2 },
      { id: 'theme-selection-cli', text: 'CLI Theme Commands (pic set)', level: 2 },
      { id: 'wallpaper-customization', text: 'Custom Wallpaper & Scaling Modes', level: 2 }
    ],
    `
<h2 id="anime-gallery">Embedded Anime Theme Gallery</h2>
<table class="doc-table">
  <thead><tr><th>Index</th><th>Identifier</th><th>Artwork Description</th></tr></thead>
  <tbody>
    <tr><td>0</td><td><code>sharingan_eye</code></td><td>Sasuke / Itachi Mangekyō Sharingan Eye</td></tr>
    <tr><td>1</td><td><code>sakura_girl</code></td><td>Sakura Blossom Anime Girl</td></tr>
    <tr><td>2</td><td><code>ribbon_girl</code></td><td>Monochrome Anime Ribbon Girl</td></tr>
    <tr><td>3</td><td><code>cyberpunk_edge</code></td><td>Cyberpunk Edgerunners Neon City</td></tr>
    <tr><td>4</td><td><code>mecha_warrior</code></td><td>Gundam Mecha Orbital Warrior</td></tr>
    <tr><td>5</td><td><code>gojo_purple</code></td><td>Gojo Satoru: Hollow Purple (JJK)</td></tr>
    <tr><td>6</td><td><code>sukuna_shrine</code></td><td>Ryomen Sukuna: Malevolent Shrine (JJK)</td></tr>
    <tr><td>7</td><td><code>naruto_rasengan</code></td><td>Naruto Uzumaki: Nine-Tails Rasengan</td></tr>
    <tr><td>8</td><td><code>demon_slayer_tanjiro</code></td><td>Tanjiro Kamado: Sun Breathing</td></tr>
    <tr><td>9</td><td><code>nezuko_blood</code></td><td>Nezuko Kamado: Blood Demon Art</td></tr>
  </tbody>
</table>

<h2 id="theme-selection-cli">CLI Theme Commands (pic set)</h2>
<pre><code class="language-bash">pic set 2               # Set Theme 2 (Ribbon Girl)
pic set gojo_purple     # Set by theme name
pic set random          # Randomize theme on each startup
pic set /path/to/my.png # Set custom user wallpaper</code></pre>

<h2 id="wallpaper-customization">Custom Wallpaper & Scaling Modes</h2>
<p>Configure wallpaper placement modes in <code>config.json</code>: <code>"fit"</code>, <code>"fill"</code>, <code>"center"</code>, or <code>"stretch"</code>.</p>
`
  );

  addArt('interface-powerline', 'Powerline', 'INTERFACE', 'implemented',
    'High-contrast Powerline status badges with Git branch detection.',
    [
      { id: 'powerline-badges', text: 'Powerline Status Badges', level: 2 },
      { id: 'git-status-segment', text: 'Real-time Git Status Segment', level: 2 },
      { id: 'custom-segments', text: 'Configuring Custom Segments', level: 2 }
    ],
    `
<h2 id="powerline-badges">Powerline Status Badges</h2>
<p>Meridian renders hardware-accelerated Powerline glyphs (, , , 󰌽) showing time, OS icon, current working directory, and exit codes.</p>

<h2 id="git-status-segment">Real-time Git Status Segment</h2>
<p><code>dev::GitIntel</code> polls the nearest <code>.git</code> repository, displaying branch name ( main), ahead/behind status (↑1 ↓0), and dirty state (●).</p>

<h2 id="custom-segments">Configuring Custom Segments</h2>
<p>Enable or disable segments in <code>~/.config/meridian/config.json</code> under the <code>"powerline"</code> section (e.g. <code>"show_time": true</code>, <code>"show_git": true</code>, <code>"show_exit_code": true</code>).</p>
`
  );
};
