---
layout: default
title: "Installation & Build"
category: "GETTING STARTED"
status: "implemented"
---


<p>Meridian provides native packages for all major Linux distributions.</p>
<h2 id="dnf-rpm">Fedora & RHEL (DNF / RPM)</h2>
<pre><code class="language-bash">sudo dnf install ./dist/meridian-terminal-2.0.0-1.fc44.x86_64.rpm</code></pre>
<h2 id="apt-deb">Debian & Ubuntu (APT / DEB)</h2>
<pre><code class="language-bash">sudo apt install ./dist/meridian-terminal_2.0.0_amd64.deb</code></pre>
<h2 id="arch-pkg">Arch Linux (Pacman / PKGBUILD)</h2>
<pre><code class="language-bash">cd packaging/arch && makepkg -si</code></pre>
<h2 id="universal-script">Universal 1-Line Installer</h2>
<pre><code class="language-bash">curl -fsSL https://raw.githubusercontent.com/charanbalaji2005/Meridian-Shell/main/install.sh | bash</code></pre>
