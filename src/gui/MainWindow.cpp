#include "MainWindow.hpp"

#include "Actions.hpp"
#include "AppContext.hpp"
#include "CommandPalette.hpp"
#include "IconButton.hpp"
#include "Icons.hpp"
#include "Motion.hpp"
#include "SettingsDialog.hpp"
#include "Sidebar.hpp"
#include "TabPage.hpp"
#include "TerminalPane.hpp"
#include "TerminalView.hpp"
#include "UiTheme.hpp"
#include "WindowFrame.hpp"

#include "../config/theme.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QStyle>
#include <QWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStackedWidget>
#include <QSvgRenderer>
#include <QTabBar>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>
#include <tuple>

namespace zephyr::gui {

namespace {
QList<MainWindow*>& windows() {
    static QList<MainWindow*> w;
    return w;
}
constexpr const char* kRepoUrl = "https://github.com/charanbalaji2005/Meridian-Shell";
} // namespace

int MainWindow::windowCount() { return static_cast<int>(windows().size()); }

MainWindow::MainWindow(const config::SessionState* session, const LaunchSpec* initial, QWidget* parent)
    : QMainWindow(parent) {
    windows().append(this);
    setAttribute(Qt::WA_DeleteOnClose);
    // Lets the terminal background be translucent (window_opacity < 100)
    // on compositing desktops; everything is painted opaque otherwise.
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle(QStringLiteral("Zephyr"));
    setWindowIcon(QIcon(QStringLiteral(":/app/zephyr-terminal.svg")));
    resize(1100, 720);
    setMinimumSize(520, 340);

    buildUi();
    createActions();
    applyKeybindings();

    bool restored = false;
    if (initial) {
        openTab(*initial);
        restored = true;
    } else if (session) {
        for (const auto& tab : session->tabs) {
            auto* page = new TabPage(tab.root, [this](const std::string& p, const std::string& c) {
                return resolveSpec(p, c);
            });
            addPage(page);
            restored = true;
        }
        if (restored) tabs_->setCurrentIndex(std::clamp(session->active_tab, 0, tabs_->count() - 1));
    }
    if (!restored) openTab(defaultSpec());

    auto& ctx = AppContext::instance();
    connect(&ctx, &AppContext::settingsChanged, this, &MainWindow::applySettings);
    connect(&ctx, &AppContext::keybindingsChanged, this, &MainWindow::applyKeybindings);
    connect(&ctx, &AppContext::profilesChanged, this, [this]() {
        rebuildNewTabMenu();
        rebuildPalette();
    });
    applySettings();
    sidebar_->setExpanded(ctx.settings().sidebar_visible, false);
}

MainWindow::~MainWindow() { windows().removeAll(this); }

// ---- UI construction ------------------------------------------------------------

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("Central"));
    auto* v = new QVBoxLayout(central);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    header_ = new QWidget(central);
    header_->setObjectName(QStringLiteral("Header"));
    header_->setAttribute(Qt::WA_StyledBackground, true);
    header_->setFixedHeight(46);
    auto* h = new QHBoxLayout(header_);
    h->setContentsMargins(8, 0, 8, 0);
    h->setSpacing(2);

    auto* sidebar_btn = new IconButton(QStringLiteral("panel-left"), tr("Toggle Sidebar"), header_);
    h->addWidget(sidebar_btn);
    header_buttons_ << sidebar_btn;

    tabs_ = new QTabBar(header_);
    tabs_->setDocumentMode(true);
    tabs_->setTabsClosable(false); // custom IconButton close buttons per tab
    tabs_->setMovable(true);
    tabs_->setExpanding(false);
    tabs_->setElideMode(Qt::ElideRight);
    tabs_->setUsesScrollButtons(true);
    tabs_->setDrawBase(false);
    tabs_->setIconSize(QSize(15, 15));
    h->addWidget(tabs_, 1);

    auto* new_tab = new IconButton(QStringLiteral("plus"), tr("New Tab"), header_);
    auto* new_tab_more = new IconButton(QStringLiteral("chevron-down"), tr("New tab with profile or SSH host"), header_);
    new_tab_more->setFixedWidth(22);
    new_tab_more->setIconSize(14);
    new_tab_menu_ = new QMenu(this);
    UiTheme::polishMenu(new_tab_menu_);
    connect(new_tab_more, &QAbstractButton::clicked, this, [this, new_tab_more]() {
        rebuildNewTabMenu();
        new_tab_menu_->popup(new_tab_more->mapToGlobal(QPoint(0, new_tab_more->height())));
    });
    h->addWidget(new_tab);
    h->addWidget(new_tab_more);
    h->addSpacing(8);
    header_buttons_ << new_tab << new_tab_more;

    // Split / search / palette / settings — bound to actions in createActions().
    for (const char* id : {"split_vertical", "split_horizontal", "search", "command_palette", "settings"}) {
        auto* b = new IconButton(QString(), QString(), header_);
        b->setObjectName(QString::fromLatin1(id));
        h->addWidget(b);
        header_buttons_ << b;
    }

    // Window controls for frameless mode (the compositor draws none then).
    window_controls_ = new QWidget(header_);
    auto* wc = new QHBoxLayout(window_controls_);
    wc->setContentsMargins(8, 0, 0, 0);
    wc->setSpacing(2);
    auto* min_btn = new IconButton(QStringLiteral("minus"), tr("Minimize"), window_controls_);
    auto* max_btn = new IconButton(QStringLiteral("maximize"), tr("Maximize"), window_controls_);
    auto* close_win = new IconButton(QStringLiteral("x"), tr("Close Window"), window_controls_);
    wc->addWidget(min_btn);
    wc->addWidget(max_btn);
    wc->addWidget(close_win);
    connect(min_btn, &QAbstractButton::clicked, this, &QWidget::showMinimized);
    connect(max_btn, &QAbstractButton::clicked, this, [this]() { isMaximized() ? showNormal() : showMaximized(); });
    connect(close_win, &QAbstractButton::clicked, this, &QWidget::close);
    window_controls_->hide();
    h->addWidget(window_controls_);
    header_->installEventFilter(this);

    pages_ = new QStackedWidget(central);
    sidebar_ = new Sidebar(central);
    auto* body = new QHBoxLayout();
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);
    body->addWidget(sidebar_);
    body->addWidget(pages_, 1);

    v->addWidget(header_);
    v->addLayout(body, 1);
    frame_ = new WindowFrame(central, this);
    setCentralWidget(frame_);

    palette_ = new CommandPalette(central);

    connect(tabs_, &QTabBar::currentChanged, this, &MainWindow::onCurrentTabChanged);
    connect(tabs_, &QTabBar::tabBarDoubleClicked, this, [this](int idx) {
        if (idx < 0) openTab(defaultSpec()); // double-click empty space = new tab
    });
    tabs_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabs_, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        const int idx = tabs_->tabAt(pos);
        if (idx < 0) return;
        QMenu menu(this);
        UiTheme::polishMenu(&menu);
        menu.addAction(Icons::icon(QStringLiteral("columns-2")), tr("Split Side by Side"), this, [this, idx]() {
            tabs_->setCurrentIndex(idx);
            actions_["split_vertical"]->trigger();
        });
        menu.addAction(Icons::icon(QStringLiteral("rows-2")), tr("Split Top and Bottom"), this, [this, idx]() {
            tabs_->setCurrentIndex(idx);
            actions_["split_horizontal"]->trigger();
        });
        menu.addSeparator();
        menu.addAction(Icons::icon(QStringLiteral("x")), tr("Close Tab"), this, [this, idx]() { closeTab(idx); });
        menu.addAction(tr("Close Other Tabs"), this, [this, idx]() {
            TabPage* keep = pageAt(idx);
            for (int i = tabs_->count() - 1; i >= 0; --i) {
                if (pageAt(i) != keep) closeTab(i);
            }
        });
        menu.exec(tabs_->mapToGlobal(pos));
    });

    connect(sidebar_btn, &QAbstractButton::clicked, this, [this]() { sidebar_->setExpanded(!sidebar_->isExpanded()); });
    connect(new_tab, &QAbstractButton::clicked, this, [this]() { openTab(defaultSpec()); });
    connect(sidebar_, &Sidebar::launchProfile, this, &MainWindow::openProfile);
    connect(sidebar_, &Sidebar::launchSsh, this, &MainWindow::openSsh);
    connect(sidebar_, &Sidebar::openSettingsPage, this, &MainWindow::openSettings);
    connect(sidebar_, &Sidebar::insertText, this, [this](const QString& text) {
        if (auto* pane = currentPane()) pane->view()->insertText(text);
    });
}

