cmake_minimum_required(VERSION 3.20)
project(zephyr-terminal
    VERSION 2.6.0
    DESCRIPTION "Zephyr Terminal — modern Linux terminal emulator, shell and developer tools"
    HOMEPAGE_URL "https://github.com/charanbalaji2005/Meridian-Shell"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)

# AUTO: build the Qt GUI when Qt 6 (Widgets + Svg) is found; ON: require it.
set(ZEPHYR_BUILD_GUI "AUTO" CACHE STRING "Build the zephyr-terminal Qt GUI (ON|OFF|AUTO)")
set_property(CACHE ZEPHYR_BUILD_GUI PROPERTY STRINGS ON OFF AUTO)
option(ZEPHYR_BUILD_TESTS "Build the Zephyr test suite" ON)
option(ZEPHYR_ENABLE_ASAN "Build with AddressSanitizer + UBSan" OFF)
option(ZEPHYR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_compile_options(-Wall -Wextra)
if(ZEPHYR_WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
endif()
if(ZEPHYR_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()
add_compile_definitions(ZEPHYR_VERSION="${PROJECT_VERSION}")

# libutil provides openpty() on glibc; it is part of libc on musl/BSD.
find_library(UTIL_LIBRARY util)
set(ZEPHYR_PTY_LIBS "")
if(UTIL_LIBRARY)
    set(ZEPHYR_PTY_LIBS ${UTIL_LIBRARY})
endif()

# ---- Libraries (source lists mirror the Makefile) ------------------------------

add_library(zephyr-core STATIC
    src/core/vt/screen_buffer.cpp
    src/core/vt/ansi_parser.cpp
    src/core/vt/input_encoder.cpp
    src/core/vt/graphics.cpp
    src/core/graphics/image_decoder.cpp
    src/core/graphics/gpu_texture_manager.cpp
    src/core/graphics/terminal_image_compositor.cpp
    src/core/graphics/graphics_protocol_parser.cpp
    src/core/graphics/graphics_manager.cpp
    src/core/graphics/ascii_art_engine.cpp
    src/core/graphics/terminal_graphic.cpp
    src/core/graphics/pixel_art_renderer.cpp
    src/core/graphics/animation_frame.cpp
    src/core/graphics/animation_controller.cpp
    src/core/graphics/animation_engine.cpp
    src/core/icons/icon_detector.cpp
    src/core/icons/icon_registry.cpp
    src/core/icons/animated_glyph.cpp
    src/core/pty/pty_manager.cpp
    src/core/pty/pty_session.cpp
    src/core/config.cpp
    src/core/terminal_image.cpp
    src/core/art_gallery.cpp
    src/platform/LinuxPTY.cpp
    src/core/renderer/damage_tracker.cpp
    src/core/renderer/glyph_atlas.cpp
    src/core/renderer/render_pipeline.cpp
    src/core/renderer/telemetry_profiler.cpp
)
target_include_directories(zephyr-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(zephyr-core PUBLIC ${ZEPHYR_PTY_LIBS})

add_library(zephyr-config-lib STATIC
    src/config/terminal_config.cpp
    src/config/keybindings.cpp
    src/config/paths.cpp
    src/config/theme.cpp
    src/config/profiles.cpp
    src/config/session_state.cpp
)
target_link_libraries(zephyr-config-lib PUBLIC zephyr-core)

add_library(zephyr-ai-lib STATIC
    src/ai/secret_redactor.cpp
    src/ai/risk_classifier.cpp
    src/ai/command_analyzer.cpp
    src/ai/command_context.cpp
    src/ai/ai_controller.cpp
    src/ai/intent_engine.cpp
    src/ai/error_diagnostics.cpp
    src/ai/ai_agent.cpp
)
target_link_libraries(zephyr-ai-lib PUBLIC zephyr-core)

add_library(zephyr-dev-lib STATIC
    src/dev/git_intel.cpp
    src/dev/system_monitor.cpp
    src/dev/file_explorer.cpp
    src/dev/command_palette.cpp
    src/dev/universal_search.cpp
    src/dev/rich_history.cpp
    src/dev/ssh_manager.cpp
    src/dev/platform_manager.cpp
    src/dev/github_integration.cpp
    src/dev/ide_detector.cpp
    src/dev/icon_provider.cpp
    src/dev/shell_history.cpp
    src/dev/package_managers.cpp
)
target_link_libraries(zephyr-dev-lib PUBLIC zephyr-core)

add_library(zephyr-workspace-lib STATIC
    src/workspace/pane_tree.cpp
    src/workspace/workspace_manager.cpp
    src/workspace/session_recorder.cpp
)
target_link_libraries(zephyr-workspace-lib PUBLIC zephyr-core)

add_library(zephyr-plugin-lib STATIC src/plugins/plugin_manager.cpp)
target_link_libraries(zephyr-plugin-lib PUBLIC zephyr-core)

add_library(zephyr-security-lib STATIC src/security/credential_store.cpp)
target_link_libraries(zephyr-security-lib PUBLIC zephyr-core)

add_library(zephyr-shell-lib STATIC
    src/shell/lexer.cpp
    src/shell/parser.cpp
    src/shell/executor.cpp
    src/shell/builtins.cpp
    src/shell/autosuggest_engine.cpp
    src/shell/line_editor.cpp
    src/shell/shell.cpp
)
target_link_libraries(zephyr-shell-lib PUBLIC
    zephyr-core zephyr-ai-lib zephyr-dev-lib zephyr-workspace-lib
    zephyr-plugin-lib zephyr-config-lib zephyr-security-lib)

# ---- Executables -----------------------------------------------------------------

# zephyr: the CLI (AI helpers, workspaces, monitors, inline images, …)
add_executable(zephyr src/app/zephyr_gui.cpp src/app/zephyr_main.cpp)
target_link_libraries(zephyr PRIVATE zephyr-shell-lib)

# zephyr-shell: the standalone POSIX-style shell
add_executable(zephyr-shell src/app/shell_main.cpp)
target_link_libraries(zephyr-shell PRIVATE zephyr-shell-lib)

# zephyr_demo: headless PTY + ANSI integration demo (not installed)
add_executable(zephyr_demo src/app/demo_main.cpp)
target_link_libraries(zephyr_demo PRIVATE zephyr-core)

# zephyr-terminal: the Qt 6 graphical terminal emulator
set(ZEPHYR_HAVE_GUI OFF)
if(NOT ZEPHYR_BUILD_GUI STREQUAL "OFF")
    if(ZEPHYR_BUILD_GUI STREQUAL "ON")
        find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Svg)
    else()
        find_package(Qt6 6.4 QUIET COMPONENTS Widgets Svg)
    endif()
    if(Qt6_FOUND)
        set(ZEPHYR_HAVE_GUI ON)
    else()
        message(STATUS "Qt 6 (Widgets, Svg) not found: zephyr-terminal GUI will not be built")
    endif()
endif()

if(ZEPHYR_HAVE_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    add_executable(zephyr-terminal
        src/app/gui_main.cpp
        src/gui/Actions.hpp
        src/gui/AppContext.cpp
        src/gui/CommandPalette.cpp
        src/gui/IconButton.cpp
        src/gui/Icons.cpp
        src/gui/MainWindow.cpp
        src/gui/Motion.cpp
        src/gui/PtyBridge.cpp
        src/gui/SettingsDialog.cpp
        src/gui/Sidebar.cpp
        src/gui/SidebarPanels.cpp
        src/gui/TabPage.cpp
        src/gui/TerminalPane.cpp
        src/gui/TerminalView.cpp
        src/gui/UiTheme.cpp
        src/gui/WindowFrame.cpp
        resources/zephyr.qrc
    )
    target_link_libraries(zephyr-terminal PRIVATE
        zephyr-core zephyr-config-lib zephyr-dev-lib zephyr-ai-lib
        Qt6::Widgets Qt6::Svg)
    target_compile_definitions(zephyr-terminal PRIVATE QT_NO_CAST_FROM_BYTEARRAY QT_NO_URL_CAST_FROM_STRING)
    message(STATUS "Building zephyr-terminal GUI with Qt ${Qt6_VERSION}")
endif()

# ---- Tests -------------------------------------------------------------------------

if(ZEPHYR_BUILD_TESTS)
    enable_testing()
    add_executable(zephyr_tests
        tests/test_main.cpp
        tests/test_screen_buffer.cpp
        tests/test_ansi_parser.cpp
        tests/test_pty_manager.cpp
        tests/test_shell_lexer.cpp
        tests/test_shell_parser.cpp
        tests/test_shell_executor.cpp
        tests/test_job_control.cpp
        tests/test_shell_class.cpp
        tests/test_ai.cpp
        tests/test_command_context.cpp
        tests/test_config.cpp
        tests/test_security.cpp
        tests/test_renderer.cpp
        tests/test_workspace.cpp
        tests/test_intent_and_diagnostics.cpp
        tests/test_dev_tools.cpp
        tests/test_advanced_protocols.cpp
        tests/test_platform_manager.cpp
        tests/test_github_integration.cpp
        tests/test_ide_detector.cpp
        tests/test_pixel_art_renderer.cpp
        tests/test_native_graphics_pipeline.cpp
        tests/test_ascii_art_engine.cpp
        tests/test_prompt_and_autosuggest.cpp
        tests/test_icon_registry.cpp
        tests/test_animation_engine.cpp
        tests/test_terminal_features.cpp
    )
    target_link_libraries(zephyr_tests PRIVATE zephyr-shell-lib)
    add_dependencies(zephyr_tests zephyr-shell)
    # Run from the source tree (some tests read resources/); point the job
    # control tests at the freshly built shell.
    add_test(NAME zephyr_tests COMMAND zephyr_tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME zephyr_manual_core_test COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/manual_core_test.sh
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(zephyr_tests zephyr_manual_core_test PROPERTIES
        ENVIRONMENT "ZEPHYR_SHELL_BIN=$<TARGET_FILE:zephyr-shell>")
endif()

# ---- Installation (shared by every Linux package) -----------------------------------

install(TARGETS zephyr zephyr-shell RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(ZEPHYR_HAVE_GUI)
    install(TARGETS zephyr-terminal RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES packaging/desktop/org.zephyr_terminal.ZephyrTerminal.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES packaging/desktop/org.zephyr_terminal.ZephyrTerminal.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(FILES resources/icons/zephyr-terminal.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    foreach(size 16 24 32 48 64 128 256 512)
        install(FILES resources/icons/hicolor/${size}x${size}/apps/zephyr-terminal.png
                DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps)
    endforeach()
endif()
install(DIRECTORY resources/images/gallery/ DESTINATION ${CMAKE_INSTALL_DATADIR}/zephyr/images/gallery
        FILES_MATCHING PATTERN "*.png")
# Distribution packages that install docs/licenses themselves (RPM %doc /
# %license, Debian copyright) turn these off to avoid duplicate files.
option(ZEPHYR_INSTALL_DOCS "Install README and CHANGELOG" ON)
option(ZEPHYR_INSTALL_LICENSE "Install LICENSE to share/licenses" ON)
if(ZEPHYR_INSTALL_DOCS)
    install(FILES README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
if(ZEPHYR_INSTALL_LICENSE)
    install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/zephyr-terminal)
endif()

# Source tarball / binary tarball via CPack. Distribution packages (.deb,
# .rpm, PKGBUILD, Nix, AppImage) are built from packaging/ instead.
set(CPACK_PACKAGE_NAME "zephyr-terminal")
set(CPACK_PACKAGE_VENDOR "Zephyr Terminal contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/dist/;/\\\\.git/;\\\\.backup")
include(CPack)
