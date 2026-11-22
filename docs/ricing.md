# Neon Night look (glowing translucent terminal)

Recreates the popular "riced" desktop terminal: a translucent purple-navy
window, a glowing pink→violet rounded border, a pixel-art sprite and a
`fastfetch` system summary. Files live in `contrib/neon-night/`.

| Piece | Who draws it | How |
|---|---|---|
| Translucent body, rounded corners, glowing border | Zephyr | Ctrl+Shift+P → **Apply Neon Night look** (or Settings → Appearance → Window style) |
| Sprite + system info with icons | programs in your shell | `fastfetch.jsonc` + `shell-startup.sh` |
| Blur behind the window | your compositor | Hyprland/Picom/KWin — see below |

## 1. Zephyr

1. Install a Nerd Font, e.g. `ttf-jetbrains-mono-nerd` (Arch), `nerd-fonts` on
   Fedora via COPR, or download *JetBrainsMono Nerd Font* from nerdfonts.com.
2. Settings → Fonts → *JetBrainsMono Nerd Font*, size 11.
3. Ctrl+Shift+P → **Apply Neon Night look**. This sets the *Neon Night*
   color scheme, pink accent, 82% opacity, the glowing rounded border and
   minimal mode (no header/sidebar).
4. Optional: Settings → Appearance → *Frameless window* for no title bar.
   Drag the glowing border to move, its outer edge to resize, double-click
   it to maximize.

Get the header back with Ctrl+Shift+P → *Toggle minimal window*, or
right-click → *Show Header & Sidebar*. Settings is always Ctrl+,.

Equivalent `~/.config/zephyr/terminal.toml` lines:

```toml
theme_name = "neon-night"
accent_color = "#ff6ad5"
window_opacity = "82"
glow_border = "true"
glow_color = "#ff5fd2"
glow_color2 = "#8b7cf6"
corner_radius = "14"
minimal_chrome = "true"
frameless_window = "true"
padding = "12"
font_family = "JetBrainsMono Nerd Font"
font_size = "11"
```

## 2. Sprite and fastfetch

```bash
sudo pacman -S fastfetch chafa          # Fedora: dnf, Debian/Ubuntu: apt
mkdir -p ~/.config/fastfetch
cp contrib/neon-night/fastfetch.jsonc ~/.config/fastfetch/config.jsonc
echo "source $PWD/contrib/neon-night/shell-startup.sh" >> ~/.zshrc
```

Sprite: set `export ZEPHYR_SPRITE=~/Pictures/my-sprite.png` (any small
pixel-art PNG; drawn with `chafa` as true-color half blocks), or install
`pokemon-colorscripts-git` from the AUR for random sprites.

## 3. Blur (compositor)

Zephyr makes the background translucent; *blurring* what's behind it is the
compositor's job.

- **Hyprland**: append `contrib/neon-night/hyprland.conf`. If Hyprland
  draws the border, turn Zephyr's glowing border off to avoid two borders.
- **Picom (X11)**: `blur-method = "dual_kawase";` and `corner-radius = 14;`.
- **KDE Plasma / GNOME**: translucency works; per-window blur isn't
  requested by Zephyr yet, so the background shows through unblurred.

## Notes

- The border glow is drawn inside the window (12 px margin), so it looks
  the same everywhere, including desktops without shadows.
- It costs nothing while typing: terminal output only repaints the
  interior; the glow is redrawn on resize.
- Not yet verified on a real desktop (see docs/VALIDATION.md).
