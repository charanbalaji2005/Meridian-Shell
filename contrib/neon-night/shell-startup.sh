# Neon Night greeting: a pixel-art sprite above fastfetch, only in Zephyr.
# Add to the END of ~/.zshrc or ~/.bashrc:
#     source /path/to/contrib/neon-night/shell-startup.sh
#
# Sprite sources, first one found wins:
#   1. ZEPHYR_SPRITE=/path/to/image.png  -> rendered with chafa as true-color
#      half blocks (any image; works in every true-color terminal)
#   2. pokemon-colorscripts               -> random sprite (AUR: pokemon-colorscripts-git)
# Zephyr sets TERM_PROGRAM=Zephyr for every shell it starts.

if [ "$TERM_PROGRAM" = "Zephyr" ] && [ -z "$ZEPHYR_GREETED" ] && [[ $- == *i* ]]; then
    export ZEPHYR_GREETED=1   # once per terminal, not in nested shells
    if [ -n "$ZEPHYR_SPRITE" ] && command -v chafa >/dev/null 2>&1; then
        chafa --format symbols --symbols block --size 28x14 "$ZEPHYR_SPRITE"
    elif command -v pokemon-colorscripts >/dev/null 2>&1; then
        pokemon-colorscripts --random --no-title
    fi
    command -v fastfetch >/dev/null 2>&1 && fastfetch
fi

# Green arrow prompt like the screenshot (zsh). Remove if you use starship/p10k.
if [ -n "$ZSH_VERSION" ]; then
    PROMPT='%F{green}❯%f '
fi
