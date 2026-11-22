# Build & validation report — Zephyr Terminal 2.6.0

This report separates what was **actually run** while preparing 2.6.0 from
what is **only verified by CI on real Linux**. No package format below has
been built yet; they are wired into CI so the first push produces real
results.

## Environment used

The work was done on Windows 11 without WSL, Docker, Nix or a Linux VM.
Verification used MSYS2:

- **MSYS (Cygwin) GCC 16.2**: a POSIX environment with real `openpty`/`fork`,
  used to build and run the core, CLI, shell and the whole test suite.
  `-D_GNU_SOURCE` is added locally because Cygwin hides POSIX declarations
  under strict `-std=c++20`, which glibc does not.
- **UCRT64 GCC 16.2 + Qt 6.12** (`qtbase`, `qtsvg`, `moc`, `rcc`, CMake 4.5):
  used to type-check the Qt GUI. It cannot link or run the GUI, because the
  PTY layer is POSIX-only.

## Verified locally

| Check | Result |
|---|---|
| Makefile build of all targets (`zephyr`, `zephyr-shell`, `zephyr_demo`, `zephyr_tests`) | ✅ builds |
| CMake build of all non-GUI targets with `-DZEPHYR_WARNINGS_AS_ERRORS=ON` | ✅ builds, 0 warnings |
| Test suite (`zephyr_tests`), via the Makefile build and via CTest | 303 tests / 2,353 assertions: **298 pass, 5 fail — all 5 are environment-specific, see below** |
| `tests/manual_core_test.sh` against the built `zephyr-shell` (direct and via CTest) | ✅ 11/11 checks |
| New tests (`test_terminal_features.cpp`, additions to `test_config.cpp`, `test_advanced_protocols.cpp`) | ✅ all pass |
| Every `src/gui/*.cpp` and `src/app/gui_main.cpp` compiled `-fsyntax-only -Wall -Wextra` against Qt 6.12, with `QT_NO_CAST_FROM_BYTEARRAY` and `QT_NO_URL_CAST_FROM_STRING` | ✅ 0 errors, 0 warnings |
| `moc` on every `Q_OBJECT` header, generated code compiled | ✅ |
| `rcc` on `resources/zephyr.qrc` (46 resources), generated code compiled | ✅ |
| CMake configure with `-DZEPHYR_BUILD_GUI=ON` finds Qt and sets up AUTOMOC/AUTORCC | ✅ (Qt 6.12.0) |
| All GitHub workflow files parse as YAML | ✅ |
| All shell scripts pass `bash -n` | ✅ |
| `scripts/make_source_tarball.sh` produces a complete tarball (582 files, no build output) | ✅ (non-git fallback path) |

### The 5 environment-specific failures

The first four fail identically on the unmodified code in the same
environment. The Ctrl+Z test also failed on the unmodified code, but that
run used the wrong shell path, so it isn't a clean comparison. Because
2.6.0 also changes how the PTY child resets its signal mask before
`exec`, treat this test as **unconfirmed until it passes in Linux CI**.

| Test | Why it fails here |
|---|---|
| `credential_store_file_has_owner_only_permissions` | NTFS ACLs don't map to POSIX mode `0600` |
| `analyzer_suggests_correction_for_transposed_typo`, `analyzer_respects_explicit_path_override`, `controller_analyze_detects_typo_and_risk_when_enabled` | They look for `grep` on `$PATH`; Cygwin binaries are named `grep.exe` |
| `job_control_ctrl_z_stops_foreground_job_via_real_pty` | Cygwin's PTY doesn't deliver `SIGTSTP` for Ctrl+Z like Linux |

`job_control_disabled_for_noninteractive_shell` *looked* like a sixth
environment failure. It was actually caused by the test hard-coding
`./build/zephyr-shell`; that is now overridable (`ZEPHYR_SHELL_BIN`) and
the test passes.

## Not verified locally — verified by CI

`.github/workflows/ci.yml` and `packages.yml` cover these. Their status is
**unknown until the workflows run**:

| Item | CI job | What it checks |
|---|---|---|
| GUI links and runs on Linux | `ci / build-test` (Ubuntu 24.04, Qt 6.4) | Full CMake build with `-Werror`; CTest; headless `zephyr-terminal --version`; **end-to-end `zephyr-terminal -e sh -c 'echo ok'`** (PTY spawn → exit → pane/tab/window close → process exits) |
| Sanitizers | `ci / sanitizers` | ASan + UBSan + leak detection over the test suite |
| Fedora RPM | `packages / rpm` | `rpmbuild` incl. `%check`, then `dnf install` and run |
| Debian 12 / Ubuntu 24.04 .deb | `packages / deb` | `dpkg-buildpackage` incl. tests, then `apt install` and run; lintian |
| Arch package | `packages / arch` | `makepkg` incl. `check()`, then `pacman -U` and run |
| Nix flake | `packages / nix` | `nix flake check`, `nix build`, run, `nix profile install`, `doCheck` build |
| AppImage | `packages / appimage` | linuxdeploy bundle, `--version`, end-to-end `-e` run |

Things I expect CI may surface: compiler warnings that only newer GCC or
Clang emit (CI uses `-Werror`), and the interaction of the Nix sandbox with
the PTY tests in the `doCheck` build.

## Not verified at all

- **Visual result and interaction.** The GUI has never been displayed: layout,
  colors, animation feel, HiDPI rendering, Wayland vs X11 behavior, IME, and
  real-world terminal apps (vim, htop, tmux, fzf) need a manual pass on a Linux
  desktop. README screenshots are pending for the same reason.
- **Icon fidelity.** The UI icons follow Lucide's 24×24 geometry, but the path
  data was transcribed by hand. Diff them against `lucide-static` before
  release.
- **`flake.lock`** is not committed; it can only be generated with Nix. CI
  uploads one as an artifact; commit it to pin nixpkgs.
- **Legacy packaging** — `flatpak/`, `snap/`, `macos/`, `windows/` — was left
  untouched and unverified.

## Manual test checklist (Linux desktop)

1. Launch from the app menu; confirm the window, icon and dock entry (Wayland and X11).
2. Type, use Tab completion, Ctrl+C, Ctrl+Z/`fg`, arrows in `vim`/`less`, mouse-wheel in `less`.
3. Split side-by-side and top/bottom, nest splits, close panes, `exit` a shell.
4. Search (Ctrl+Shift+F) across long output; copy/paste; paste multi-line text into `sh`.
5. Change theme, mode, accent, opacity and font in Settings; restart; settings persist.
6. Create a profile with env vars and an SSH host; open both from the sidebar and palette.
7. Quit with several split tabs, relaunch, and confirm the layout is restored.
8. Turn on *Reduce motion* and confirm transitions become instant.
