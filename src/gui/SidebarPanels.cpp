#include "SidebarPanels.hpp"

#include "AppContext.hpp"
#include "Icons.hpp"
#include "UiTheme.hpp"

#include "../ai/intent_engine.hpp"
#include "../dev/package_managers.hpp"
#include "../dev/rich_history.hpp"
#include "../dev/shell_history.hpp"
#include "../dev/system_monitor.hpp"

#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardPaths>
#include <QStyle>
#include <QSysInfo>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace zephyr::gui {

namespace {

QLabel* muted(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setObjectName(QStringLiteral("Muted"));
    l->setWordWrap(true);
    return l;
}

QPushButton* button(const QString& icon, const QString& text, QWidget* parent) {
    auto* b = new QPushButton(Icons::icon(icon), text, parent);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QString read_file_tail(const QString& path, qint64 max_bytes = 2 * 1024 * 1024) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    if (f.size() > max_bytes) f.seek(f.size() - max_bytes);
    return QString::fromUtf8(f.readAll());
}

QString human_bytes(double bytes) {
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    int u = 0;
    while (bytes >= 1024.0 && u < 4) { bytes /= 1024.0; ++u; }
    return QStringLiteral("%1 %2").arg(bytes, 0, 'f', u == 0 ? 0 : 1).arg(units[u]);
}

QVBoxLayout* panel_layout(QWidget* w) {
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(10, 4, 10, 10);
    lay->setSpacing(8);
    return lay;
}

} // namespace

// ---- Profiles ---------------------------------------------------------------

ProfilesPanel::ProfilesPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = panel_layout(this);
    list_ = new QListWidget(this);
    list_->setIconSize(QSize(16, 16));
    lay->addWidget(list_, 1);
    auto* row = new QHBoxLayout();
    auto* manage_profiles = button(QStringLiteral("user"), tr("Profiles…"), this);
    auto* manage_ssh = button(QStringLiteral("server"), tr("SSH…"), this);
    row->addWidget(manage_profiles);
    row->addWidget(manage_ssh);
    lay->addLayout(row);
    lay->addWidget(muted(tr("Click to open a new tab with that profile."), this));

    connect(list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        const QString name = item->data(Qt::UserRole).toString();
        if (item->data(Qt::UserRole + 1).toBool()) emit launchSsh(name);
        else emit launchProfile(name);
    });
    connect(manage_profiles, &QPushButton::clicked, this, [this]() { emit manage(QStringLiteral("profiles")); });
    connect(manage_ssh, &QPushButton::clicked, this, [this]() { emit manage(QStringLiteral("ssh")); });
    connect(&AppContext::instance(), &AppContext::profilesChanged, this, &ProfilesPanel::refresh);
    refresh();
}

void ProfilesPanel::refresh() {
    list_->clear();
    const auto& store = AppContext::instance().profiles();
    const QString def = QString::fromStdString(AppContext::instance().settings().default_profile);
    for (const auto& p : store.profiles()) {
        const QString name = QString::fromStdString(p.name);
        auto* item = new QListWidgetItem(Icons::icon(QStringLiteral("terminal")),
                                         name == def ? tr("%1  (default)").arg(name) : name, list_);
        item->setData(Qt::UserRole, name);
        item->setToolTip(QString::fromStdString(p.program + " " + config::join_args(p.args)));
    }
    for (const auto& s : store.ssh_profiles()) {
        auto* item = new QListWidgetItem(Icons::icon(QStringLiteral("server")), QString::fromStdString(s.name), list_);
        item->setData(Qt::UserRole, QString::fromStdString(s.name));
        item->setData(Qt::UserRole + 1, true);
        item->setToolTip(QString::fromStdString((s.user.empty() ? "" : s.user + "@") + s.host +
                                                (s.port != 22 ? ":" + std::to_string(s.port) : "")));
    }
}

// ---- Files --------------------------------------------------------------------

