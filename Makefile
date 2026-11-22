# Zephyr Terminal — Makefile
#
# Zephyr 2.0 Unified Build System

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -g
LDLIBS := -lutil

BUILD := build

CORE_SRC := \
    src/core/vt/screen_buffer.cpp \
    src/core/vt/ansi_parser.cpp \
    src/core/vt/input_encoder.cpp \
    src/core/vt/graphics.cpp \
    src/core/graphics/image_decoder.cpp \
    src/core/graphics/gpu_texture_manager.cpp \
    src/core/graphics/terminal_image_compositor.cpp \
    src/core/graphics/graphics_protocol_parser.cpp \
    src/core/graphics/graphics_manager.cpp \
    src/core/graphics/ascii_art_engine.cpp \
    src/core/graphics/terminal_graphic.cpp \
    src/core/graphics/pixel_art_renderer.cpp \
    src/core/graphics/animation_frame.cpp \
    src/core/graphics/animation_controller.cpp \
    src/core/graphics/animation_engine.cpp \
    src/core/icons/icon_detector.cpp \
    src/core/icons/icon_registry.cpp \
    src/core/icons/animated_glyph.cpp \
    src/core/pty/pty_manager.cpp \
    src/core/pty/pty_session.cpp \
    src/core/config.cpp \
    src/core/terminal_image.cpp \
    src/core/art_gallery.cpp \
    src/platform/LinuxPTY.cpp \
    src/core/renderer/damage_tracker.cpp \
    src/core/renderer/glyph_atlas.cpp \
    src/core/renderer/render_pipeline.cpp \
    src/core/renderer/telemetry_profiler.cpp

SHELL_SRC := \
    src/shell/lexer.cpp \
    src/shell/parser.cpp \
    src/shell/executor.cpp \
    src/shell/builtins.cpp \
    src/shell/autosuggest_engine.cpp \
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
    src/dev/rich_history.cpp \
    src/dev/ssh_manager.cpp \
    src/dev/platform_manager.cpp \
    src/dev/github_integration.cpp \
    src/dev/ide_detector.cpp \
    src/dev/icon_provider.cpp \
    src/dev/shell_history.cpp \
    src/dev/package_managers.cpp

PLUGIN_SRC := \
    src/plugins/plugin_manager.cpp

CONFIG_SRC := \
    src/config/terminal_config.cpp \
    src/config/keybindings.cpp \
    src/config/paths.cpp \
    src/config/theme.cpp \
    src/config/profiles.cpp \
    src/config/session_state.cpp

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
    tests/test_dev_tools.cpp \
    tests/test_advanced_protocols.cpp \
    tests/test_platform_manager.cpp \
    tests/test_github_integration.cpp \
    tests/test_ide_detector.cpp \
    tests/test_pixel_art_renderer.cpp \
    tests/test_native_graphics_pipeline.cpp \
    tests/test_ascii_art_engine.cpp \
    tests/test_prompt_and_autosuggest.cpp \
    tests/test_icon_registry.cpp \
    tests/test_animation_engine.cpp \
    tests/test_terminal_features.cpp

CORE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CORE_SRC))
SHELL_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SHELL_SRC))
AI_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(AI_SRC))
WORKSPACE_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(WORKSPACE_SRC))
DEV_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(DEV_SRC))
PLUGIN_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(PLUGIN_SRC))
CONFIG_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CONFIG_SRC))
SECURITY_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SECURITY_SRC))
TEST_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(TEST_SRC))

VERSION ?= 2.6.0
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
MANDIR ?= $(DATADIR)/man
DESKTOPDIR ?= $(DATADIR)/applications
ICONDIR ?= $(DATADIR)/icons/hicolor/scalable/apps
METAINFODIR ?= $(DATADIR)/metainfo
DOCDIR ?= $(DATADIR)/doc/zephyr-terminal

.PHONY: all test demo shell zephyr manual-test clean install uninstall tarball gui

all: $(BUILD)/zephyr_tests $(BUILD)/zephyr_demo $(BUILD)/zephyr-shell $(BUILD)/zephyr

