---
layout: default
title: "Shell"
category: "TERMINAL"
status: "implemented"
---


<p>Meridian includes an internal lexer, recursive-descent parser, and AST executor.</p>
<h2 id="pipelines">Pipelines & Redirections</h2>
<pre><code class="language-bash">cat log.txt | grep -i error | wc -l > errors.txt 2>&1</code></pre>