FilesPanel::FilesPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = panel_layout(this);
    path_label_ = muted(QString(), this);
    path_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lay->addWidget(path_label_);

    model_ = new QFileSystemModel(this);
    model_->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    tree_ = new QTreeView(this);
    tree_->setModel(model_);
    tree_->setHeaderHidden(true);
    for (int c = 1; c < model_->columnCount(); ++c) tree_->hideColumn(c);
    tree_->setIconSize(QSize(16, 16));
    tree_->setAnimated(AppContext::instance().animationsEnabled());
    tree_->setToolTip(tr("Double-click a folder to open it, or a file to type its path at the prompt"));
    lay->addWidget(tree_, 1);

    auto* row = new QHBoxLayout();
    auto* up = button(QStringLiteral("chevron-up"), tr("Up"), this);
    auto* follow = button(QStringLiteral("refresh"), tr("Terminal dir"), this);
    auto* open = button(QStringLiteral("external-link"), tr("Open"), this);
    up->setToolTip(tr("Parent folder"));
    follow->setToolTip(tr("Jump to the active terminal's working directory"));
    open->setToolTip(tr("Open this folder in your file manager"));
    row->addWidget(up);
    row->addWidget(follow);
    row->addWidget(open);
    lay->addLayout(row);

    connect(tree_, &QTreeView::doubleClicked, this, [this](const QModelIndex& idx) {
        const QString path = model_->filePath(idx);
        if (model_->isDir(idx)) navigate(path);
        else emit insertText(QString::fromStdString(dev::shell_quote(path.toStdString())) + QLatin1Char(' '));
    });
    connect(up, &QPushButton::clicked, this, [this]() {
        QDir d(current_dir_);
        if (d.cdUp()) navigate(d.absolutePath());
    });
    connect(follow, &QPushButton::clicked, this, [this]() { navigate(terminal_dir_); });
    connect(open, &QPushButton::clicked, this,
            [this]() { QDesktopServices::openUrl(QUrl::fromLocalFile(current_dir_)); });
    navigate(QDir::homePath());
}

void FilesPanel::setTerminalDirectory(const QString& dir) {
    // Follow the terminal until the user browses elsewhere.
    const bool following = current_dir_ == terminal_dir_ || terminal_dir_.isEmpty();
    terminal_dir_ = dir;
    if (following) navigate(dir);
}

void FilesPanel::navigate(const QString& dir) {
    if (dir.isEmpty() || !QFileInfo(dir).isDir()) return;
    current_dir_ = QDir(dir).absolutePath();
    tree_->setRootIndex(model_->setRootPath(current_dir_));
    QString shown = current_dir_;
    if (shown.startsWith(QDir::homePath())) shown = QStringLiteral("~") + shown.mid(QDir::homePath().size());
    path_label_->setText(shown);
}

// ---- History --------------------------------------------------------------------

HistoryPanel::HistoryPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = panel_layout(this);
    filter_ = new QLineEdit(this);
    filter_->setPlaceholderText(tr("Filter history"));
    filter_->setClearButtonEnabled(true);
    lay->addWidget(filter_);
    list_ = new QListWidget(this);
    list_->setUniformItemSizes(true);
    list_->setToolTip(tr("Double-click to type the command at the prompt (it is not run)"));
    lay->addWidget(list_, 1);
    auto* row = new QHBoxLayout();
    auto* insert = button(QStringLiteral("corner-down-left"), tr("Insert"), this);
    auto* reload = button(QStringLiteral("refresh"), tr("Reload"), this);
    row->addWidget(insert);
    row->addWidget(reload);
    lay->addLayout(row);
    source_label_ = muted(QString(), this);
    lay->addWidget(source_label_);

    auto insert_current = [this]() {
        if (auto* item = list_->currentItem()) emit insertText(item->data(Qt::UserRole).toString());
    };
    connect(list_, &QListWidget::itemActivated, this, insert_current);
    connect(insert, &QPushButton::clicked, this, insert_current);
    connect(reload, &QPushButton::clicked, this, &HistoryPanel::reload);
    connect(filter_, &QLineEdit::textChanged, this, &HistoryPanel::applyFilter);
}

