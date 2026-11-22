cask "zephyr-terminal" do
  version "2.5.0"
  sha256 :no_check

  url "https://github.com/charanbalaji2005/Meridian-Shell/releases/download/v#{version}/Zephyr-Terminal-#{version}-macOS-arm64.dmg",
      verified: "github.com/charanbalaji2005/Meridian-Shell/"
  name "Zephyr Terminal"
  desc "GPU-accelerated terminal emulator, unified developer environment, and AI platform"
  homepage "https://github.com/charanbalaji2005/Meridian-Shell"

  livecheck do
    url :url
    strategy :github_latest
  end

  auto_updates true
  depends_on macos: ">= :big_sur"

  app "Zephyr.app"
  binary "#{appdir}/Zephyr.app/Contents/MacOS/zephyr", target: "zephyr"
  binary "#{appdir}/Zephyr.app/Contents/MacOS/zephyr-shell", target: "zephyr-shell"

  zap trash: [
    "~/.config/zephyr",
    "~/Library/Application Support/Zephyr",
    "~/Library/Preferences/org.zephyr-terminal.ZephyrTerminal.plist",
    "~/Library/Saved Application State/org.zephyr-terminal.ZephyrTerminal.savedState",
  ]
end

