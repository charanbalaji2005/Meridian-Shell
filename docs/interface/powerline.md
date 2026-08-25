---
layout: default
title: "Powerline"
category: "INTERFACE"
status: "implemented"
---

<h2 id="powerline-badges">Powerline Status Badges</h2>
<p>Meridian renders hardware-accelerated Powerline glyphs (, , , 󰌽) showing time, OS icon, current working directory, and exit codes.</p>

<h2 id="git-status-segment">Real-time Git Status Segment</h2>
<p><code>dev::GitIntel</code> polls the nearest <code>.git</code> repository, displaying branch name ( main), ahead/behind status (↑1 ↓0), and dirty state (●).</p>

<h2 id="custom-segments">Configuring Custom Segments</h2>
<p>Enable or disable segments in <code>~/.config/meridian/config.json</code> under the <code>"powerline"</code> section (e.g. <code>"show_time": true</code>, <code>"show_git": true</code>, <code>"show_exit_code": true</code>).</p>