void HistoryPanel::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    if (!loaded_) reload(); // lazy: history files are only read when the panel is opened
}

void HistoryPanel::reload() {
    loaded_ = true;
    const QString home = QDir::homePath();
    std::vector<std::string> all;
    QStringList sources;
    auto add = [&](const QString& label, std::vector<std::string> cmds) {
        if (cmds.empty()) return;
        sources << QStringLiteral("%1 (%2)").arg(label).arg(cmds.size());
        all.insert(all.end(), cmds.begin(), cmds.end());
    };
    add(QStringLiteral("bash"), dev::parse_bash_history(read_file_tail(home + "/.bash_history").toStdString()));
    const QByteArray histfile = qgetenv("HISTFILE");
    const QString zsh_path = !histfile.isEmpty() && histfile.contains("zsh") ? QString::fromLocal8Bit(histfile)
                                                                             : home + "/.zsh_history";
    {
        QFile f(zsh_path); // zsh history is metafied bytes, not UTF-8 — read raw
        if (f.open(QIODevice::ReadOnly)) {
            if (f.size() > 2 * 1024 * 1024) f.seek(f.size() - 2 * 1024 * 1024);
            add(QStringLiteral("zsh"), dev::parse_zsh_history(f.readAll().toStdString()));
        }
    }
    const QString fish_dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/fish";
    add(QStringLiteral("fish"), dev::parse_fish_history(read_file_tail(fish_dir + "/fish_history").toStdString()));
    {
        dev::RichHistory rich; // Zephyr shell's own history database
        std::vector<std::string> cmds;
        for (const auto& r : rich.all_records()) cmds.push_back(r.command);
        add(QStringLiteral("zephyr-shell"), cmds);
    }

    list_->clear();
    for (const auto& cmd : dev::dedupe_recent_first(all, 1000)) {
        QString text = QString::fromStdString(cmd);
        auto* item = new QListWidgetItem(QString(text).replace('\n', QStringLiteral(" ⏎ ")), list_);
        item->setData(Qt::UserRole, text);
        item->setToolTip(text);
    }
    source_label_->setText(sources.isEmpty() ? tr("No shell history files found.")
                                             : tr("Sources: %1").arg(sources.join(QStringLiteral(", "))));
    applyFilter(filter_->text());
}

void HistoryPanel::applyFilter(const QString& text) {
    for (int i = 0; i < list_->count(); ++i) {
        auto* item = list_->item(i);
        item->setHidden(!text.isEmpty() && !item->data(Qt::UserRole).toString().contains(text, Qt::CaseInsensitive));
    }
}

// ---- System -------------------------------------------------------------------------

