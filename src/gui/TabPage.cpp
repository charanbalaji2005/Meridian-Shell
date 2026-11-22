#include "TabPage.hpp"

#include "Motion.hpp"
#include "TerminalPane.hpp"
#include "TerminalView.hpp"

#include <QSplitter>
#include <QVBoxLayout>

namespace zephyr::gui {

TabPage::TabPage(const LaunchSpec& spec, QWidget* parent) : QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    auto* pane = makePane(spec);
    root_ = pane;
    layout_->addWidget(root_);
    active_ = pane;
}

TabPage::TabPage(const config::PaneLayout& layout, const SpecResolver& resolve, QWidget* parent)
    : QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    root_ = build(layout, resolve);
    layout_->addWidget(root_);
    const auto all = panes();
    active_ = all.isEmpty() ? nullptr : all.first();
    refreshMultiPane();
}

QWidget* TabPage::build(const config::PaneLayout& l, const SpecResolver& resolve) {
    if (!l.is_split) return makePane(resolve(l.profile, l.cwd));
    auto* sp = makeSplitter(l.side_by_side ? Qt::Horizontal : Qt::Vertical);
    for (const auto& child : l.children) sp->addWidget(build(child, resolve));
    if (l.sizes.size() == l.children.size()) {
        QList<int> sizes;
        for (int s : l.sizes) sizes << std::max(1, s);
        sp->setSizes(sizes);
    }
    return sp;
}

TerminalPane* TabPage::makePane(const LaunchSpec& spec) {
    auto* pane = new TerminalPane(spec, this);
    connect(pane, &TerminalPane::focused, this, [this](TerminalPane* p) {
        active_ = p;
        for (auto* other : panes()) other->update(); // move the focus ring
        emit titleChanged();
    });
    connect(pane, &TerminalPane::titleChanged, this, [this](TerminalPane* p) {
        if (p == active_) emit titleChanged();
    });
    // When the shell exits (e.g. `exit`), its pane closes, like any terminal.
    connect(pane, &TerminalPane::finished, this, [this](TerminalPane* p, int) {
        if (!p->view()->startFailed()) closePane(p);
    });
    connect(pane, &TerminalPane::contextMenuRequested, this, &TabPage::paneContextMenu);
    return pane;
}

QSplitter* TabPage::makeSplitter(Qt::Orientation o) {
    auto* sp = new QSplitter(o, this);
    sp->setChildrenCollapsible(false);
    sp->setHandleWidth(3);
    sp->setOpaqueResize(true);
    return sp;
}

TerminalPane* TabPage::activePane() const {
    if (active_) return active_;
    const auto all = panes();
    return all.isEmpty() ? nullptr : all.first();
}

QList<TerminalPane*> TabPage::panes() const {
    // findChildren returns depth-first creation order; walk the tree for
    // visual (left-to-right, top-to-bottom) order instead.
    QList<TerminalPane*> out;
    std::function<void(QWidget*)> walk = [&](QWidget* w) {
        if (auto* p = qobject_cast<TerminalPane*>(w)) { out << p; return; }
        if (auto* sp = qobject_cast<QSplitter*>(w)) {
            for (int i = 0; i < sp->count(); ++i) walk(sp->widget(i));
        }
    };
    if (root_) walk(root_);
    return out;
}

void TabPage::replaceInParent(QWidget* old_w, QWidget* new_w) {
    if (old_w == root_) {
        layout_->replaceWidget(old_w, new_w);
        root_ = new_w;
        return;
    }
    if (auto* sp = qobject_cast<QSplitter*>(old_w->parentWidget())) {
        const QList<int> sizes = sp->sizes();
        sp->replaceWidget(sp->indexOf(old_w), new_w);
        sp->setSizes(sizes);
    }
}

