// tests / test_terminal_features.cpp
//
// Covers the core pieces the Qt terminal window is built on: scrollback
// viewport + search, terminal modes and query replies in the parser, PTY
// lifecycle hardening, themes, profiles (incl. SSH argument safety),
// session layout serialization and the extended settings.

#include "mini_test.hpp"
#include "../src/config/paths.hpp"
#include "../src/config/profiles.hpp"
#include "../src/config/session_state.hpp"
#include "../src/config/terminal_config.hpp"
#include "../src/config/theme.hpp"
#include "../src/core/pty/pty_manager.hpp"
#include "../src/core/vt/ansi_parser.hpp"
#include "../src/core/vt/input_encoder.hpp"
#include "../src/core/vt/screen_buffer.hpp"
#include "../src/dev/package_managers.hpp"
#include "../src/dev/shell_history.hpp"

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <set>
#include <signal.h>
#include <string>
#include <unistd.h>

using namespace zephyr;

namespace {

void write_lines(vt::AnsiParser& p, int n, const std::string& prefix = "line") {
    for (int i = 0; i < n; ++i) p.feed(prefix + std::to_string(i) + "\r\n");
}

std::string tmp_path(const std::string& name) {
    return "/tmp/zephyr_tf_" + name + "_" + std::to_string(getpid());
}

} // namespace

// ---- ScreenBuffer: viewport, scrollback, search -----------------------------

MTEST(viewport_scrolls_into_history_and_clamps) {
    vt::ScreenBuffer sb(3, 20, 100);
    vt::AnsiParser p(sb);
    write_lines(p, 10); // 10 lines + cursor on an empty last row -> 8 in history
    ASSERT_EQ(sb.scrollback().size(), (std::size_t)8);
    ASSERT_EQ(sb.view_offset(), 0);

    sb.scroll_view(2);
    ASSERT_EQ(sb.view_offset(), 2);
    // Top viewport row now shows history line index 6 ("line6").
    ASSERT_EQ(sb.line_text(sb.viewport_to_line(0)), std::string("line6"));
    ASSERT_EQ(static_cast<char>(sb.visible_cell(0, 4).codepoint), '6');

    sb.scroll_view(1000);
    ASSERT_EQ(sb.view_offset(), 8); // clamped to history size
    sb.scroll_view(-1000);
    ASSERT_EQ(sb.view_offset(), 0);
}

MTEST(viewport_stays_anchored_while_output_arrives) {
    vt::ScreenBuffer sb(3, 20, 100);
    vt::AnsiParser p(sb);
    write_lines(p, 10);
    sb.scroll_view(3);
    std::string before = sb.line_text(sb.viewport_to_line(0));
    write_lines(p, 4, "new");
    ASSERT_EQ(sb.line_text(sb.viewport_to_line(0)), before);
    ASSERT_EQ(sb.view_offset(), 7);
}

MTEST(scrollback_limit_and_clear) {
    vt::ScreenBuffer sb(2, 10, 5);
    vt::AnsiParser p(sb);
    write_lines(p, 20);
    ASSERT_EQ(sb.scrollback().size(), (std::size_t)5);
    sb.set_scrollback_limit(2);
    ASSERT_EQ(sb.scrollback().size(), (std::size_t)2);
    sb.scroll_view(2);
    sb.clear_scrollback();
    ASSERT_EQ(sb.scrollback().size(), (std::size_t)0);
    ASSERT_EQ(sb.view_offset(), 0);

    vt::ScreenBuffer none(2, 10, 0); // scrollback disabled
    vt::AnsiParser p2(none);
    write_lines(p2, 6);
    ASSERT_EQ(none.scrollback().size(), (std::size_t)0);
}

