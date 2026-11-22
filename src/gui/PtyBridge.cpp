#include "PtyBridge.hpp"

#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>

#include <cerrno>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace zephyr::gui {

LaunchSpec LaunchSpec::fromProfile(const config::Profile& p) {
    LaunchSpec s;
    s.program = QString::fromStdString(p.program);
    for (const auto& a : p.args) s.args << QString::fromStdString(a);
    s.cwd = QString::fromStdString(p.cwd);
    for (const auto& e : p.env) {
        if (config::valid_env_name(e.name)) {
            s.extra_env << QString::fromStdString(e.name + "=" + e.value);
        }
    }
    s.profile_name = QString::fromStdString(p.name);
    return s;
}

LaunchSpec LaunchSpec::fromSsh(const config::SshProfile& p) {
    LaunchSpec s;
    s.program = QStringLiteral("ssh");
    for (const auto& a : p.ssh_args()) s.args << QString::fromStdString(a);
    s.profile_name = QStringLiteral("ssh:") + QString::fromStdString(p.name);
    s.is_ssh = true;
    return s;
}

PtyBridge::PtyBridge(int rows, int cols, std::size_t scrollback, QObject* parent)
    : QObject(parent), screen_(rows, cols, scrollback), parser_(screen_) {}

PtyBridge::~PtyBridge() {
    if (notifier_) notifier_->setEnabled(false);
    notifier_.reset();
    pty_.terminate(200); // hang up, reap; never leaves a zombie or orphan shell
}

bool PtyBridge::start(const LaunchSpec& spec) {
    spec_ = spec;

    pty::SpawnOptions opts;
    opts.program = spec.program.toStdString();
    for (const auto& a : spec.args) opts.args.push_back(a.toStdString());
    opts.rows = screen_.rows();
    opts.cols = screen_.cols();

    QString cwd = spec.cwd;
    if (cwd.startsWith(QLatin1String("~"))) cwd = QDir::homePath() + cwd.mid(1);
    if (cwd.isEmpty() || !QFileInfo(cwd).isDir()) cwd = QDir::homePath();
    opts.cwd = cwd.toStdString();

    // Full environment for the child: inherited + terminal identification +
    // the profile's own variables (which win).
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-256color"));
    env.insert(QStringLiteral("COLORTERM"), QStringLiteral("truecolor"));
    env.insert(QStringLiteral("TERM_PROGRAM"), QStringLiteral("Zephyr"));
    env.insert(QStringLiteral("TERM_PROGRAM_VERSION"), QStringLiteral(ZEPHYR_VERSION));
    env.insert(QStringLiteral("PWD"), cwd);
    for (const auto& kv : spec.extra_env) {
        const int eq = kv.indexOf('=');
        if (eq > 0) env.insert(kv.left(eq), kv.mid(eq + 1));
    }
    for (const auto& kv : env.toStringList()) opts.env.push_back(kv.toStdString());

    if (!pty_.spawn(opts)) {
        error_ = QString::fromStdString(pty_.last_error());
        return false;
    }
    int flags = fcntl(pty_.master_fd(), F_GETFL, 0);
    if (flags >= 0) fcntl(pty_.master_fd(), F_SETFL, flags | O_NONBLOCK);

    notifier_ = std::make_unique<QSocketNotifier>(pty_.master_fd(), QSocketNotifier::Read);
    connect(notifier_.get(), &QSocketNotifier::activated, this, &PtyBridge::onReadable);
    running_ = true;
    return true;
}

void PtyBridge::onReadable() {
    // Read in bounded chunks so a flood of output (e.g. `cat bigfile`) can
    // never starve input handling or painting; the notifier fires again.
    constexpr std::size_t kChunk = 64 * 1024;
    constexpr int kMaxChunksPerWake = 4;
    char buf[kChunk];
    bool got_data = false;

    for (int i = 0; i < kMaxChunksPerWake; ++i) {
        ssize_t n = pty_.read(buf, sizeof(buf));
        if (n > 0) {
            parser_.feed(buf, static_cast<std::size_t>(n));
            got_data = true;
            if (static_cast<std::size_t>(n) < sizeof(buf)) break;
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) break;
        // n == 0 (EOF) or EIO: the slave side is closed — the child exited.
        if (got_data) emit outputReceived();
        handleExit();
        return;
    }

    if (parser_.has_responses()) {
        const std::string reply = parser_.take_responses();
        pty_.write(reply);
    }
    const QString t = QString::fromStdString(parser_.window_title());
    if (t != title_) {
        title_ = t;
        emit titleChanged(title_);
    }
    if (got_data) emit outputReceived();
}

void PtyBridge::handleExit() {
    if (!running_) return;
    running_ = false;
    if (notifier_) notifier_->setEnabled(false);
    int code = -1;
    if (!pty_.try_reap(&code)) {
        pty_.terminate(200);
    }
    emit finished(code);
}

void PtyBridge::write(const QByteArray& data) {
    if (!running_ || data.isEmpty()) return;
    pty_.write(std::string(data.constData(), static_cast<std::size_t>(data.size())));
}

void PtyBridge::resize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return;
    if (rows == screen_.rows() && cols == screen_.cols()) return;
    screen_.resize(rows, cols);
    if (running_) pty_.resize(rows, cols);
    emit outputReceived();
}

bool PtyBridge::hasForegroundJob() const {
    if (!running_ || pty_.master_fd() < 0) return false;
    pid_t fg = tcgetpgrp(pty_.master_fd());
    return fg > 0 && fg != pty_.child_pid();
}

QString PtyBridge::currentDirectory() const {
    const std::string& osc7 = screen_.working_directory();
    if (!osc7.empty()) {
        QString d = QString::fromStdString(osc7);
        if (d.startsWith(QLatin1String("file://"))) {
            d = d.mid(7);
            d = d.mid(d.indexOf('/')); // drop the hostname part
        }
        if (QFileInfo(d).isDir()) return d;
    }
#ifdef __linux__
    if (running_ && pty_.child_pid() > 0) {
        QFileInfo link(QStringLiteral("/proc/%1/cwd").arg(pty_.child_pid()));
        QString target = link.symLinkTarget();
        if (!target.isEmpty()) return target;
    }
#endif
    return spec_.cwd.isEmpty() ? QDir::homePath() : spec_.cwd;
}

} // namespace zephyr::gui