TerminalPane* TabPage::split(Qt::Orientation o) {
    TerminalPane* current = activePane();
    if (!current) return nullptr;
    LaunchSpec spec = current->view()->bridge()->spec();
    spec.cwd = current->view()->bridge()->currentDirectory();

    auto* fresh = makePane(spec);
    auto* parent_split = qobject_cast<QSplitter*>(current->parentWidget());
    if (parent_split && parent_split->orientation() == o) {
        const int idx = parent_split->indexOf(current);
        parent_split->insertWidget(idx + 1, fresh);
        QList<int> even;
        const int total = o == Qt::Horizontal ? parent_split->width() : parent_split->height();
        for (int i = 0; i < parent_split->count(); ++i) even << total / parent_split->count();
        parent_split->setSizes(even);
    } else {
        auto* sp = makeSplitter(o);
        replaceInParent(current, sp);
        sp->addWidget(current);
        sp->addWidget(fresh);
        sp->setSizes({1, 1});
    }
    refreshMultiPane();
    motion::fadeIn(fresh, motion::kNormal);
    fresh->view()->setFocus();
    active_ = fresh;
    return fresh;
}

void TabPage::closePane(TerminalPane* pane) {
    if (!pane) return;
    if (pane == root_) {
        root_ = nullptr;
        pane->hide();
        pane->deleteLater();
        emit empty();
        return;
    }
    auto* sp = qobject_cast<QSplitter*>(pane->parentWidget());
    const auto order = panes();
    const int idx = order.indexOf(pane);
    pane->hide();
    pane->setParent(nullptr); // removes it from the splitter
    pane->deleteLater();      // destroys the view -> PTY hang-up + reap

    if (sp && sp->count() == 1) {
        QWidget* survivor = sp->widget(0);
        replaceInParent(sp, survivor);
        sp->deleteLater();
    }
    const auto remaining = panes();
    if (remaining.isEmpty()) {
        emit empty();
        return;
    }
    active_ = remaining.value(std::clamp(idx, 0, static_cast<int>(remaining.size()) - 1));
    refreshMultiPane();
    focusActive();
    emit titleChanged();
}

void TabPage::focusNeighbour(int direction) {
    const auto all = panes();
    if (all.size() < 2) return;
    int idx = all.indexOf(activePane());
    idx = (idx + direction + static_cast<int>(all.size())) % static_cast<int>(all.size());
    active_ = all[idx];
    focusActive();
}

void TabPage::focusActive() {
    if (auto* p = activePane()) p->view()->setFocus();
}

void TabPage::refreshMultiPane() {
    const auto all = panes();
    for (auto* p : all) p->setMultiPane(all.size() > 1);
}

QString TabPage::title() const {
    auto* p = activePane();
    return p ? p->title() : QString();
}

bool TabPage::isSsh() const {
    auto* p = activePane();
    return p && p->view()->bridge()->spec().is_ssh;
}

bool TabPage::hasForegroundJobs() const {
    for (auto* p : panes()) {
        if (p->view()->bridge()->hasForegroundJob()) return true;
    }
    return false;
}

void TabPage::applySettings() {
    for (auto* p : panes()) p->applySettings();
}

config::PaneLayout TabPage::layoutOf(QWidget* w) const {
    if (auto* p = qobject_cast<TerminalPane*>(w)) {
        const auto* b = p->view()->bridge();
        return config::PaneLayout::leaf(b->spec().profile_name.toStdString(), b->currentDirectory().toStdString());
    }
    config::PaneLayout l;
    if (auto* sp = qobject_cast<QSplitter*>(w)) {
        l.is_split = true;
        l.side_by_side = sp->orientation() == Qt::Horizontal;
        for (int s : sp->sizes()) l.sizes.push_back(s);
        for (int i = 0; i < sp->count(); ++i) l.children.push_back(layoutOf(sp->widget(i)));
    }
    return l;
}

config::PaneLayout TabPage::layout() const {
    return root_ ? layoutOf(root_) : config::PaneLayout{};
}

} // namespace zephyr::gui
