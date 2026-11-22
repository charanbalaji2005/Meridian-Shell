# Nix and NixOS

The repository is a Nix flake. Outputs:

| Output | Purpose |
|---|---|
| `packages.<system>.default` / `.zephyr-terminal` | The package (GUI + CLI + shell, desktop entry, icons) |
| `apps.<system>.default` | `nix run` target (`zephyr-terminal`) |
| `devShells.<system>.default` | Compiler, CMake, Ninja, Qt 6, gdb, clang-tools, validators |
| `overlays.default` | Adds `pkgs.zephyr-terminal` |
| `nixosModules.default` | `programs.zephyr-terminal` options |
| `checks.<system>.package` | Built by `nix flake check` |

Systems: `x86_64-linux`, `aarch64-linux`. Dependencies come from
`nixos-unstable`; pin them by committing `flake.lock`.

## Use it

```bash
nix run github:charanbalaji2005/Meridian-Shell                 # run without installing
nix profile install github:charanbalaji2005/Meridian-Shell     # install for your user
nix profile upgrade --all                                     # update later
```

From a checkout: `nix build` (result in `./result/bin/zephyr-terminal`),
`nix run`, `nix develop`.

Without flakes: `nix-build` or `nix-env -f . -i` (uses `default.nix` and
`<nixpkgs>` from your channel).

## NixOS

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    zephyr.url = "github:charanbalaji2005/Meridian-Shell";
    zephyr.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { nixpkgs, zephyr, ... }: {
    nixosConfigurations.myhost = nixpkgs.lib.nixosSystem {
      system = "x86_64-linux";
      modules = [
        ./configuration.nix
        zephyr.nixosModules.default
        {
          programs.zephyr-terminal.enable = true;
          # Optional: make launchers that need a terminal use Zephyr
          # (writes /etc/xdg/xdg-terminals.list for xdg-terminal-exec).
          programs.zephyr-terminal.setAsDefaultTerminal = true;
        }
      ];
    };
  };
}
```

Or with the overlay: `nixpkgs.overlays = [ zephyr.overlays.default ];` and
`environment.systemPackages = [ pkgs.zephyr-terminal ];`.

Home Manager: `home.packages = [ inputs.zephyr.packages.${pkgs.system}.default ];`.

## Package details

`nix/package.nix` builds with CMake + Ninja and `qt6.wrapQtAppsHook`, which
wraps the binaries so Qt finds its platform plugins (XCB, and Wayland via
`qt6.qtwayland`). The source is restricted with `lib.fileset` to the files
the build needs.

The C++ test suite is off inside the sandbox by default because it spawns
real PTYs and writes under `/tmp`, which not every Nix sandbox configuration
provides. Run it with:

```bash
nix build --impure --expr \
  '(builtins.getFlake (toString ./.)).packages.x86_64-linux.default.override { doCheck = true; }'
```

or in `nix develop` with `cmake -B build -G Ninja && cmake --build build && ctest --test-dir build`.

The in-app **Settings → NixOS** page detects NixOS / Nix and offers these
snippets with copy buttons.
