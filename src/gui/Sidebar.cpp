#include "Sidebar.hpp"

#include "IconButton.hpp"
#include "Motion.hpp"
#include "SidebarPanels.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace zephyr::gui {

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    rail_ = new QWidget(this);
    rail_->setObjectName(QStringLiteral("SidebarRail"));
    rail_->setAttribute(Qt::WA_StyledBackground, true);
    rail_->setFixedWidth(48);
    auto* rail_lay = new QVBoxLayout(rail_);
    rail_lay->setContentsMargins(8, 10, 8, 10);
    rail_lay->setSpacing(6);
    rail_lay->addStretch(1);

    panel_area_ = new QWidget(this);
    panel_area_->setObjectName(QStringLiteral("Sidebar"));
    panel_area_->setAttribute(Qt::WA_StyledBackground, true);
    panel_area_->setMinimumWidth(0);
    panel_area_->setMaximumWidth(kPanelWidth);
    auto* panel_lay = new QVBoxLayout(panel_area_);
    panel_lay->setContentsMargins(0, 12, 0, 0);
    panel_lay->setSpacing(4);
    panel_title_ = new QLabel(panel_area_);
    panel_title_->setObjectName(QStringLiteral("SidebarTitle"));
    panel_title_->setContentsMargins(14, 0, 0, 4);
    stack_ = new QStackedWidget(panel_area_);
    panel_lay->addWidget(panel_title_);
    panel_lay->addWidget(stack_, 1);

    lay->addWidget(rail_);
    lay->addWidget(panel_area_);

    auto* profiles = new ProfilesPanel(this);
    auto* files = new FilesPanel(this);
    auto* history = new HistoryPanel(this);
    auto* system = new SystemPanel(this);
    auto* packages = new PackagesPanel(this);
    auto* assistant = new AssistantPanel(this);
    addPanel(QStringLiteral("profiles"), QStringLiteral("user"), tr("Profiles & SSH"), profiles);
    addPanel(QStringLiteral("files"), QStringLiteral("folder"), tr("Files"), files);
    addPanel(QStringLiteral("history"), QStringLiteral("history"), tr("Command History"), history);
    addPanel(QStringLiteral("packages"), QStringLiteral("package"), tr("Packages"), packages);
    addPanel(QStringLiteral("system"), QStringLiteral("cpu"), tr("System Information"), system);
    addPanel(QStringLiteral("assistant"), QStringLiteral("bot"), tr("Command Assistant"), assistant);

    connect(profiles, &ProfilesPanel::launchProfile, this, &Sidebar::launchProfile);
    connect(profiles, &ProfilesPanel::launchSsh, this, &Sidebar::launchSsh);
    connect(profiles, &ProfilesPanel::manage, this, &Sidebar::openSettingsPage);
    connect(files, &FilesPanel::insertText, this, &Sidebar::insertText);
    connect(history, &HistoryPanel::insertText, this, &Sidebar::insertText);
    connect(packages, &PackagesPanel::insertText, this, &Sidebar::insertText);
    connect(assistant, &AssistantPanel::insertText, this, &Sidebar::insertText);
    showPanel(QStringLiteral("profiles"));
}

void Sidebar::addPanel(const QString& id, const QString& icon, const QString& title, QWidget* panel) {
    auto* btn = new IconButton(icon, title, rail_);
    btn->setCheckable(true);
    auto* rail_lay = static_cast<QVBoxLayout*>(rail_->layout());
    rail_lay->insertWidget(rail_lay->count() - 1, btn); // keep the stretch last
    stack_->addWidget(panel);
    entries_.append({id, icon, title, btn, panel});
    connect(btn, &QAbstractButton::clicked, this, [this, id]() {
        if (current_ == id && expanded_) setExpanded(false);
        else showPanel(id);
    });
}

void Sidebar::showPanel(const QString& id) {
    for (const auto& e : entries_) {
        const bool active = e.id == id;
        e.button->setChecked(active);
        if (active) {
            stack_->setCurrentWidget(e.panel);
            panel_title_->setText(e.title.toUpper());
            current_ = id;
            if (expanded_) motion::fadeIn(e.panel, motion::kFast);
        }
    }
    if (!expanded_) setExpanded(true);
}

void Sidebar::setExpanded(bool expanded, bool animate) {
    if (expanded == expanded_ && panel_area_->isVisible() == expanded) return;
    expanded_ = expanded;
    for (const auto& e : entries_) e.button->setChecked(expanded && e.id == current_);
    if (expanded) {
        panel_area_->setMaximumWidth(0);
        panel_area_->show();
        if (animate) motion::animateWidth(panel_area_, kPanelWidth);
        else panel_area_->setMaximumWidth(kPanelWidth);
    } else if (animate) {
        motion::animateWidth(panel_area_, 0, motion::kNormal, [this]() {
            if (!expanded_) panel_area_->hide();
        });
    } else {
        panel_area_->hide();
    }
    emit expandedChanged(expanded);
}

void Sidebar::setCurrentDirectory(const QString& dir) {
    for (const auto& e : entries_) {
        if (auto* f = qobject_cast<FilesPanel*>(e.panel)) f->setTerminalDirectory(dir);
        if (auto* a = qobject_cast<AssistantPanel*>(e.panel)) a->setWorkingDirectory(dir);
    }
}

void Sidebar::refreshProfiles() {
    for (const auto& e : entries_) {
        if (auto* p = qobject_cast<ProfilesPanel*>(e.panel)) p->refresh();
    }
}

void Sidebar::applyTheme() {
    for (const auto& e : entries_) e.button->update();
    refreshProfiles(); // re-renders item icons in the new colors
}

} // namespace zephyr::gui
