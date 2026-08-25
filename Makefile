# Meridian Terminal — Makefile
#
# Meridian 2.0 Unified Build System

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -g
LDLIBS := -lutil

BUILD := build

CORE_SRC := \
    src/core/vt/screen_buffer.cpp \
    src/core/vt/ansi_parser.cpp \
    src/core/vt/graphics.cpp \
    src/core/pty/pty_manager.cpp \
    src/core/pty/pty_session.cpp \
    src/core/config.cpp \
    src/core/terminal_image.cpp \
    src/platform/LinuxPTY.cpp \
    src/core/renderer/damage_tracker.cpp \
    src/core/renderer/glyph_atlas.cpp \
    src/core/renderer/render_pipeline.cpp

SHELL_SRC := \
    src/shell/lexer.cpp \
    src/shell/parser.cpp \
    src/shell/executor.cpp \
    src/shell/builtins.cpp \
    src/shell/line_editor.cpp \
    src/shell/shell.cpp

AI_SRC := \
    src/ai/secret_redactor.cpp \
    src/ai/risk_classifier.cpp \
    src/ai/command_analyzer.cpp \
    src/ai/command_context.cpp \
    src/ai/ai_controller.cpp \
    src/ai/intent_engine.cpp \
    src/ai/error_diagnostics.cpp \
    src/ai/ai_agent.cpp

WORKSPACE_SRC := \
    src/workspace/pane_tree.cpp \
    src/workspace/workspace_manager.cpp \
    src/workspace/session_recorder.cpp

DEV_SRC := \
    src/dev/git_intel.cpp \
    src/dev/system_monitor.cpp \
    src/dev/file_explorer.cpp \
    src/dev/command_palette.cpp \
    src/dev/universal_search.cpp \
    src/dev/rich_history.cpp

CONFIG_SRC := \
    src/config/terminal_config.cpp \
    src/config/keybindings.cpp

SECURITY_SRC := \
    src/security/credential_store.cpp

TEST_SRC := \
    tests/test_main.cpp \
    tests/test_screen_buffer.cpp \
    tests/test_ansi_parser.cpp \
    tests/test_pty_manager.cpp \
    tests/test_shell_lexer.cpp \
    tests/test_shell_parser.cpp \
    tests/test_shell_executor.cpp \
    tests/test_job_control.cpp \
    tests/test_shell_class.cpp \
    tests/test_ai.cpp \
    tests/test_command_context.cpp \
    tests/test_config.cpp \
    tests/test_security.cpp \
    tests/test_renderer.cpp \
    tests/test_workspace.cpp \
    tests/test_intent_and_diagnostics.cpp \
    tests/test_dev_tools.cpp

CORE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CORE_SRC))
SHELL_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SHELL_SRC))
AI_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(AI_SRC))
WORKSPACE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(WORKSPACE_SRC))
DEV_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(DEV_SRC))
CONFIG_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CONFIG_SRC))
SECURITY_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SECURITY_SRC))
TEST_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(TEST_SRC))

VERSION ?= 0.1.0
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
MANDIR ?= $(DATADIR)/man
DESKTOPDIR ?= $(DATADIR)/applications
ICONDIR ?= $(DATADIR)/icons/hicolor/scalable/apps
METAINFODIR ?= $(DATADIR)/metainfo
DOCDIR ?= $(DATADIR)/doc/meridian-terminal

.PHONY: all test demo shell meridian manual-test clean install uninstall tarball

all: $(BUILD)/meridian_tests $(BUILD)/meridian_demo $(BUILD)/meridian-shell $(BUILD)/meridian

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/meridian_tests: $(CORE_OBJ) $(SHELL_OBJ) $(AI_OBJ) $(WORKSPACE_OBJ) $(DEV_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ) $(TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian_demo: $(CORE_OBJ) $(BUILD)/src/app/demo_main.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian-shell: $(SHELL_OBJ) $(CORE_OBJ) $(AI_OBJ) $(DEV_OBJ) $(BUILD)/src/app/shell_main.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/meridian: $(CORE_OBJ) $(SHELL_OBJ) $(AI_OBJ) $(WORKSPACE_OBJ) $(DEV_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ) $(BUILD)/src/app/meridian_gui.o $(BUILD)/src/app/meridian_main.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

test: $(BUILD)/meridian_tests $(BUILD)/meridian-shell
	$(BUILD)/meridian_tests

manual-test: $(BUILD)/meridian-shell
	./tests/manual_core_test.sh

demo: $(BUILD)/meridian_demo
	$(BUILD)/meridian_demo

shell: $(BUILD)/meridian-shell
	$(BUILD)/meridian-shell

meridian: $(BUILD)/meridian
	$(BUILD)/meridian $(ARGS)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BUILD)/meridian $(DESTDIR)$(BINDIR)/meridian
	install -m 755 $(BUILD)/meridian-shell $(DESTDIR)$(BINDIR)/meridian-shell
	install -m 755 $(BUILD)/meridian_demo $(DESTDIR)$(BINDIR)/meridian_demo
	install -d $(DESTDIR)$(DESKTOPDIR)
	install -m 644 packaging/desktop/org.meridian_terminal.MeridianTerminal.desktop $(DESTDIR)$(DESKTOPDIR)/org.meridian_terminal.MeridianTerminal.desktop
	install -d $(DESTDIR)$(ICONDIR)
	install -m 644 resources/icons/meridian-terminal.svg $(DESTDIR)$(ICONDIR)/meridian-terminal.svg
	install -d $(DESTDIR)$(METAINFODIR)
	install -m 644 packaging/desktop/org.meridian_terminal.MeridianTerminal.metainfo.xml $(DESTDIR)$(METAINFODIR)/org.meridian_terminal.MeridianTerminal.metainfo.xml
	install -d $(DESTDIR)$(DOCDIR)
	install -m 644 README.md LICENSE $(DESTDIR)$(DOCDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/meridian
	rm -f $(DESTDIR)$(BINDIR)/meridian-shell
	rm -f $(DESTDIR)$(BINDIR)/meridian_demo
	rm -f $(DESTDIR)$(DESKTOPDIR)/org.meridian_terminal.MeridianTerminal.desktop
	rm -f $(DESTDIR)$(ICONDIR)/meridian-terminal.svg
	rm -f $(DESTDIR)$(METAINFODIR)/org.meridian_terminal.MeridianTerminal.metainfo.xml
	rm -rf $(DESTDIR)$(DOCDIR)

tarball: clean
	mkdir -p $(BUILD)/dist/meridian-terminal-$(VERSION)
	cp -r CMakeLists.txt Makefile README.md LICENSE CHANGELOG.md CONTRIBUTING.md src tests packaging resources docs cmake $(BUILD)/dist/meridian-terminal-$(VERSION)/
	tar -czf $(BUILD)/meridian-terminal-$(VERSION).tar.gz -C $(BUILD)/dist meridian-terminal-$(VERSION)
	@echo "Source tarball created at $(BUILD)/meridian-terminal-$(VERSION).tar.gz"

clean:
	rm -rf $(BUILD)