void MainWindow::createActions() {
    const QHash<QString, std::function<void()>> handlers = {
        {"new_tab", [this] { openTab(defaultSpec()); }},
        {"new_window", [] { (new MainWindow())->show(); }},
        {"close_tab", [this] { closeTab(tabs_->currentIndex()); }},
        {"next_tab", [this] { if (tabs_->count()) tabs_->setCurrentIndex((tabs_->currentIndex() + 1) % tabs_->count()); }},
        {"prev_tab", [this] { if (tabs_->count()) tabs_->setCurrentIndex((tabs_->currentIndex() - 1 + tabs_->count()) % tabs_->count()); }},
        {"split_vertical", [this] { if (auto* p = currentPage()) p->split(Qt::Horizontal); }},
        {"split_horizontal", [this] { if (auto* p = currentPage()) p->split(Qt::Vertical); }},
        {"close_pane", [this] {
            auto* page = currentPage();
            auto* pane = currentPane();
            if (!page || !pane) return;
            if (pane->view()->bridge()->hasForegroundJob() && !confirmClose(tr("this pane"))) return;
            page->closePane(pane);
        }},
        {"focus_next_pane", [this] { if (auto* p = currentPage()) p->focusNeighbour(+1); }},
        {"focus_prev_pane", [this] { if (auto* p = currentPage()) p->focusNeighbour(-1); }},
        {"copy", [this] { if (auto* p = currentPane()) p->view()->copySelection(); }},
        {"paste", [this] { if (auto* p = currentPane()) p->view()->paste(); }},
        {"select_all", [this] { if (auto* p = currentPane()) p->view()->selectAll(); }},
        {"search", [this] { if (auto* p = currentPane()) p->openSearch(); }},
        {"find_next", [this] { if (auto* p = currentPane()) p->view()->findNext(); }},
        {"find_prev", [this] { if (auto* p = currentPane()) p->view()->findPrevious(); }},
        {"clear_scrollback", [this] { if (auto* p = currentPane()) p->view()->clearScrollback(); }},
        {"increase_font_size", [this] { if (auto* p = currentPane()) p->view()->zoomIn(); }},
        {"decrease_font_size", [this] { if (auto* p = currentPane()) p->view()->zoomOut(); }},
        {"reset_font_size", [this] { if (auto* p = currentPane()) p->view()->zoomReset(); }},
        {"toggle_sidebar", [this] { sidebar_->setExpanded(!sidebar_->isExpanded()); }},
        {"toggle_fullscreen", [this] { isFullScreen() ? showNormal() : showFullScreen(); }},
        {"command_palette", [this] { rebuildPalette(); palette_->popup(); }},
        {"settings", [this] { openSettings(); }},
        {"ai_popup", [this] { sidebar_->showPanel(QStringLiteral("assistant")); }},
    };
    for (const auto& info : actionCatalog()) {
        const QString id = QString::fromLatin1(info.id);
        auto* a = new QAction(QCoreApplication::translate("Actions", info.label), this);
        a->setShortcutContext(Qt::WindowShortcut);
        if (*info.icon) a->setIcon(Icons::icon(QString::fromLatin1(info.icon)));
        auto handler = handlers.value(id);
        connect(a, &QAction::triggered, this, [handler]() { if (handler) handler(); });
        addAction(a); // window-wide shortcut
        actions_.insert(id, a);
    }
    for (auto* b : header_buttons_) {
        const QString id = b->objectName();
        if (actions_.contains(id)) {
            b->setIconName(QString::fromLatin1([&] {
                for (const auto& i : actionCatalog()) if (id == QLatin1String(i.id)) return i.icon;
                return "";
            }()));
            b->setAction(actions_[id]);
        }
    }
    // The sidebar toggle and new-tab buttons mirror actions for tooltips.
    header_buttons_[0]->setToolTip(actions_["toggle_sidebar"]->text());
}