MTEST(search_finds_matches_across_history_case_insensitively) {
    vt::ScreenBuffer sb(3, 30, 100);
    vt::AnsiParser p(sb);
    p.feed("Error: disk full\r\nok\r\nanother ERROR here error\r\nlast\r\n");
    auto hits = sb.find_all("error", false);
    ASSERT_EQ(hits.size(), (std::size_t)3);
    ASSERT_EQ(hits[0].line, 0);
    ASSERT_EQ(hits[0].col, 0);
    ASSERT_EQ(hits[0].length, 5);
    ASSERT_EQ(hits[1].col, 8);
    ASSERT_EQ(hits[2].col, 19);

    auto exact = sb.find_all("ERROR", true);
    ASSERT_EQ(exact.size(), (std::size_t)1);
    ASSERT_TRUE(sb.find_all("", false).empty());
}

MTEST(search_reports_wide_glyph_cell_span) {
    vt::ScreenBuffer sb(2, 20, 10);
    vt::AnsiParser p(sb);
    p.feed("ab\xe4\xb8\xad" "cd"); // a b 中 c d  (中 is two cells wide)
    auto hits = sb.find_all("\xe4\xb8\xad" "c", true);
    ASSERT_EQ(hits.size(), (std::size_t)1);
    ASSERT_EQ(hits[0].col, 2);
    ASSERT_EQ(hits[0].length, 3); // 2 cells for 中 + 1 for c
}

MTEST(text_range_spans_lines_and_trims) {
    vt::ScreenBuffer sb(3, 10, 10);
    vt::AnsiParser p(sb);
    p.feed("hello\r\nworld\r\n");
    ASSERT_EQ(sb.text_range(0, 2, 1, 2), std::string("llo\nwor"));
    ASSERT_EQ(sb.text_range(1, 2, 0, 2), std::string("llo\nwor")); // reversed endpoints
}

MTEST(alt_screen_restores_cursor_and_disables_history_view) {
    vt::ScreenBuffer sb(4, 10, 10);
    vt::AnsiParser p(sb);
    write_lines(p, 6);
    p.feed("ab");
    int row = sb.cursor_row(), col = sb.cursor_col();
    p.feed("\x1b[?1049h");
    sb.scroll_view(3);
    ASSERT_EQ(sb.view_offset(), 0);
    p.feed("\x1b[3;3Hxyz\x1b[?1049l");
    ASSERT_EQ(sb.cursor_row(), row);
    ASSERT_EQ(sb.cursor_col(), col);
}

MTEST(reverse_index_scrolls_down_at_top_margin) {
    vt::ScreenBuffer sb(3, 10, 10);
    vt::AnsiParser p(sb);
    p.feed("top\r\nmid\r\nbot");
    p.feed("\x1b[1;1H\x1bM");
    ASSERT_EQ(sb.dump_row_text(0), std::string(""));
    ASSERT_EQ(sb.dump_row_text(1), std::string("top"));
    ASSERT_EQ(sb.dump_row_text(2), std::string("mid"));
}

// ---- AnsiParser: modes and replies -----------------------------------------

MTEST(parser_tracks_cursor_visibility_app_keys_and_bracketed_paste) {
    vt::ScreenBuffer sb(5, 20);
    vt::AnsiParser p(sb);
    ASSERT_TRUE(sb.modes().cursor_visible);
    p.feed("\x1b[?25l\x1b[?1h\x1b[?2004h");
    ASSERT_FALSE(sb.modes().cursor_visible);
    ASSERT_TRUE(sb.modes().application_cursor_keys);
    ASSERT_TRUE(sb.modes().bracketed_paste);
    p.feed("\x1b[?25h\x1b[?1l\x1b[?2004l");
    ASSERT_TRUE(sb.modes().cursor_visible);
    ASSERT_FALSE(sb.modes().application_cursor_keys);
    ASSERT_FALSE(sb.modes().bracketed_paste);
}

