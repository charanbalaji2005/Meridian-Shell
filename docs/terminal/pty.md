---
layout: default
title: "PTY"
category: "TERMINAL"
status: "implemented"
---


<p>Meridian uses POSIX <code>openpty</code> to fork master/slave session pairs.</p>
<h2 id="signals">Signal Handling & Resizing</h2>
<p>Propagates <code>SIGWINCH</code> window resize notifications and forwards <code>SIGINT</code>, <code>SIGTSTP</code>, and <code>SIGQUIT</code> directly to foreground process groups.</p>