SystemPanel::SystemPanel(QWidget* parent) : QWidget(parent), monitor_(new dev::SystemMonitor()) {
    auto* lay = panel_layout(this);
    os_ = new QLabel(this);
    os_->setObjectName(QStringLiteral("Heading"));
    os_->setWordWrap(true);
    kernel_ = muted(QString(), this);
    host_ = muted(QString(), this);
    desktop_ = muted(QString(), this);
    uptime_ = muted(QString(), this);
    for (auto* l : {static_cast<QLabel*>(os_), kernel_, host_, desktop_, uptime_}) lay->addWidget(l);
    lay->addSpacing(6);
    cpu_bar_ = addMeter(lay, tr("CPU"), &cpu_val_);
    mem_bar_ = addMeter(lay, tr("Memory"), &mem_val_);
    swap_bar_ = addMeter(lay, tr("Swap"), &swap_val_);
    disk_bar_ = addMeter(lay, tr("Disk (/)"), &disk_val_);
    net_ = muted(QString(), this);
    lay->addWidget(net_);
    lay->addStretch(1);

    QFile osr(QStringLiteral("/etc/os-release"));
    QString pretty = QSysInfo::prettyProductName();
    if (osr.open(QIODevice::ReadOnly)) {
        auto kv = dev::parse_os_release(osr.readAll().toStdString());
        if (kv.count("PRETTY_NAME")) pretty = QString::fromStdString(kv["PRETTY_NAME"]);
    }
    os_->setText(pretty);
    kernel_->setText(tr("Kernel %1 · %2").arg(QSysInfo::kernelVersion(), QSysInfo::currentCpuArchitecture()));
    host_->setText(tr("Host %1").arg(QSysInfo::machineHostName()));
    const QString de = qEnvironmentVariable("XDG_CURRENT_DESKTOP", tr("unknown desktop"));
    const QString session = qEnvironmentVariable("XDG_SESSION_TYPE", tr("unknown session"));
    desktop_->setText(tr("%1 · %2").arg(de, session));

    timer_.setInterval(2000); // only runs while the panel is visible
    connect(&timer_, &QTimer::timeout, this, &SystemPanel::sample);
}

SystemPanel::~SystemPanel() { delete monitor_; }

QProgressBar* SystemPanel::addMeter(QVBoxLayout* lay, const QString& label, QLabel** value_out) {
    auto* row = new QHBoxLayout();
    auto* name = new QLabel(label, this);
    auto* value = muted(QString(), this);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    value->setWordWrap(false);
    row->addWidget(name);
    row->addStretch(1);
    row->addWidget(value);
    lay->addLayout(row);
    auto* bar = new QProgressBar(this);
    bar->setRange(0, 1000);
    bar->setTextVisible(false);
    bar->setFixedHeight(6);
    lay->addWidget(bar);
    *value_out = value;
    return bar;
}

void SystemPanel::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    // Accent-colored meters that follow the theme.
    const auto c = UiTheme::colors();
    const QString ss = QStringLiteral("QProgressBar { background: %1; border: none; border-radius: 3px; }"
                                      "QProgressBar::chunk { background: %2; border-radius: 3px; }")
                           .arg(UiTheme::css(c.border), UiTheme::css(c.accent));
    for (auto* b : {cpu_bar_, mem_bar_, swap_bar_, disk_bar_}) b->setStyleSheet(ss);
    sample();
    timer_.start();
}

void SystemPanel::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    timer_.stop();
}

void SystemPanel::sample() {
    const auto m = monitor_->sample();
    cpu_bar_->setValue(static_cast<int>(m.cpu_percent * 10));
    cpu_val_->setText(QStringLiteral("%1%").arg(m.cpu_percent, 0, 'f', 1));
    mem_bar_->setValue(static_cast<int>(m.mem_percent * 10));
    mem_val_->setText(QStringLiteral("%1 / %2").arg(human_bytes(m.mem_used_bytes), human_bytes(m.mem_total_bytes)));
    const double swap_pct = m.swap_total_bytes ? 100.0 * m.swap_used_bytes / m.swap_total_bytes : 0.0;
    swap_bar_->setValue(static_cast<int>(swap_pct * 10));
    swap_val_->setText(m.swap_total_bytes ? QStringLiteral("%1 / %2").arg(human_bytes(m.swap_used_bytes),
                                                                          human_bytes(m.swap_total_bytes))
                                          : tr("none"));
    disk_bar_->setValue(static_cast<int>(m.disk_percent * 10));
    disk_val_->setText(QStringLiteral("%1%").arg(m.disk_percent, 0, 'f', 1));
    net_->setText(tr("Network ↓ %1/s  ↑ %2/s").arg(human_bytes(m.net_rx_rate_bytes), human_bytes(m.net_tx_rate_bytes)));

    QFile up(QStringLiteral("/proc/uptime"));
    if (up.open(QIODevice::ReadOnly)) {
        const qint64 secs = static_cast<qint64>(QString::fromLatin1(up.readAll()).split(' ').value(0).toDouble());
        uptime_->setText(tr("Up %1d %2h %3m").arg(secs / 86400).arg((secs % 86400) / 3600).arg((secs % 3600) / 60));
    }
}

