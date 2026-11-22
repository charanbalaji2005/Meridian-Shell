#pragma once
// src/gui/PtyBridge.hpp
//
// Connects one real PTY session (core::pty::PtyManager) to its screen model
// (vt::ScreenBuffer + vt::AnsiParser) inside Qt's event loop. Everything
// runs on the GUI thread: a QSocketNotifier wakes us when the child writes,
// so there is no reader thread and no locking.

#include <QObject>
#include <QSocketNotifier>
#include <QString>
#include <QStringList>

#include <memory>

#include "../config/profiles.hpp"
#include "../core/pty/pty_manager.hpp"
#include "../core/vt/ansi_parser.hpp"
#include "../core/vt/screen_buffer.hpp"

namespace zephyr::gui {

struct LaunchSpec {
    QString program;            // absolute path or bare name (resolved via PATH)
    QStringList args;
    QString cwd;                // empty = $HOME
    QStringList extra_env;      // "KEY=VALUE", applied on top of the inherited env
    QString profile_name;       // for session restore / display
    bool is_ssh = false;

    static LaunchSpec fromProfile(const config::Profile& p);
    static LaunchSpec fromSsh(const config::SshProfile& s);
};

class PtyBridge : public QObject {
    Q_OBJECT

public:
    PtyBridge(int rows, int cols, std::size_t scrollback, QObject* parent = nullptr);
    ~PtyBridge() override;

    // Returns false and sets errorString() if the program could not start.
    bool start(const LaunchSpec& spec);
    void write(const QByteArray& data);
    void resize(int rows, int cols);

    vt::ScreenBuffer& screen() { return screen_; }
    const vt::ScreenBuffer& screen() const { return screen_; }

    bool isRunning() const { return running_; }
    // True if something other than the shell itself owns the terminal's
    // foreground (e.g. a running build, vim, ssh): closing would kill it.
    bool hasForegroundJob() const;
    QString currentDirectory() const; // OSC 7 if reported, else /proc/<pid>/cwd
    QString title() const { return title_; }
    QString errorString() const { return error_; }
    const LaunchSpec& spec() const { return spec_; }

signals:
    void outputReceived();          // screen content changed
    void titleChanged(const QString& title);
    void finished(int exit_code);

private slots:
    void onReadable();

private:
    void handleExit();

    vt::ScreenBuffer screen_;
    vt::AnsiParser parser_;
    pty::PtyManager pty_;
    std::unique_ptr<QSocketNotifier> notifier_;
    LaunchSpec spec_;
    QString title_;
    QString error_;
    bool running_ = false;
};

} // namespace zephyr::gui
