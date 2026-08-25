#include "TabManager.hpp"

namespace meridian::gui {

TerminalTab::TerminalTab(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    // Top: Artwork + Live Linux SysInfo + Clock
    header_ = new HeaderSection(this);

    // Bottom: Real PTY Terminal View
    bridge_ = std::make_shared<PtyBridge>(24, 80, this);
    bridge_->startSession();

    terminal_view_ = new TerminalView(bridge_, this);
    terminal_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(bridge_.get(), &PtyBridge::screenUpdated, this, [this]() {
        header_->updateWorkingDir(bridge_->currentWorkingDir());
    });

    layout->addWidget(header_, 0);
    layout->addWidget(terminal_view_, 1);
}

TabManager::TabManager(QWidget* parent)
    : QTabWidget(parent)
{
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    setStyleSheet(
        "QTabWidget::pane { border: none; background: transparent; } "
        "QTabBar::tab { background: #18191c; color: #7f848e; padding: 6px 16px; border-radius: 4px; margin-right: 4px; } "
        "QTabBar::tab:selected { background: #23272e; color: #abb2bf; font-weight: bold; border-bottom: 2px solid #61afef; } "
        "QTabBar::tab:hover { background: #1e2227; color: #d8dee9; }"
    );

    connect(this, &QTabWidget::tabCloseRequested, this, [this](int idx) {
        if (count() > 1) {
            removeTab(idx);
        }
    });

    createNewTab("main");
}

TerminalTab* TabManager::createNewTab(const QString& title) {
    auto* tab = new TerminalTab(this);
    int idx = addTab(tab, title);
    setCurrentIndex(idx);
    return tab;
}

TerminalTab* TabManager::currentTerminalTab() {
    return qobject_cast<TerminalTab*>(currentWidget());
}

} // namespace meridian::gui

