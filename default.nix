# Non-flake entry point:
#   nix-build                      # builds ./result/bin/zephyr-terminal
#   nix-env -f . -i                # installs into your user profile
{
  pkgs ? import <nixpkgs> { },
}:
pkgs.callPackage ./nix/package.nix { }