MTEST(parser_applies_decscusr_cursor_shapes) {
    vt::ScreenBuffer sb(5, 20);
    vt::AnsiParser p(sb);
    p.feed("\x1b[6 q");
    ASSERT_TRUE(sb.modes().cursor_shape == vt::AppCursorShape::Bar);
    ASSERT_FALSE(sb.modes().cursor_blink);
    p.feed("\x1b[3 q");
    ASSERT_TRUE(sb.modes().cursor_shape == vt::AppCursorShape::Underline);
    ASSERT_TRUE(sb.modes().cursor_blink);
    p.feed("\x1b[0 q");
    ASSERT_TRUE(sb.modes().cursor_shape == vt::AppCursorShape::Default);
    // The 'q' with an intermediate must not be treated as anything else.
    ASSERT_EQ(sb.dump_row_text(0), std::string(""));
}

MTEST(parser_answers_cursor_position_and_device_attribute_queries) {
    vt::ScreenBuffer sb(10, 40);
    vt::AnsiParser p(sb);
    p.feed("\x1b[4;7H\x1b[6n");
    ASSERT_EQ(p.take_responses(), std::string("\x1b[4;7R"));
    ASSERT_FALSE(p.has_responses());
    p.feed("\x1b[5n\x1b[c");
    ASSERT_EQ(p.take_responses(), std::string("\x1b[0n\x1b[?62;22c"));
}

MTEST(parser_supports_absolute_column_row_and_erase_chars) {
    vt::ScreenBuffer sb(5, 20);
    vt::AnsiParser p(sb);
    p.feed("abcdefgh\x1b[3G");
    ASSERT_EQ(sb.cursor_col(), 2);
    p.feed("\x1b[3X");
    ASSERT_EQ(sb.dump_row_text(0), std::string("ab   fgh"));
    p.feed("\x1b[4d");
    ASSERT_EQ(sb.cursor_row(), 3);
    ASSERT_EQ(sb.cursor_col(), 2);
}

// ---- PtyManager lifecycle hardening ---------------------------------------

MTEST(pty_resolves_bare_program_names_via_path) {
    const char* path = std::getenv("PATH");
    std::string sh = pty::PtyManager::resolve_executable("sh", path ? path : "/bin:/usr/bin");
    ASSERT_FALSE(sh.empty());
    ASSERT_TRUE(pty::PtyManager::resolve_executable("zephyr-definitely-not-a-program", "/bin:/usr/bin").empty());
    ASSERT_TRUE(pty::PtyManager::resolve_executable("/nonexistent/abs/path", "").empty());
}

MTEST(pty_spawn_reports_missing_program_instead_of_forking) {
    pty::PtyManager m;
    pty::SpawnOptions o;
    o.program = "zephyr-definitely-not-a-program";
    ASSERT_FALSE(m.spawn(o));
    ASSERT_TRUE(m.last_error().find("not found") != std::string::npos);
    ASSERT_FALSE(m.is_running());
}

MTEST(pty_master_is_close_on_exec_and_terminate_reaps_child) {
    pty::PtyManager m;
    pty::SpawnOptions o;
    o.program = "sleep"; // bare name: exercises PATH resolution too
    o.args = {"100"};
    ASSERT_TRUE(m.spawn(o));
    int flags = fcntl(m.master_fd(), F_GETFD);
    ASSERT_TRUE(flags >= 0 && (flags & FD_CLOEXEC));
    pid_t pid = m.child_pid();
    m.terminate(500);
    ASSERT_FALSE(m.is_running());
    ASSERT_EQ(m.master_fd(), -1);
    // The process is gone (reaped, not a zombie we still own).
    ASSERT_TRUE(kill(pid, 0) != 0);
    m.terminate(); // idempotent
}

// ---- Themes ---------------------------------------------------------------