void MainWindow::applyKeybindings() {
    auto& kb = AppContext::instance().keybindings();
    for (auto it = actions_.begin(); it != actions_.end(); ++it) {
        const QString combo = QString::fromStdString(kb.get(it.key().toStdString()));
        it.value()->setShortcut(QKeySequence::fromString(combo, QKeySequence::PortableText));
    }
    auto tip = [](QAction* a) {
        const QString s = a->shortcut().toString(QKeySequence::NativeText);
        return s.isEmpty() ? a->text() : QStringLiteral("%1  (%2)").arg(a->text(), s);
    };
    header_buttons_[0]->setToolTip(tip(actions_["toggle_sidebar"]));
    header_buttons_[1]->setToolTip(tip(actions_["new_tab"]));
}

void MainWindow::refreshIcons() {
    for (const auto& info : actionCatalog()) {
        if (*info.icon) actions_[QString::fromLatin1(info.id)]->setIcon(Icons::icon(QString::fromLatin1(info.icon)));
    }
    for (int i = 0; i < tabs_->count(); ++i) {
        auto* page = pageAt(i);
        tabs_->setTabIcon(i, Icons::icon(page && page->isSsh() ? QStringLiteral("server") : QStringLiteral("terminal")));
    }
    for (auto* b : header_buttons_) b->update();
    sidebar_->applyTheme();
}

