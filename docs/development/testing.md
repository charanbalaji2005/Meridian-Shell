---
layout: default
title: "Testing"
category: "DEVELOPMENT"
status: "implemented"
---

<h2 id="mini-test-framework">MiniTest Dependency-Free Test Harness</h2>
<p>Meridian uses an internal, dependency-free C++20 test runner (<code>tests/mini_test.hpp</code>) providing clear colorized assertion reporting.</p>

<h2 id="test-coverage-spec">Test Coverage Breakdown (133 Tests)</h2>
<ul>
  <li><code>test_screen_buffer.cpp</code>: Cell matrix, cursor movement, wrap-around.</li>
  <li><code>test_ansi_parser.cpp</code>: ANSI/VT escape sequences, SGR colors, alternate buffer.</li>
  <li><code>test_pty_manager.cpp</code>: POSIX openpty fork, signal propagation.</li>
  <li><code>test_shell_executor.cpp</code>: AST execution, pipelines, job control.</li>
  <li><code>test_advanced_protocols.cpp</code>: OSC 8 hyperlinks, OSC 52 clipboard, OSC 7 CWD, OSC 133 prompts.</li>
</ul>

<h2 id="writing-tests">Authoring New Test Cases</h2>
<pre><code class="language-cpp">TEST_CASE("OSC 8 Hyperlink Parsing") {
    vt::ScreenBuffer buffer(80, 24);
    vt::ANSIParser parser(buffer);
    parser.feed("\033]8;;https://meridian-shell.org\033\\Link\033]8;;\033\\");
    ASSERT_TRUE(buffer.get_cell(0, 0).attrs.hyperlink_id != 0);
}</code></pre>