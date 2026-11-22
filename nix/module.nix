# NixOS module: `programs.zephyr-terminal.enable = true;`
#
# Takes the flake's `self` so the default package always matches the
# flake revision the system was built from.
self:
{
  config,
  lib,
  pkgs,
  ...
}:

let
  cfg = config.programs.zephyr-terminal;
in
{
  options.programs.zephyr-terminal = {
    enable = lib.mkEnableOption "Zephyr Terminal, a modern terminal emulator";

    package = lib.mkOption {
      type = lib.types.package;
      default = self.packages.${pkgs.stdenv.hostPlatform.system}.default;
      defaultText = lib.literalExpression "inputs.zephyr.packages.\${pkgs.system}.default";
      description = "The Zephyr Terminal package to install.";
    };

    setAsDefaultTerminal = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = ''
        Register Zephyr Terminal as the preferred terminal for
        xdg-terminal-exec (used by desktop launchers to open programs
        that need a terminal).
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    environment.systemPackages = [ cfg.package ];

    environment.etc."xdg/xdg-terminals.list" = lib.mkIf cfg.setAsDefaultTerminal {
      text = ''
        org.zephyr_terminal.ZephyrTerminal.desktop
      '';
    };
  };
}
