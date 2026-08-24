#!/usr/bin/env bash
# ==============================================================================
# Meridian Terminal — Universal Multi-Distribution Linux Installer
# https://github.com/meridian-terminal/meridian-terminal
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/meridian-terminal/meridian-terminal/main/install.sh | bash
#   or locally:
#   ./install.sh [options]
# ==============================================================================

set -euo pipefail

APP_VERSION="0.1.0"
REPO_URL="https://github.com/meridian-terminal/meridian-terminal"
TARBALL_URL="https://github.com/meridian-terminal/meridian-terminal/archive/v${APP_VERSION}/meridian-terminal-${APP_VERSION}.tar.gz"

# ------------------------------------------------------------------------------
# Colors & Formatting
# ------------------------------------------------------------------------------
if [ -t 1 ]; then
    BOLD="\033[1m"
    DIM="\033[2m"
    GREEN="\033[32m"
    YELLOW="\033[33m"
    CYAN="\033[36m"
    RED="\033[31m"
    RESET="\033[0m"
else
    BOLD=""
    DIM=""
    GREEN=""
    YELLOW=""
    CYAN=""
    RED=""
    RESET=""
fi

log_info()    { echo -e "${CYAN}==>${RESET} ${BOLD}$*${RESET}"; }
log_success() { echo -e "${GREEN}==>${RESET} ${BOLD}$*${RESET}"; }
log_warn()    { echo -e "${YELLOW}==> WARNING:${RESET} $*"; }
log_error()   { echo -e "${RED}==> ERROR:${RESET} $*" >&2; }

# ------------------------------------------------------------------------------
# Defaults & Command-line flags
# ------------------------------------------------------------------------------
MODE="install"
DRY_RUN=0
NON_INTERACTIVE=0
INSTALL_PREFIX=""
USER_MODE=0

show_help() {
    cat <<EOF
Meridian Terminal — Universal Linux Installer

Usage:
  ./install.sh [options]

Options:
  -u, --user           Install for current user only (~/.local) without root/sudo
  -s, --system         Install system-wide to /usr/local (requires sudo)
  --prefix <PATH>      Custom installation prefix (e.g. /usr, /opt/meridian)
  -y, --yes            Automatic yes to prompts (non-interactive mode)
  -n, --dry-run        Print detection and planned actions without making changes
  --uninstall          Remove previously installed Meridian Terminal files
  -v, --version        Show installer version
  -h, --help           Show this help message

Examples:
  curl -fsSL https://get.meridian-terminal.dev | bash
  ./install.sh --user
  ./install.sh --prefix /opt/meridian
  ./install.sh --uninstall

EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -u|--user)
            USER_MODE=1
            shift
            ;;
        -s|--system)
            USER_MODE=0
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -y|--yes)
            NON_INTERACTIVE=1
            shift
            ;;
        -n|--dry-run)
            DRY_RUN=1
            shift
            ;;
        --uninstall)
            MODE="uninstall"
            shift
            ;;
        -v|--version)
            echo "Meridian Terminal Installer v${APP_VERSION}"
            exit 0
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# ------------------------------------------------------------------------------
# Determine Target Installation Prefix & Permissions
# ------------------------------------------------------------------------------
is_path_writable() {
    local target="$1"
    local check="$target"
    while [ -n "$check" ] && [ ! -d "$check" ]; do
        check="$(dirname "$check")"
    done
    [ -w "${check:-/}" ]
}

if [ -n "$INSTALL_PREFIX" ]; then
    PREFIX="$INSTALL_PREFIX"
    if is_path_writable "$PREFIX" || [ "$EUID" -eq 0 ]; then
        USE_SUDO=0
    else
        USE_SUDO=1
    fi
elif [ "$USER_MODE" -eq 1 ]; then
    PREFIX="$HOME/.local"
    USE_SUDO=0
