module.exports = function(addArt) {
  addArt('development-building', 'Building', 'DEVELOPMENT', 'implemented',
    'Compiling Meridian Terminal with C++20, Make, and dependencies.',
    [
      { id: 'prerequisites', text: 'Compiler & Library Prerequisites', level: 2 },
      { id: 'make-targets', text: 'Make Targets & Build Commands', level: 2 },
      { id: 'custom-cxxflags', text: 'Custom Compiler Flags & Optimization', level: 2 }
    ],
    `
<h2 id="prerequisites">Compiler & Library Prerequisites</h2>
<p>Requires GCC 11+ or Clang 13+ with C++20 support, and standard POSIX <code>libutil</code>.</p>

<h2 id="make-targets">Make Targets & Build Commands</h2>
<pre><code class="language-bash">make all -j$(nproc)  # Compile all targets
make test            # Run automated test suite (133 tests)
make demo            # Run graphical demo
sudo make install    # Install binaries and assets to system
make clean           # Clean build artifacts</code></pre>

<h2 id="custom-cxxflags">Custom Compiler Flags & Optimization</h2>
<pre><code class="language-bash"># Compile with aggressive optimizations and LTO
CXXFLAGS="-std=c++20 -O3 -flto -march=native" make all -j$(nproc)</code></pre>
`
  );

  addArt('development-testing', 'Testing', 'DEVELOPMENT', 'implemented',
    'Automated test suites covering PTY, VT parser, AST executor, and protocols.',
    [
      { id: 'mini-test-framework', text: 'MiniTest Dependency-Free Test Harness', level: 2 },
      { id: 'test-coverage-spec', text: 'Test Coverage Breakdown (133 Tests)', level: 2 },
      { id: 'writing-tests', text: 'Authoring New Test Cases', level: 2 }
    ],
    `
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
    parser.feed("\\033]8;;https://meridian-shell.org\\033\\\\Link\\033]8;;\\033\\\\");
    ASSERT_TRUE(buffer.get_cell(0, 0).attrs.hyperlink_id != 0);
}</code></pre>
`
  );

  addArt('development-debugging', 'Debugging', 'DEVELOPMENT', 'implemented',
    'GDB debugging, ASan, Valgrind, and Telemetry profiler.',
    [
      { id: 'gdb-asan', text: 'GDB & AddressSanitizer', level: 2 },
      { id: 'telemetry-profiler-dev', text: 'Telemetry Profiler (meridian --performance)', level: 2 },
      { id: 'valgrind-profiling', text: 'Valgrind Memory Profiling', level: 2 }
    ],
    `
<h2 id="gdb-asan">GDB & AddressSanitizer</h2>
<pre><code class="language-bash"># Build with AddressSanitizer and debug symbols
CXXFLAGS="-std=c++20 -fsanitize=address -g -O0" make all</code></pre>

<h2 id="telemetry-profiler-dev">Telemetry Profiler (meridian --performance)</h2>
<pre><code class="language-bash">meridian --performance
# Outputs real-time FPS, frame time (ms), glyph cache hits, VRAM, and PTY latency.</code></pre>

<h2 id="valgrind-profiling">Valgrind Memory Profiling</h2>
<pre><code class="language-bash">valgrind --leak-check=full --track-origins=yes ./bin/meridian</code></pre>
`
  );

  addArt('development-contributing', 'Contributing', 'DEVELOPMENT', 'implemented',
    'How to report bugs, resolve visual glitches, add anime themes, and submit PRs.',
    [
      { id: 'contributing-guide-spec', text: 'Open Source Contribution Workflow', level: 2 },
      { id: 'code-style-guide', text: 'Code Style Guidelines (clang-format)', level: 2 }
    ],
    `
<h2 id="contributing-guide-spec">Open Source Contribution Workflow</h2>
<p>Fork the repository on GitHub, create a feature branch, run <code>make test</code> to verify zero regressions, and open a Pull Request.</p>

<h2 id="code-style-guide">Code Style Guidelines (clang-format)</h2>
<pre><code class="language-bash"># Format all C++ source files
clang-format -i src/**/*.cpp src/**/*.hpp tests/**/*.cpp</code></pre>
`
  );

  addArt('development-release', 'Release Process', 'DEVELOPMENT', 'implemented',
    'Version tagging, packaging automation, and release checklist.',
    [
      { id: 'release-flow-spec', text: 'Release & Packaging Automation', level: 2 },
      { id: 'semantic-versioning', text: 'Semantic Versioning & Changelogs', level: 2 }
    ],
    `
<h2 id="release-flow-spec">Release & Packaging Automation</h2>
<p>Automated GitHub Actions workflows package RPM, DEB, Arch PKGBUILD, and universal tarballs on version tag creation.</p>

<h2 id="semantic-versioning">Semantic Versioning & Changelogs</h2>
<p>Meridian strictly follows Semantic Versioning (MAJOR.MINOR.PATCH). Release notes are maintained in <code>docs/project/changelog.md</code>.</p>
`
  );
};
