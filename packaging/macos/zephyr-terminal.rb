class ZephyrTerminal < Formula
  desc "Modern Terminal & Unified Developer Environment"
  homepage "https://github.com/charanbalaji2005/Meridian-Shell"
  url "https://github.com/charanbalaji2005/Meridian-Shell/archive/refs/tags/v2.5.0.tar.gz"
  license "GPL-3.0-or-later"

  depends_on "cmake" => :build

  def install
    system "make", "all"
    bin.install "build/zephyr-shell"
    bin.install "build/zephyr"
  end

  test do
    system "#{bin}/zephyr-shell", "-c", "echo 'Zephyr Terminal Live'"
  end
end