MTEST(theme_hex_parsing_and_lookup) {
    auto c = config::parse_hex_color("#1a2B3c");
    ASSERT_TRUE(c.has_value());
    ASSERT_EQ((int)c->r, 0x1a);
    ASSERT_EQ((int)c->g, 0x2b);
    ASSERT_EQ((int)c->b, 0x3c);
    ASSERT_EQ(config::to_hex_color(*c), std::string("#1a2b3c"));
    ASSERT_FALSE(config::parse_hex_color("#12345").has_value());
    ASSERT_FALSE(config::parse_hex_color("#zzzzzz").has_value());

    std::set<std::string> ids;
    bool has_dark = false, has_light = false;
    for (const auto& t : config::builtin_themes()) {
        ASSERT_TRUE(ids.insert(t.id).second);
        (t.dark ? has_dark : has_light) = true;
    }
    ASSERT_TRUE(has_dark && has_light);
    ASSERT_EQ(config::theme_or_default("no-such-theme", false).id, std::string("zephyr-light"));
    ASSERT_EQ(config::theme_or_default("dark-nord").id, std::string("dark-nord"));
}

// ---- Profiles & SSH safety ------------------------------------------------

MTEST(argument_splitting_round_trips) {
    std::vector<std::string> args = {"-l", "with space", "quote\"inside", "back\\slash", ""};
    auto joined = config::join_args(args);
    ASSERT_TRUE(config::split_args(joined) == args);
    auto parsed = config::split_args("  -c 'echo hi'  \"a b\" x\\ y ");
    ASSERT_EQ(parsed.size(), (std::size_t)4);
    ASSERT_EQ(parsed[1], std::string("echo hi"));
    ASSERT_EQ(parsed[2], std::string("a b"));
    ASSERT_EQ(parsed[3], std::string("x y"));
}

MTEST(ssh_profile_rejects_option_injection) {
    config::SshProfile s;
    s.name = "evil";
    s.host = "-oProxyCommand=touch /tmp/pwned";
    std::string err;
    ASSERT_FALSE(s.validate(&err));
    ASSERT_FALSE(err.empty());
    s.host = "example.com";
    s.user = "-oProxyCommand=x";
    ASSERT_FALSE(s.validate());
    s.user = "dev";
    s.port = 70000;
    ASSERT_FALSE(s.validate());
    s.port = 2222;
    ASSERT_TRUE(s.validate());

    auto args = s.ssh_args();
    ASSERT_EQ(args.back(), std::string("dev@example.com"));
    ASSERT_EQ(args[args.size() - 2], std::string("--"));
    ASSERT_EQ(args[0], std::string("-p"));
    ASSERT_EQ(args[1], std::string("2222"));
}

MTEST(profile_store_round_trips_and_drops_invalid_ssh) {
    std::string path = tmp_path("profiles");
    std::remove(path.c_str());
    {
        config::ProfileStore store(path);
        config::Profile p;
        p.name = "Zsh login";
        p.program = "/bin/zsh";
        p.args = {"-l"};
        p.cwd = "/tmp";
        p.env = {{"EDITOR", "nvim"}, {"GREETING", "hello world"}};
        config::SshProfile s;
        s.name = "box";
        s.host = "10.0.0.2";
        s.user = "root";
        store.set_profiles({p});
        store.set_ssh_profiles({s});
        ASSERT_TRUE(store.save());
    }
    {
        // Simulate a hand-edited / tampered entry.
        FILE* f = std::fopen(path.c_str(), "a");
        std::fputs("ssh.count = \"2\"\nssh.1.name = \"bad\"\nssh.1.host = \"-oProxyCommand=x\"\n", f);
        std::fclose(f);
    }
    {
        config::ProfileStore store(path);
        ASSERT_TRUE(store.load());
        ASSERT_EQ(store.profiles().size(), (std::size_t)1);
        const auto* p = store.find("Zsh login");
        ASSERT_TRUE(p != nullptr);
        if (p) {
            ASSERT_TRUE(p->args == std::vector<std::string>{"-l"});
            ASSERT_EQ(p->env.size(), (std::size_t)2);
            ASSERT_EQ(p->env[1].value, std::string("hello world"));
        }
        ASSERT_EQ(store.ssh_profiles().size(), (std::size_t)1);
        ASSERT_TRUE(store.find_ssh("bad") == nullptr);
    }
    std::remove(path.c_str());

    config::ProfileStore empty(tmp_path("missing_profiles"));
    ASSERT_EQ(empty.profiles().size(), (std::size_t)1); // always has "Default"
    ASSERT_EQ(empty.profiles()[0].name, std::string("Default"));
    ASSERT_FALSE(config::valid_env_name("1BAD"));
    ASSERT_FALSE(config::valid_env_name("A-B"));
    ASSERT_TRUE(config::valid_env_name("_OK1"));
}