// ---- Packages ----------------------------------------------------------------------

PackagesPanel::PackagesPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = panel_layout(this);
    distro_ = new QLabel(this);
    distro_->setObjectName(QStringLiteral("Heading"));
    lay->addWidget(distro_);
    manager_ = new QComboBox(this);
    lay->addWidget(manager_);
    package_ = new QLineEdit(this);
    package_->setPlaceholderText(tr("Package name"));
    package_->setClearButtonEnabled(true);
    lay->addWidget(package_);

    auto* grid = new QHBoxLayout();
    auto* search = button(QStringLiteral("search"), tr("Search"), this);
    auto* install = button(QStringLiteral("download"), tr("Install"), this);
    auto* remove = button(QStringLiteral("trash"), tr("Remove"), this);
    grid->addWidget(search);
    grid->addWidget(install);
    grid->addWidget(remove);
    lay->addLayout(grid);
    auto* upgrade = button(QStringLiteral("refresh"), tr("Upgrade system"), this);
    lay->addWidget(upgrade);
    status_ = muted(QString(), this);
    lay->addWidget(status_);
    lay->addWidget(muted(tr("Commands are typed at the prompt for you to review — "
                            "nothing runs until you press Enter."), this));
    lay->addStretch(1);

    // Detect: distro preference first, then anything else installed.
    std::map<std::string, std::string> osr;
    QFile f(QStringLiteral("/etc/os-release"));
    if (f.open(QIODevice::ReadOnly)) osr = dev::parse_os_release(f.readAll().toStdString());
    distro_->setText(osr.count("PRETTY_NAME") ? QString::fromStdString(osr["PRETTY_NAME"]) : tr("Linux"));
    std::vector<std::string> order = dev::package_managers_for_distro(osr);
    for (const auto& pm : dev::known_package_managers()) {
        if (std::find(order.begin(), order.end(), pm.id) == order.end()) order.push_back(pm.id);
    }
    for (const auto& id : order) {
        const auto* pm = dev::find_package_manager(id);
        if (pm && !QStandardPaths::findExecutable(QString::fromStdString(pm->executable)).isEmpty()) {
            manager_->addItem(QString::fromStdString(pm->display), QString::fromStdString(pm->id));
        }
    }
    const bool any = manager_->count() > 0;
    if (!any) status_->setText(tr("No supported package manager found on PATH."));
    for (auto* w : std::initializer_list<QWidget*>{manager_, package_, search, install, remove, upgrade}) w->setEnabled(any);

    connect(search, &QPushButton::clicked, this, [this]() { emitCommand(QStringLiteral("search")); });
    connect(install, &QPushButton::clicked, this, [this]() { emitCommand(QStringLiteral("install")); });
    connect(remove, &QPushButton::clicked, this, [this]() { emitCommand(QStringLiteral("remove")); });
    connect(upgrade, &QPushButton::clicked, this, [this]() { emitCommand(QStringLiteral("upgrade")); });
    connect(package_, &QLineEdit::returnPressed, this, [this]() { emitCommand(QStringLiteral("search")); });
}

void PackagesPanel::emitCommand(const QString& which) {
    const auto* pm = dev::find_package_manager(manager_->currentData().toString().toStdString());
    if (!pm) return;
    if (which == QLatin1String("upgrade")) {
        status_->clear();
        emit insertText(QString::fromStdString(pm->upgrade));
        return;
    }
    const std::string name = package_->text().trimmed().toStdString();
    const std::string& tmpl = which == QLatin1String("search") ? pm->search
                            : which == QLatin1String("install") ? pm->install : pm->remove;
    const std::string cmd = dev::package_command(tmpl, name);
    if (cmd.empty()) {
        status_->setText(tr("Enter a valid package name (letters, digits, . _ + - @ :)."));
        return;
    }
    status_->clear();
    emit insertText(QString::fromStdString(cmd));
}