void MainWindow::applySettings() {
    const auto& s = AppContext::instance().settings();
    tabs_->setVisible(s.tab_bar_visible);
    header_->setVisible(!s.minimal_chrome);
    sidebar_->setVisible(!s.minimal_chrome);

    // Window style: glowing rounded frame and/or frameless window.
    WindowFrame::Style fs;
    fs.glow = s.glow_border;
    fs.color1 = QColor(QString::fromStdString(s.glow_color));
    fs.color2 = QColor(QString::fromStdString(s.glow_color2));
    fs.radius = s.corner_radius;
    fs.fill = UiTheme::toQColor(UiTheme::terminalTheme().background, s.window_opacity * 255 / 100);
    fs.frameless = s.frameless_window;
    frame_->setFrameStyle(fs);
    // The window itself must be see-through around the rounded frame.
    setProperty("zephyrGlow", s.glow_border);
    style()->unpolish(this);
    style()->polish(this);
    window_controls_->setVisible(s.frameless_window);
    if (s.frameless_window != frameless_) {
        frameless_ = s.frameless_window;
        const bool was_visible = isVisible();
        setWindowFlag(Qt::FramelessWindowHint, frameless_); // hides the window…
        if (was_visible) show();                           // …so show it again
    }

    refreshIcons();
    for (int i = 0; i < tabs_->count(); ++i) {
        if (auto* p = pageAt(i)) p->applySettings();
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* e) {
    // In frameless mode the empty part of the header is the title bar.
    if (obj == header_ && frameless_) {
        if (e->type() == QEvent::MouseButtonPress &&
            static_cast<QMouseEvent*>(e)->button() == Qt::LeftButton && windowHandle()) {
            windowHandle()->startSystemMove();
            return true;
        }
        if (e->type() == QEvent::MouseButtonDblClick) {
            isMaximized() ? showNormal() : showMaximized();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::rebuildNewTabMenu() {
    new_tab_menu_->clear();
    const auto& store = AppContext::instance().profiles();
    for (const auto& p : store.profiles()) {
        const QString name = QString::fromStdString(p.name);
        new_tab_menu_->addAction(Icons::icon(QStringLiteral("terminal")), name, this, [this, name]() { openProfile(name); });
    }
    if (!store.ssh_profiles().empty()) {
        new_tab_menu_->addSeparator();
        for (const auto& s : store.ssh_profiles()) {
            const QString name = QString::fromStdString(s.name);
            new_tab_menu_->addAction(Icons::icon(QStringLiteral("server")), name, this, [this, name]() { openSsh(name); });
        }
    }
    new_tab_menu_->addSeparator();
    new_tab_menu_->addAction(Icons::icon(QStringLiteral("settings")), tr("Manage Profiles…"), this,
                             [this]() { openSettings(QStringLiteral("profiles")); });
}

void MainWindow::rebuildPalette() {
    qDeleteAll(dynamic_actions_);
    dynamic_actions_.clear();
    QList<QPair<QString, QAction*>> list;
    for (const auto& info : actionCatalog()) {
        list.append({QCoreApplication::translate("Actions", info.group), actions_[QString::fromLatin1(info.id)]});
    }
    auto add_dynamic = [&](const QString& group, const QString& icon, const QString& text, std::function<void()> fn) {
        auto* a = new QAction(Icons::icon(icon), text, this);
        connect(a, &QAction::triggered, this, fn);
        dynamic_actions_.append(a);
        list.append({group, a});
    };
    const auto& store = AppContext::instance().profiles();
    for (const auto& p : store.profiles()) {
        const QString name = QString::fromStdString(p.name);
        add_dynamic(tr("Profile"), QStringLiteral("terminal"), tr("New tab: %1").arg(name), [this, name] { openProfile(name); });
    }
    for (const auto& s : store.ssh_profiles()) {
        const QString name = QString::fromStdString(s.name);
        add_dynamic(tr("SSH"), QStringLiteral("server"), tr("Connect to %1").arg(name), [this, name] { openSsh(name); });
    }
    auto set_theme = [](const std::string& id) {
        auto s = AppContext::instance().settings();
        s.theme_name = id;
        AppContext::instance().setSettings(s);
    };
    add_dynamic(tr("Theme"), QStringLiteral("palette"), tr("Zephyr (follows light/dark mode)"),
                [set_theme] { set_theme("zephyr-auto"); });
    for (const auto& t : config::builtin_themes()) {
        const std::string id = t.id;
        add_dynamic(tr("Theme"), QStringLiteral("palette"), QString::fromStdString(t.display_name), [set_theme, id] { set_theme(id); });
    }
    for (const auto& [mode, label, icon] : {std::tuple{"dark", tr("Dark mode"), "moon"},
                                             std::tuple{"light", tr("Light mode"), "sun"},
                                             std::tuple{"system", tr("Follow system appearance"), "monitor"}}) {
        const std::string m = mode;
        add_dynamic(tr("Appearance"), QString::fromLatin1(icon), label, [m] {
            auto s = AppContext::instance().settings();
            s.ui_mode = m;
            AppContext::instance().setSettings(s);
        });
    }
    for (const auto& [id, label, icon] : {std::tuple{"files", tr("Show Files"), "folder"},
                                           std::tuple{"history", tr("Show Command History"), "history"},
                                           std::tuple{"packages", tr("Show Package Manager"), "package"},
                                           std::tuple{"system", tr("Show System Information"), "cpu"},
                                           std::tuple{"profiles", tr("Show Profiles & SSH"), "user"}}) {
        const QString panel = QString::fromLatin1(id);
        add_dynamic(tr("Sidebar"), QString::fromLatin1(icon), label, [this, panel] { sidebar_->showPanel(panel); });
    }
    add_dynamic(tr("App"), QStringLiteral("folder"), tr("Open Current Folder in File Manager"), [this] {
        if (auto* p = currentPane()) QDesktopServices::openUrl(QUrl::fromLocalFile(p->view()->bridge()->currentDirectory()));
    });
    add_dynamic(tr("Look"), QStringLiteral("palette"), tr("Apply Neon Night look (glow border, minimal)"), [] {
        AppContext::instance().setSettings(config::neon_night_preset(AppContext::instance().settings()));
    });
    add_dynamic(tr("Look"), QStringLiteral("panel-left"), tr("Toggle minimal window (hide header & sidebar)"), [] {
        auto s = AppContext::instance().settings();
        s.minimal_chrome = !s.minimal_chrome;
        AppContext::instance().setSettings(s);
    });
    add_dynamic(tr("Look"), QStringLiteral("activity"), tr("Toggle glowing border"), [] {
        auto s = AppContext::instance().settings();
        s.glow_border = !s.glow_border;
        AppContext::instance().setSettings(s);
    });
    add_dynamic(tr("App"), QStringLiteral("info"), tr("About Zephyr"), [this] { showAbout(); });
    palette_->setActions(list);
}

// ---- Tabs ---------------------------------------------------------------------------

int MainWindow::addPage(TabPage* page) {
    pages_->addWidget(page);
    const int idx = tabs_->addTab(Icons::icon(page->isSsh() ? QStringLiteral("server") : QStringLiteral("terminal")),
                                  page->title());
    tabs_->setTabData(idx, QVariant::fromValue(static_cast<QObject*>(page)));
    auto* close_btn = new IconButton(QStringLiteral("x"), tr("Close Tab"), tabs_);
    close_btn->setFixedSize(20, 20);
    close_btn->setIconSize(13);
    tabs_->setTabButton(idx, QTabBar::RightSide, close_btn);
    connect(close_btn, &QAbstractButton::clicked, this, [this, page]() {
        for (int i = 0; i < tabs_->count(); ++i) {
            if (pageAt(i) == page) { closeTab(i); return; }
        }
    });
    connect(page, &TabPage::titleChanged, this, [this, page]() { updateTabTitle(page); });
    connect(page, &TabPage::empty, this, [this, page]() {
        for (int i = 0; i < tabs_->count(); ++i) {
            if (pageAt(i) == page) {
                tabs_->removeTab(i);
                pages_->removeWidget(page);
                page->deleteLater();
                break;
            }
        }
        if (tabs_->count() == 0 && !closing_) close();
    });
    connect(page, &TabPage::paneContextMenu, this, &MainWindow::showPaneMenu);
    tabs_->setCurrentIndex(idx);
    return idx;
}

void MainWindow::openTab(const LaunchSpec& spec) {
    addPage(new TabPage(spec));
    if (auto* p = currentPage()) p->focusActive();
}

void MainWindow::openProfile(const QString& name) {
    openTab(resolveSpec(name.toStdString(), std::string()));
}

void MainWindow::openSsh(const QString& name) {
    openTab(resolveSpec("ssh:" + name.toStdString(), std::string()));
}

void MainWindow::closeTab(int index) {
    TabPage* page = pageAt(index);
    if (!page) return;
    if (page->hasForegroundJobs() && !confirmClose(tr("this tab"))) return;
    tabs_->removeTab(index);
    pages_->removeWidget(page);
    page->deleteLater(); // panes -> views -> PTYs hung up and reaped
    if (tabs_->count() == 0) close();
}

TabPage* MainWindow::pageAt(int index) const {
    if (index < 0 || index >= tabs_->count()) return nullptr;
    return qobject_cast<TabPage*>(tabs_->tabData(index).value<QObject*>());
}

TabPage* MainWindow::currentPage() const { return pageAt(tabs_->currentIndex()); }

TerminalPane* MainWindow::currentPane() const {
    auto* page = currentPage();
    return page ? page->activePane() : nullptr;
}

void MainWindow::onCurrentTabChanged(int index) {
    TabPage* page = pageAt(index);
    if (!page) return;
    pages_->setCurrentWidget(page);
    motion::fadeIn(page, motion::kFast);
    page->focusActive();
    updateWindowTitle();
    if (auto* pane = page->activePane()) sidebar_->setCurrentDirectory(pane->view()->bridge()->currentDirectory());
}

void MainWindow::updateTabTitle(TabPage* page) {
    for (int i = 0; i < tabs_->count(); ++i) {
        if (pageAt(i) != page) continue;
        const QString t = page->title();
        tabs_->setTabText(i, t.isEmpty() ? tr("Terminal") : t);
        tabs_->setTabToolTip(i, t);
        if (i == tabs_->currentIndex()) {
            updateWindowTitle();
            if (auto* pane = page->activePane()) sidebar_->setCurrentDirectory(pane->view()->bridge()->currentDirectory());
        }
    }
}

void MainWindow::updateWindowTitle() {
    auto* page = currentPage();
    const QString t = page ? page->title() : QString();
    setWindowTitle(t.isEmpty() ? QStringLiteral("Zephyr") : QStringLiteral("%1 — Zephyr").arg(t));
}

void MainWindow::showPaneMenu(TerminalPane* pane, const QPoint& global_pos) {
    QMenu menu(this);
    UiTheme::polishMenu(&menu);
    actions_["copy"]->setEnabled(pane->view()->hasSelection());
    for (const char* id : {"copy", "paste", "select_all"}) menu.addAction(actions_[id]);
    menu.addSeparator();
    for (const char* id : {"search", "clear_scrollback"}) menu.addAction(actions_[id]);
    menu.addSeparator();
    for (const char* id : {"split_vertical", "split_horizontal", "close_pane"}) menu.addAction(actions_[id]);
    menu.addSeparator();
    menu.addAction(Icons::icon(QStringLiteral("folder")), tr("Open Folder in File Manager"), this, [pane]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(pane->view()->bridge()->currentDirectory()));
    });
    if (AppContext::instance().settings().minimal_chrome) {
        menu.addAction(Icons::icon(QStringLiteral("panel-left")), tr("Show Header && Sidebar"), this, []() {
            auto s = AppContext::instance().settings();
            s.minimal_chrome = false;
            AppContext::instance().setSettings(s);
        });
    }
    menu.exec(global_pos);
    actions_["copy"]->setEnabled(true);
}

// ---- Dialogs ------------------------------------------------------------------------

void MainWindow::openSettings(const QString& page) {
    if (!settings_) {
        settings_ = new SettingsDialog(this);
        settings_->setAttribute(Qt::WA_DeleteOnClose);
        connect(settings_, &SettingsDialog::connectSsh, this, &MainWindow::openSsh);
        connect(settings_, &SettingsDialog::reopenRequested, this, [this](const QString& p) {
            settings_->hide();
            settings_->deleteLater(); // safe: runs after the emitting slot returns
            settings_ = nullptr;
            openSettings(p);
        });
    }
    if (!page.isEmpty()) settings_->showPage(page);
    settings_->show();
    settings_->raise();
    settings_->activateWindow();
}

void MainWindow::showAbout() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("About Zephyr"));
    auto* v = new QVBoxLayout(&dlg);
    v->setContentsMargins(32, 28, 32, 24);
    v->setSpacing(10);
    auto* logo = new QLabel(&dlg);
    QPixmap pm(96 * devicePixelRatioF(), 96 * devicePixelRatioF());
    pm.fill(Qt::transparent);
    {
        QPainter p(&pm);
        QSvgRenderer(QStringLiteral(":/app/zephyr-terminal.svg")).render(&p);
    }
    pm.setDevicePixelRatio(devicePixelRatioF());
    logo->setPixmap(pm);
    logo->setAlignment(Qt::AlignCenter);
    auto* name = new QLabel(QStringLiteral("Zephyr Terminal"), &dlg);
    name->setObjectName(QStringLiteral("Heading"));
    name->setAlignment(Qt::AlignCenter);
    auto* info = new QLabel(tr("Version %1 · Qt %2<br>A fast, native Linux terminal emulator.<br><br>"
                               "License: GPL-3.0-or-later<br>Icons: Lucide (ISC)<br><br>"
                               "<a href=\"%3\">%3</a>")
                                .arg(QStringLiteral(ZEPHYR_VERSION), QString::fromLatin1(qVersion()), QLatin1String(kRepoUrl)),
                            &dlg);
    info->setObjectName(QStringLiteral("Muted"));
    info->setAlignment(Qt::AlignCenter);
    info->setOpenExternalLinks(true);
    auto* ok = new QPushButton(tr("Close"), &dlg);
    ok->setObjectName(QStringLiteral("Primary"));
    connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
    v->addWidget(logo);
    v->addWidget(name);
    v->addWidget(info);
    v->addWidget(ok, 0, Qt::AlignCenter);
    dlg.exec();
}