// ---- Session layout ---------------------------------------------------------

MTEST(session_layout_serialization_round_trips) {
    config::PaneLayout root;
    root.is_split = true;
    root.side_by_side = true;
    root.sizes = {60, 40};
    config::PaneLayout right;
    right.is_split = true;
    right.children = {config::PaneLayout::leaf("Default", "/tmp/a,b"),
                      config::PaneLayout::leaf("ssh:box", "/home/me/(odd)|dir %x")};
    root.children = {config::PaneLayout::leaf("Default", "/home/me"), right};

    std::string text = config::serialize_layout(root);
    config::PaneLayout back;
    ASSERT_TRUE(config::parse_layout(text, back));
    ASSERT_EQ(config::serialize_layout(back), text);
    ASSERT_EQ(back.leaf_count(), 3);
    ASSERT_EQ(back.children[1].children[1].cwd, std::string("/home/me/(odd)|dir %x"));
    ASSERT_TRUE(back.sizes == std::vector<int>({60, 40}));
}

MTEST(session_layout_rejects_malformed_input) {
    config::PaneLayout out;
    ASSERT_FALSE(config::parse_layout("", out));
    ASSERT_FALSE(config::parse_layout("P(a|b", out));
    ASSERT_FALSE(config::parse_layout("S1(P(a|b))", out));          // split needs 2+ children
    ASSERT_FALSE(config::parse_layout("P(a|b)trailing", out));
    ASSERT_FALSE(config::parse_layout("P(a%G1|b)", out));            // bad escape
    std::string deep;
    for (int i = 0; i < 40; ++i) deep += "S0(";
    ASSERT_FALSE(config::parse_layout(deep, out));                   // depth limit, no stack overflow
}

MTEST(session_store_round_trips_tabs) {
    std::string path = tmp_path("session");
    config::SessionStore store(path);
    config::SessionState st;
    st.tabs.push_back({"build", config::PaneLayout::leaf("Default", "/tmp")});
    st.tabs.push_back({"logs & stuff", config::PaneLayout::leaf("Default", "/var/log")});
    st.active_tab = 1;
    ASSERT_TRUE(store.save(st));
    config::SessionState back;
    ASSERT_TRUE(store.load(back));
    ASSERT_EQ(back.tabs.size(), (std::size_t)2);
    ASSERT_EQ(back.tabs[1].title, std::string("logs & stuff"));
    ASSERT_EQ(back.active_tab, 1);
    ASSERT_TRUE(store.clear());
    ASSERT_FALSE(store.load(back));
}

// ---- Settings ---------------------------------------------------------------

