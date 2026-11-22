#include "CommandPalette.hpp"

#include "Icons.hpp"
#include "Motion.hpp"
#include "UiTheme.hpp"

#include "../dev/command_palette.hpp"

#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace zephyr::gui {

namespace {
constexpr int kActionRole = Qt::UserRole + 1;
}

CommandPalette::CommandPalette(QWidget* parent) : QFrame(parent) {
    setObjectName(QStringLiteral("Palette"));
    setAttribute(Qt::WA_StyledBackground, true);
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(10, 6, 10, 10);
    lay->setSpacing(4);

    auto* row = new QHBoxLayout();
    auto* icon = new QLabel(this);
    icon->setPixmap(Icons::pixmap(QStringLiteral("command"), UiTheme::colors().muted, 18, devicePixelRatioF()));
    edit_ = new QLineEdit(this);
    edit_->setPlaceholderText(tr("Type a command, profile, host or theme…"));
    edit_->setAccessibleName(tr("Command palette search"));
    row->addWidget(icon);
    row->addWidget(edit_, 1);
    lay->addLayout(row);

    list_ = new QListWidget(this);
    list_->setUniformItemSizes(true);
    list_->setIconSize(QSize(16, 16));
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lay->addWidget(list_, 1);

    connect(edit_, &QLineEdit::textChanged, this, &CommandPalette::refilter);
    connect(list_, &QListWidget::itemActivated, this, [this](QListWidgetItem*) { runCurrent(); });
    edit_->installEventFilter(this);
    parent->installEventFilter(this); // follow window resizes
    // Keep keyboard focus in the search field; the list is driven from it.
    list_->setFocusPolicy(Qt::NoFocus);
    hide();
}

void CommandPalette::setActions(const QList<QPair<QString, QAction*>>& grouped) {
    actions_.clear();
    for (const auto& [group, action] : grouped) actions_.append({group, QPointer<QAction>(action)});
}

void CommandPalette::reposition() {
    QWidget* p = parentWidget();
    const int w = std::min(620, p->width() - 40);
    const int h = std::min(420, p->height() - 80);
    setFixedSize(std::max(320, w), std::max(200, h));
    move((p->width() - width()) / 2, 56);
}

void CommandPalette::popup() {
    reposition();
    edit_->clear();
    refilter(QString());
    motion::popIn(this, -10, motion::kNormal);
    edit_->setFocus();
}

void CommandPalette::dismiss() {
    if (!isVisible()) return;
    motion::fadeOut(this, motion::kFast);
    if (parentWidget()) parentWidget()->setFocus();
}

void CommandPalette::refilter(const QString& query) {
    struct Hit { int score; int order; QString group; QAction* action; };
    std::vector<Hit> hits;
    const std::string q = query.trimmed().toStdString();
    int order = 0;
    for (const auto& [group, action] : actions_) {
        ++order;
        if (!action || !action->isEnabled() || action->text().isEmpty()) continue;
        const QString label = action->text().remove('&');
        int score = 1;
        if (!q.empty()) {
            score = std::max(dev::CommandPalette::fuzzy_score(q, label.toStdString()) * 2,
                             dev::CommandPalette::fuzzy_score(q, (group + " " + label).toStdString()));
            if (score <= 0) continue;
        }
        hits.push_back({score, order, group, action});
    }
    std::stable_sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) {
        return a.score != b.score ? a.score > b.score : a.order < b.order;
    });

    list_->clear();
    for (const auto& h : hits) {
        QString text = h.group.isEmpty() ? h.action->text().remove('&')
                                         : QStringLiteral("%1: %2").arg(h.group, h.action->text().remove('&'));
        const QString shortcut = h.action->shortcut().toString(QKeySequence::NativeText);
        if (!shortcut.isEmpty()) text += QStringLiteral("    ") + shortcut;
        auto* item = new QListWidgetItem(h.action->icon(), text, list_);
        item->setData(kActionRole, QVariant::fromValue(static_cast<QObject*>(h.action)));
        item->setToolTip(h.action->toolTip());
    }
    if (list_->count() > 0) list_->setCurrentRow(0);
}

void CommandPalette::runCurrent() {
    auto* item = list_->currentItem();
    if (!item) return;
    QPointer<QAction> action = qobject_cast<QAction*>(item->data(kActionRole).value<QObject*>());
    dismiss();
    if (action) action->trigger();
}

bool CommandPalette::eventFilter(QObject* obj, QEvent* e) {
    if (obj == parentWidget() && e->type() == QEvent::Resize && isVisible()) reposition();
    if (obj == edit_ && e->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(e);
        switch (ke->key()) {
            case Qt::Key_Escape: dismiss(); return true;
            case Qt::Key_Return: case Qt::Key_Enter: runCurrent(); return true;
            case Qt::Key_Down:
                list_->setCurrentRow(std::min(list_->currentRow() + 1, list_->count() - 1));
                return true;
            case Qt::Key_Up:
                list_->setCurrentRow(std::max(list_->currentRow() - 1, 0));
                return true;
            case Qt::Key_PageDown:
                list_->setCurrentRow(std::min(list_->currentRow() + 8, list_->count() - 1));
                return true;
            case Qt::Key_PageUp:
                list_->setCurrentRow(std::max(list_->currentRow() - 8, 0));
                return true;
            default: break;
        }
    }
    if (obj == edit_ && e->type() == QEvent::FocusOut && isVisible()) {
        dismiss(); // clicking outside closes the palette
    }
    return QFrame::eventFilter(obj, e);
}

} // namespace zephyr::gui