bool MainWindow::confirmClose(const QString& what) {
    if (!AppContext::instance().settings().confirm_close_running) return true;
    QMessageBox box(QMessageBox::Question, tr("Close %1?").arg(what),
                    tr("A program is still running in %1. Closing will stop it.").arg(what),
                    QMessageBox::Cancel, this);
    QAbstractButton* close_btn = box.addButton(tr("Close"), QMessageBox::DestructiveRole);
    box.setDefaultButton(QMessageBox::Cancel);
    box.exec();
    return box.clickedButton() == close_btn;
}

// ---- Launch specs & session ----------------------------------------------------------

LaunchSpec MainWindow::defaultSpec() const {
    return resolveSpec(AppContext::instance().settings().default_profile, std::string());
}

LaunchSpec MainWindow::resolveSpec(const std::string& profile, const std::string& cwd) const {
    const auto& ctx = AppContext::instance();
    const auto& store = ctx.profiles();
    LaunchSpec spec;
    if (profile.rfind("ssh:", 0) == 0) {
        if (const auto* s = store.find_ssh(profile.substr(4))) return LaunchSpec::fromSsh(*s);
    }
    if (const auto* p = store.find(profile)) spec = LaunchSpec::fromProfile(*p);
    else spec = LaunchSpec::fromProfile(ctx.defaultProfile());
    if (!cwd.empty()) spec.cwd = QString::fromStdString(cwd);
    if (spec.cwd.isEmpty()) spec.cwd = QString::fromStdString(ctx.settings().startup_directory);
    return spec;
}

config::SessionState MainWindow::sessionState() const {
    config::SessionState st;
    for (int i = 0; i < tabs_->count(); ++i) {
        if (auto* page = pageAt(i)) st.tabs.push_back({page->title().toStdString(), page->layout()});
    }
    st.active_tab = std::max(0, tabs_->currentIndex());
    return st;
}

void MainWindow::closeEvent(QCloseEvent* e) {
    bool running = false;
    for (int i = 0; i < tabs_->count(); ++i) {
        if (auto* p = pageAt(i)) running = running || p->hasForegroundJobs();
    }
    if (running && !confirmClose(tr("this window"))) {
        e->ignore();
        return;
    }
    // The last window to close records the session for next launch.
    auto& ctx = AppContext::instance();
    if (windows().size() == 1) {
        // Closing the last tab means "start fresh next time".
        if (ctx.settings().restore_session && tabs_->count() > 0) ctx.sessionStore().save(sessionState());
        else ctx.sessionStore().clear();
    }
    closing_ = true;
    e->accept();
}

} // namespace zephyr::gui