MTEST(extended_settings_round_trip_and_sanitize) {
    std::string path = tmp_path("settings");
    std::remove(path.c_str());
    {
        config::TerminalConfig cfg(path);
        auto s = cfg.get();
        s.theme_name = "gruvbox-dark";
        s.ui_mode = "light";
        s.accent_color = "#ff8800";
        s.window_opacity = 85;
        s.font_ligatures = false;
        s.reduced_motion = true;
        s.restore_session = false;
        s.max_fps = 120;
        s.cursor_style = "bar";
        cfg.set(s);
        ASSERT_TRUE(cfg.save());
    }
    {
        config::TerminalConfig cfg(path);
        ASSERT_TRUE(cfg.load());
        auto s = cfg.get();
        ASSERT_EQ(s.theme_name, std::string("gruvbox-dark"));
        ASSERT_EQ(s.ui_mode, std::string("light"));
        ASSERT_EQ(s.accent_color, std::string("#ff8800"));
        ASSERT_EQ(s.window_opacity, 85);
        ASSERT_FALSE(s.font_ligatures);
        ASSERT_TRUE(s.reduced_motion);
        ASSERT_FALSE(s.restore_session);
        ASSERT_EQ(s.max_fps, 120);
        ASSERT_EQ(s.cursor_style, std::string("bar"));
    }
    std::remove(path.c_str());

    config::TerminalSettings bad;
    bad.window_opacity = 5;
    bad.font_size = 500;
    bad.max_fps = 75;
    bad.cursor_style = "triangle";
    bad.accent_color = "red";
    bad.ui_mode = "neon";
    bad.sanitize();
    ASSERT_EQ(bad.window_opacity, 30);
    ASSERT_EQ(bad.font_size, 72);
    ASSERT_EQ(bad.max_fps, 60);
    ASSERT_EQ(bad.cursor_style, std::string("block"));
    ASSERT_EQ(bad.accent_color, std::string("#8b7cf6"));
    ASSERT_EQ(bad.ui_mode, std::string("dark"));
}

MTEST(config_dir_honours_override_and_xdg) {
    const char* old_override = std::getenv("ZEPHYR_CONFIG_HOME");
    std::string saved = old_override ? old_override : "";
    const char* old_xdg = std::getenv("XDG_CONFIG_HOME");
    std::string saved_xdg = old_xdg ? old_xdg : "";
    setenv("ZEPHYR_CONFIG_HOME", "/tmp/zephyr-cfg-test", 1);
    ASSERT_EQ(config::config_dir(), std::string("/tmp/zephyr-cfg-test"));
    unsetenv("ZEPHYR_CONFIG_HOME");
    setenv("XDG_CONFIG_HOME", "/tmp/xdg-test", 1);
    ASSERT_EQ(config::config_file("terminal.toml"), std::string("/tmp/xdg-test/zephyr/terminal.toml"));
    unsetenv("XDG_CONFIG_HOME");
    if (!saved.empty()) setenv("ZEPHYR_CONFIG_HOME", saved.c_str(), 1);
    if (!saved_xdg.empty()) setenv("XDG_CONFIG_HOME", saved_xdg.c_str(), 1);
}

// ---- Input encoding & paste safety -----------------------------------------

MTEST(key_encoding_follows_xterm_conventions) {
    using vt::Key;
    vt::KeyMods none;
    vt::KeyMods ctrl; ctrl.ctrl = true;
    vt::KeyMods shift; shift.shift = true;
    ASSERT_EQ(vt::encode_key(Key::Up, none, false), std::string("\x1b[A"));
    ASSERT_EQ(vt::encode_key(Key::Up, none, true), std::string("\x1bOA"));   // DECCKM
    ASSERT_EQ(vt::encode_key(Key::Right, ctrl, true), std::string("\x1b[1;5C"));
    ASSERT_EQ(vt::encode_key(Key::Delete, none, false), std::string("\x1b[3~"));
    ASSERT_EQ(vt::encode_key(Key::PageUp, shift, false), std::string("\x1b[5;2~"));
    ASSERT_EQ(vt::encode_key(Key::F1, none, false), std::string("\x1bOP"));
    ASSERT_EQ(vt::encode_key(Key::F5, none, false), std::string("\x1b[15~"));
    ASSERT_EQ(vt::encode_key(Key::F12, ctrl, false), std::string("\x1b[24;5~"));
    ASSERT_EQ(vt::encode_key(Key::Backspace, none, false), std::string("\x7f"));
    ASSERT_EQ(vt::encode_key(Key::Tab, shift, false), std::string("\x1b[Z"));
    ASSERT_EQ(vt::encode_key(Key::Enter, none, false), std::string("\r"));
}