elif [ "$EUID" -eq 0 ]; then
    PREFIX="/usr/local"
    USE_SUDO=0
else
    # Non-root user with no explicit prefix
    if is_path_writable "/usr/local"; then
        PREFIX="/usr/local"
        USE_SUDO=0
    elif command -v sudo >/dev/null 2>&1; then
        # Default to user mode if sudo cannot be used or user prefers local
        PREFIX="$HOME/.local"
        USE_SUDO=0
    else
        PREFIX="$HOME/.local"
        USE_SUDO=0
    fi
fi

BINDIR="$PREFIX/bin"
DATADIR="$PREFIX/share"
DESKTOPDIR="$DATADIR/applications"
ICONDIR="$DATADIR/icons/hicolor/scalable/apps"
METAINFODIR="$DATADIR/metainfo"
DOCDIR="$DATADIR/doc/meridian-terminal"

run_cmd() {
    if [ "$DRY_RUN" -eq 1 ]; then
        echo -e "${DIM}[dry-run] $*${RESET}"
        return 0
    fi
    if [ "${USE_SUDO:-0}" -eq 1 ] && [ "$EUID" -ne 0 ]; then
        sudo "$@"
    else
        "$@"
    fi
}

# ------------------------------------------------------------------------------
# Distribution Detection
# ------------------------------------------------------------------------------
detect_distribution() {
    OS_ID="unknown"
    OS_LIKE=""
    OS_NAME="Linux"

    if [ -f /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        OS_ID="${ID:-unknown}"
        OS_LIKE="${ID_LIKE:-}"
        OS_NAME="${PRETTY_NAME:-$NAME}"
    elif [ -f /etc/redhat-release ]; then
        OS_ID="rhel"
        OS_NAME=$(cat /etc/redhat-release)
    elif [ -f /etc/debian_version ]; then
        OS_ID="debian"
        OS_NAME="Debian $(cat /etc/debian_version)"
    fi
}

# ------------------------------------------------------------------------------
# Handle Uninstallation
# ------------------------------------------------------------------------------
do_uninstall() {
    log_info "Uninstalling Meridian Terminal from prefix: ${PREFIX}"

    FILES_TO_REMOVE=(
        "$BINDIR/meridian"
        "$BINDIR/meridian-shell"
        "$BINDIR/meridian_demo"
        "$DESKTOPDIR/org.meridian_terminal.MeridianTerminal.desktop"
        "$ICONDIR/meridian-terminal.svg"
        "$METAINFODIR/org.meridian_terminal.MeridianTerminal.metainfo.xml"
    )

    for f in "${FILES_TO_REMOVE[@]}"; do
        if [ -e "$f" ]; then
            log_info "Removing $f"
            run_cmd rm -f "$f"
        fi
    done

    if [ -d "$DOCDIR" ]; then
        log_info "Removing $DOCDIR"
        run_cmd rm -rf "$DOCDIR"
    fi

    # Update desktop database if available
    if command -v update-desktop-database >/dev/null 2>&1; then
        run_cmd update-desktop-database "$DESKTOPDIR" 2>/dev/null || true
    fi

    log_success "Meridian Terminal has been successfully uninstalled."
    exit 0
}

# ------------------------------------------------------------------------------
# Main Installation Flow
# ------------------------------------------------------------------------------
main() {
    echo -e "${CYAN}======================================================${RESET}"
    echo -e "${BOLD}       Meridian Terminal — Universal Installer        ${RESET}"
    echo -e "${CYAN}======================================================${RESET}"

    detect_distribution
    log_info "Detected OS: ${BOLD}${OS_NAME}${RESET} (${OS_ID})"
    log_info "Installation target prefix: ${BOLD}${PREFIX}${RESET}"

    if [ "$MODE" = "uninstall" ]; then
        do_uninstall
    fi

    # Check for required tools
    MISSING_TOOLS=()
    for tool in g++ make tar; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            MISSING_TOOLS+=("$tool")
        fi
    done

    if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
        log_warn "Missing required build tool(s): ${MISSING_TOOLS[*]}"
        log_info "To install required build tools on ${OS_NAME}:"

        case "$OS_ID" in
            fedora|rhel|centos|rocky|almalinux|aetheros)
                echo "  sudo dnf install gcc-c++ make"
                ;;
            ubuntu|debian|linuxmint|pop)
                echo "  sudo apt update && sudo apt install build-essential"
                ;;
            arch|manjaro|endeavouros)
                echo "  sudo pacman -S base-devel"
                ;;
            opensuse*|suse)
                echo "  sudo zypper install gcc-c++ make"
                ;;
            alpine)
                echo "  sudo apk add g++ make"
                ;;
            *)
                echo "  Please install a C++20 compiler (g++ >= 11) and make using your package manager."
                ;;
        esac

        if [ "$DRY_RUN" -eq 0 ]; then
            log_error "Cannot proceed without required build tools."
            exit 1
        fi
    fi

    # Determine build source directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    WORK_DIR=""

    if [ -f "$SCRIPT_DIR/Makefile" ] && [ -d "$SCRIPT_DIR/src" ]; then
        log_info "Using local source tree at: $SCRIPT_DIR"
        SRC_DIR="$SCRIPT_DIR"
    else
        log_info "Downloading latest Meridian Terminal release..."
        WORK_DIR=$(mktemp -d "/tmp/meridian-build-XXXXXX")
        SRC_DIR="$WORK_DIR/meridian-terminal-${APP_VERSION}"
        if [ "$DRY_RUN" -eq 0 ]; then
            curl -fsSL "$TARBALL_URL" | tar -xz -C "$WORK_DIR"
        fi
    fi

    # Build
    log_info "Building Meridian Terminal (C++20)..."
    if [ "$DRY_RUN" -eq 0 ]; then
        make -C "$SRC_DIR" all
    fi

    # Run core self-tests
    log_info "Running test suite..."
    if [ "$DRY_RUN" -eq 0 ]; then
        make -C "$SRC_DIR" test
    fi

    # Install
    log_info "Installing binaries and desktop integration files..."
    run_cmd make -C "$SRC_DIR" install DESTDIR="" PREFIX="$PREFIX"

    # Clean up temp workdir if created
    if [ -n "$WORK_DIR" ] && [ -d "$WORK_DIR" ]; then
        rm -rf "$WORK_DIR"
    fi

    # Desktop database refresh
    if command -v update-desktop-database >/dev/null 2>&1; then
        run_cmd update-desktop-database "$DESKTOPDIR" 2>/dev/null || true
    fi

    # Success summary
    echo
    echo -e "${GREEN}======================================================${RESET}"
    log_success "Meridian Terminal v${APP_VERSION} installed successfully!"
    echo -e "${GREEN}======================================================${RESET}"
    echo
    echo "Installed binaries:"
    echo "  - $BINDIR/meridian       (AI & Controller CLI)"
    echo "  - $BINDIR/meridian-shell (Interactive Shell with Job Control)"
    echo "  - $BINDIR/meridian_demo  (Headless PTY + VT Engine Demo)"
    echo

    # Check if BINDIR is in PATH
    if [[ ":$PATH:" != *":$BINDIR:"* ]]; then
        log_warn "$BINDIR is not in your current PATH."
        echo "Add it to your environment by adding this line to ~/.bashrc or ~/.zshrc:"
        echo -e "  ${BOLD}export PATH=\"$BINDIR:\$PATH\"${RESET}"
        echo
    fi

    echo "Quick start:"
    echo "  meridian-shell             # Launch interactive shell"
    echo "  meridian ai status         # Inspect local AI status"
    echo "  meridian ai on             # Turn on local typo & risk detection"
    echo "  meridian ai analyze \"sl\"   # Try typo correction"
    echo
}

main "$@"
