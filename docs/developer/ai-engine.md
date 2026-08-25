---
layout: default
title: "AI Engine"
category: "DEVELOPER"
status: "experimental"
---

<h2 id="intent-engine-dev">Intent Engine (meridian ask)</h2>
<pre><code class="language-bash">meridian ask "find all log files modified in the last 24 hours"
# -> find . -name "*.log" -mtime -1</code></pre>

<h2 id="diagnostics-dev">Compiler & Runtime Diagnostics (meridian diag)</h2>
<pre><code class="language-bash">meridian diag "g++: error: cannot find -lutil"
# -> Diagnosis: Missing POSIX utility library. Fix: install glibc-devel / libutil.</code></pre>

<h2 id="agent-dev">Autonomous Coding Agent (meridian agent)</h2>
<pre><code class="language-bash">meridian agent "add unit tests for OSC 8 hyperlinks"</code></pre>