MTEST(text_encoding_maps_ctrl_and_alt) {
    vt::KeyMods ctrl; ctrl.ctrl = true;
    vt::KeyMods alt; alt.alt = true;
    ASSERT_EQ(vt::encode_text("c", ctrl), std::string("\x03"));
    ASSERT_EQ(vt::encode_text("C", ctrl), std::string("\x03"));
    ASSERT_EQ(vt::encode_text(" ", ctrl), std::string(1, '\0'));
    ASSERT_EQ(vt::encode_text("[", ctrl), std::string("\x1b"));
    ASSERT_EQ(vt::encode_text("/", ctrl), std::string("\x1f"));
    ASSERT_EQ(vt::encode_text("b", alt), std::string("\x1b" "b"));
    ASSERT_EQ(vt::encode_text("\xc3\xa9", vt::KeyMods{}), std::string("\xc3\xa9")); // é passes through
}

MTEST(paste_strips_escape_injection_and_brackets) {
    // A malicious clipboard trying to end bracketed paste and run a command.
    std::string evil = "ls\x1b[201~rm -rf ~\r\n";
    std::string out = vt::prepare_paste(evil, true);
    ASSERT_EQ(out, std::string("\x1b[200~ls[201~rm -rf ~\r\x1b[201~"));
    // Exactly one closing marker: the one we added.
    ASSERT_EQ(out.find("\x1b[201~"), out.size() - 6);

    ASSERT_EQ(vt::prepare_paste("a\r\nb\nc", false), std::string("a\rb\rc"));
    ASSERT_EQ(vt::prepare_paste("tab\there\x07", false), std::string("tab\there"));
    ASSERT_TRUE(vt::is_multiline_paste("echo a\necho b"));
    ASSERT_FALSE(vt::is_multiline_paste("echo a\n"));
    ASSERT_FALSE(vt::is_multiline_paste("single"));
}

// ---- Shell history parsing, quoting, package managers -----------------------

MTEST(history_parsers_handle_each_shell_format) {
    auto bash = dev::parse_bash_history("#1700000000\nls -la\n\ngit status\n");
    ASSERT_TRUE(bash == std::vector<std::string>({"ls -la", "git status"}));

    // EXTENDED_HISTORY, a multi-line entry, and a metafied byte (0x83 0xA3 -> 0x83).
    std::string zsh = ": 1700000000:0;make\n: 1700000001:2;for f in *; do\\\necho $f\\\ndone\nplain\n";
    zsh += "echo \x83\xa3\n";
    auto z = dev::parse_zsh_history(zsh);
    ASSERT_EQ(z.size(), (std::size_t)4);
    ASSERT_EQ(z[0], std::string("make"));
    ASSERT_EQ(z[1], std::string("for f in *; do\necho $f\ndone"));
    ASSERT_EQ(z[2], std::string("plain"));
    ASSERT_EQ(z[3], std::string("echo \x83"));

    auto fish = dev::parse_fish_history("- cmd: cargo build\n  when: 1700000000\n- cmd: echo a\\nb\n");
    ASSERT_TRUE(fish == std::vector<std::string>({"cargo build", "echo a\nb"}));

    auto d = dev::dedupe_recent_first({"a", "b", "a", "c", "b"}, 10);
    ASSERT_TRUE(d == std::vector<std::string>({"b", "c", "a"}));
    ASSERT_EQ(dev::dedupe_recent_first({"a", "b", "c"}, 2).size(), (std::size_t)2);
}

