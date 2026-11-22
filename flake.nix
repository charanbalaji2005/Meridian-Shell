{
  description = "Zephyr Terminal — fast, modern Linux terminal emulator";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: rec {
        zephyr-terminal = pkgs.callPackage ./nix/package.nix { };
        default = zephyr-terminal;
      });

      apps = forAllSystems (pkgs: {
        default = {
          type = "app";
          program = "${self.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/zephyr-terminal";
        };
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.stdenv.hostPlatform.system}.default ];
          packages = with pkgs; [
            gdb
            clang-tools # clang-format / clangd
            desktop-file-utils # desktop-file-validate
            appstream # appstreamcli validate
          ];
          shellHook = ''
            echo "Zephyr dev shell: cmake -B build -G Ninja && cmake --build build && ctest --test-dir build"
          '';
        };
      });

      # `nix flake check` builds the package. The C++ test suite is not run in
      # the sandbox by default (see nix/package.nix); CI runs it separately.
      checks = forAllSystems (pkgs: {
        package = self.packages.${pkgs.stdenv.hostPlatform.system}.default;
      });

      overlays.default = final: _prev: {
        zephyr-terminal = final.callPackage ./nix/package.nix { };
      };

      nixosModules.default = import ./nix/module.nix self;

      formatter = forAllSystems (pkgs: pkgs.nixfmt-rfc-style);
    };
}
