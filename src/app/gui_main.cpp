// src/app/gui_main.cpp
//
// zephyr-terminal: the graphical terminal emulator.
//
//   zephyr-terminal                        restore last session (if enabled) or open the default profile
//   zephyr-terminal --profile NAME         open a tab with a saved profile
//   zephyr-terminal -d DIR                 start in DIR
//   zephyr-terminal -e PROGRAM [ARGS...]   run PROGRAM instead of a shell (used by desktops,
//                                          xdg-terminal-exec and .desktop files with Terminal=true)

#include "../gui/AppContext.hpp"
#include "../gui/MainWindow.hpp"
#include "../gui/UiTheme.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>

#include <cstdio>

using namespace zephyr;

int main(int argc, char** argv) {
    // Everything after -e / -x / --command is the program and its arguments,
    // verbatim; split it off before Qt or the option parser see it.
    QStringList command;
    int qt_argc = argc;
    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a == QLatin1String("-e") || a == QLatin1String("-x") || a == QLatin1String("--command")) {
            for (int j = i + 1; j < argc; ++j) command << QString::fromLocal8Bit(argv[j]);
            qt_argc = i;
            break;
        }
    }

    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication app(qt_argc, argv);
    // applicationName drives the X11 WM_CLASS and config paths; the desktop
    // file name becomes the Wayland app_id so docks show the right icon.
    QApplication::setApplicationName(QStringLiteral("zephyr-terminal"));
    QApplication::setApplicationDisplayName(QStringLiteral("Zephyr Terminal"));
    QApplication::setApplicationVersion(QStringLiteral(ZEPHYR_VERSION));
    QApplication::setOrganizationName(QStringLiteral("Zephyr"));
    QGuiApplication::setDesktopFileName(QStringLiteral("org.zephyr_terminal.ZephyrTerminal"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("zephyr-terminal"),
                                                 QIcon(QStringLiteral(":/app/zephyr-terminal.svg"))));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Zephyr Terminal — a modern Linux terminal emulator.\n"
                                                    "Use -e PROGRAM [ARGS...] to run a program instead of a shell."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption profile_opt({"p", "profile"}, QStringLiteral("Open a tab with the named profile."), "name");
    QCommandLineOption dir_opt({"d", "working-directory"}, QStringLiteral("Start in this directory."), "dir");
    QCommandLineOption no_restore_opt("no-restore", QStringLiteral("Don't restore the previous session."));
    parser.addOptions({profile_opt, dir_opt, no_restore_opt});
    parser.process(app);

    gui::UiTheme::captureSystemPalette();
    auto& ctx = gui::AppContext::instance();
    gui::UiTheme::apply();
    QObject::connect(&ctx, &gui::AppContext::settingsChanged, &app, []() { gui::UiTheme::apply(); });

    const bool explicit_launch = !command.isEmpty() || parser.isSet(profile_opt) || parser.isSet(dir_opt);
    gui::MainWindow* window = nullptr;

    if (explicit_launch) {
        config::Profile base = ctx.defaultProfile();
        if (parser.isSet(profile_opt)) {
            const auto* p = ctx.profiles().find(parser.value(profile_opt).toStdString());
            if (!p) {
                std::fprintf(stderr, "zephyr-terminal: no profile named '%s'\n",
                             parser.value(profile_opt).toLocal8Bit().constData());
                return 2;
            }
            base = *p;
        }
        gui::LaunchSpec spec = gui::LaunchSpec::fromProfile(base);
        if (!command.isEmpty()) {
            spec.program = command.takeFirst();
            spec.args = command;
        }
        if (parser.isSet(dir_opt)) spec.cwd = QDir(parser.value(dir_opt)).absolutePath();
        else if (spec.cwd.isEmpty()) spec.cwd = QDir::currentPath();
        window = new gui::MainWindow(nullptr, &spec);
    } else {
        config::SessionState session;
        const bool restore = ctx.settings().restore_session && !parser.isSet(no_restore_opt) &&
                             ctx.sessionStore().load(session);
        window = new gui::MainWindow(restore ? &session : nullptr);
    }
    window->show();
    return app.exec();
}