// ---- Assistant ------------------------------------------------------------------------

AssistantPanel::AssistantPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = panel_layout(this);
    lay->addWidget(muted(tr("Describe what you want to do. Suggestions come from Zephyr's built-in, "
                            "rule-based engine: offline, nothing leaves your machine."), this));
    input_ = new QLineEdit(this);
    input_->setPlaceholderText(tr("e.g. find python files modified in 3 days"));
    input_->setClearButtonEnabled(true);
    lay->addWidget(input_);
    auto* go = button(QStringLiteral("bot"), tr("Suggest command"), this);
    go->setObjectName(QStringLiteral("Primary"));
    lay->addWidget(go);

    card_ = new QWidget(this);
    QWidget* card = card_;
    card->setObjectName(QStringLiteral("Card"));
    card->setAttribute(Qt::WA_StyledBackground, true);
    auto* card_lay = new QVBoxLayout(card);
    command_ = new QLabel(card);
    command_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    command_->setWordWrap(true);
    QFont mono(QStringLiteral("monospace"));
    mono.setStyleHint(QFont::Monospace);
    command_->setFont(mono);
    risk_ = new QLabel(card);
    explanation_ = muted(QString(), card);
    card_lay->addWidget(command_);
    card_lay->addWidget(risk_);
    card_lay->addWidget(explanation_);
    insert_ = button(QStringLiteral("corner-down-left"), tr("Type at prompt"), card);
    card_lay->addWidget(insert_);
    lay->addWidget(card);
    card->hide();

    alternatives_ = new QListWidget(this);
    alternatives_->setToolTip(tr("Double-click an alternative to type it at the prompt"));
    alternatives_->hide();
    lay->addWidget(alternatives_, 1);
    lay->addStretch(1);

    connect(go, &QPushButton::clicked, this, &AssistantPanel::ask);
    connect(input_, &QLineEdit::returnPressed, this, &AssistantPanel::ask);
    connect(insert_, &QPushButton::clicked, this, [this]() { emit insertText(command_->text()); });
    connect(alternatives_, &QListWidget::itemActivated, this,
            [this](QListWidgetItem* item) { emit insertText(item->text()); });
}

void AssistantPanel::ask() {
    const QString q = input_->text().trimmed();
    if (q.isEmpty()) return;
    ai::IntentEngine engine;
    const auto r = engine.translate(q.toStdString(), cwd_.toStdString());
    QWidget* card = card_;
    if (r.generated_command.empty()) {
        command_->setText(QString());
        risk_->setText(QString());
        explanation_->setText(tr("No suggestion for that request. Try describing a concrete task, "
                                 "e.g. \"kill process on port 3000\"."));
        insert_->hide();
        alternatives_->hide();
        if (card) card->show();
        return;
    }
    command_->setText(QString::fromStdString(r.generated_command));
    const QString level = QString::fromStdString(ai::to_string(r.risk));
    risk_->setObjectName(r.risk == ai::RiskLevel::Low ? QStringLiteral("Success")
                         : r.risk == ai::RiskLevel::Medium ? QStringLiteral("Warning") : QStringLiteral("Danger"));
    risk_->style()->unpolish(risk_);
    risk_->style()->polish(risk_);
    risk_->setText(r.risk_reason.empty() ? tr("Risk: %1").arg(level)
                                         : tr("Risk: %1 — %2").arg(level, QString::fromStdString(r.risk_reason)));
    explanation_->setText(QString::fromStdString(r.explanation));
    insert_->show();
    alternatives_->clear();
    for (const auto& alt : r.alternatives) alternatives_->addItem(QString::fromStdString(alt));
    alternatives_->setVisible(!r.alternatives.empty());
    if (card) card->show();
}

} // namespace zephyr::gui