# -MMD -MP: record header dependencies so editing a header rebuilds every
# object that includes it (stale objects with old struct layouts crash).
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(wildcard $(BUILD)/src/*/*.d $(BUILD)/src/*/*/*.d $(BUILD)/tests/*.d)

$(BUILD)/zephyr_tests: $(CORE_OBJ) $(SHELL_OBJ) $(AI_OBJ) $(WORKSPACE_OBJ) $(DEV_OBJ) $(PLUGIN_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ) $(TEST_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/zephyr_demo: $(CORE_OBJ) $(BUILD)/src/app/demo_main.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/zephyr-shell: $(BUILD)/src/app/shell_main.o $(SHELL_OBJ) $(CORE_OBJ) $(AI_OBJ) $(WORKSPACE_OBJ) $(DEV_OBJ) $(PLUGIN_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/zephyr: $(CORE_OBJ) $(SHELL_OBJ) $(AI_OBJ) $(WORKSPACE_OBJ) $(DEV_OBJ) $(PLUGIN_OBJ) $(CONFIG_OBJ) $(SECURITY_OBJ) $(BUILD)/src/app/zephyr_gui.o $(BUILD)/src/app/zephyr_main.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

test: $(BUILD)/zephyr_tests $(BUILD)/zephyr-shell
	ZEPHYR_SHELL_BIN=$(BUILD)/zephyr-shell $(BUILD)/zephyr_tests

manual-test: $(BUILD)/zephyr-shell
	ZEPHYR_SHELL_BIN=$(BUILD)/zephyr-shell ./tests/manual_core_test.sh

# The Qt GUI needs moc/rcc, so it is built through CMake (requires Qt 6.4+).
gui:
	cmake -S . -B $(BUILD)/cmake -G Ninja -DZEPHYR_BUILD_GUI=ON -DZEPHYR_BUILD_TESTS=OFF
	cmake --build $(BUILD)/cmake --target zephyr-terminal
	@echo "Built $(BUILD)/cmake/zephyr-terminal"

demo: $(BUILD)/zephyr_demo
	$(BUILD)/zephyr_demo

shell: $(BUILD)/zephyr-shell
	$(BUILD)/zephyr-shell

zephyr: $(BUILD)/zephyr
	$(BUILD)/zephyr $(ARGS)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BUILD)/zephyr $(DESTDIR)$(BINDIR)/zephyr
	install -m 755 $(BUILD)/zephyr-shell $(DESTDIR)$(BINDIR)/zephyr-shell
	install -m 755 $(BUILD)/zephyr_demo $(DESTDIR)$(BINDIR)/zephyr_demo
	@# The desktop entry launches the GUI, which this Makefile does not build:
	@# use `cmake --install` (see docs/build.md) for a full install.
	install -d $(DESTDIR)$(DOCDIR)
	install -m 644 README.md LICENSE $(DESTDIR)$(DOCDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/zephyr
	rm -f $(DESTDIR)$(BINDIR)/zephyr-shell
	rm -f $(DESTDIR)$(BINDIR)/zephyr_demo
	rm -f $(DESTDIR)$(DESKTOPDIR)/org.zephyr_terminal.ZephyrTerminal.desktop
	rm -f $(DESTDIR)$(ICONDIR)/zephyr-terminal.svg
	rm -f $(DESTDIR)$(METAINFODIR)/org.zephyr_terminal.ZephyrTerminal.metainfo.xml
	rm -rf $(DESTDIR)$(DOCDIR)

tarball: clean
	mkdir -p $(BUILD)/dist/zephyr-terminal-$(VERSION)
	cp -r CMakeLists.txt Makefile README.md LICENSE CHANGELOG.md CONTRIBUTING.md src tests packaging resources docs cmake $(BUILD)/dist/zephyr-terminal-$(VERSION)/
	tar -czf $(BUILD)/zephyr-terminal-$(VERSION).tar.gz -C $(BUILD)/dist zephyr-terminal-$(VERSION)
	@echo "Source tarball created at $(BUILD)/zephyr-terminal-$(VERSION).tar.gz"

clean:
	rm -rf $(BUILD)