MTEST(shell_quote_is_injection_safe) {
    ASSERT_EQ(dev::shell_quote("/usr/bin/simple-name.txt"), std::string("/usr/bin/simple-name.txt"));
    ASSERT_EQ(dev::shell_quote("my file"), std::string("'my file'"));
    ASSERT_EQ(dev::shell_quote("it's; rm -rf ~"), std::string("'it'\\''s; rm -rf ~'"));
    ASSERT_EQ(dev::shell_quote("$(whoami)"), std::string("'$(whoami)'"));
    ASSERT_EQ(dev::shell_quote(""), std::string("''"));
}

MTEST(package_manager_detection_and_validation) {
    auto fedora = dev::parse_os_release("NAME=\"Fedora Linux\"\nID=fedora\nVERSION_ID=41\n");
    ASSERT_EQ(fedora["NAME"], std::string("Fedora Linux"));
    ASSERT_TRUE(dev::package_managers_for_distro(fedora) == std::vector<std::string>({"dnf"}));
    auto ubuntu = dev::parse_os_release("ID=ubuntu\nID_LIKE=debian\n");
    ASSERT_TRUE(dev::package_managers_for_distro(ubuntu) == std::vector<std::string>({"apt"}));
    auto manjaro = dev::parse_os_release("ID=manjaro\nID_LIKE=arch\n");
    ASSERT_TRUE(dev::package_managers_for_distro(manjaro) == std::vector<std::string>({"pacman"}));
    auto nixos = dev::parse_os_release("ID=nixos\n");
    ASSERT_TRUE(dev::package_managers_for_distro(nixos) == std::vector<std::string>({"nix"}));

    ASSERT_TRUE(dev::valid_package_name("gcc-c++"));
    ASSERT_TRUE(dev::valid_package_name("python3.12"));
    ASSERT_FALSE(dev::valid_package_name("-oDebug"));
    ASSERT_FALSE(dev::valid_package_name("vim; rm -rf ~"));
    ASSERT_FALSE(dev::valid_package_name("$(id)"));
    const auto* dnf = dev::find_package_manager("dnf");
    ASSERT_TRUE(dnf != nullptr);
    if (dnf) {
        ASSERT_EQ(dev::package_command(dnf->install, "htop"), std::string("sudo dnf install htop"));
        ASSERT_EQ(dev::package_command(dnf->install, "bad name"), std::string(""));
    }
    const auto* nix = dev::find_package_manager("nix");
    if (nix) ASSERT_EQ(dev::package_command(nix->install, "htop"), std::string("nix profile install nixpkgs#htop"));
}

// ---- Window style / Neon Night preset ----------------------------------------

MTEST(window_style_settings_round_trip_and_preset) {
    std::string path = tmp_path("winstyle");
    std::remove(path.c_str());
    {
        config::TerminalConfig cfg(path);
        auto s = config::neon_night_preset(cfg.get());
        s.font_family = "JetBrains Mono";
        s.frameless_window = true;
        cfg.set(s);
        ASSERT_TRUE(cfg.save());
    }
    {
        config::TerminalConfig cfg(path);
        ASSERT_TRUE(cfg.load());
        auto s = cfg.get();
        ASSERT_TRUE(s.glow_border);
        ASSERT_TRUE(s.minimal_chrome);
        ASSERT_TRUE(s.frameless_window);
        ASSERT_EQ(s.theme_name, std::string("neon-night"));
        ASSERT_EQ(s.glow_color, std::string("#ff5fd2"));
        ASSERT_EQ(s.corner_radius, 14);
        ASSERT_EQ(s.font_family, std::string("JetBrains Mono")); // preset keeps personal choices
    }
    std::remove(path.c_str());
    ASSERT_TRUE(config::find_theme("neon-night") != nullptr);

    config::TerminalSettings bad;
    bad.corner_radius = 99;
    bad.glow_color = "pink";
    bad.sanitize();
    ASSERT_EQ(bad.corner_radius, 32);
    ASSERT_EQ(bad.glow_color, std::string("#ff5fd2"));
}
