#include "TerminalPane.hpp"

#include "IconButton.hpp"
#include "Motion.hpp"
#include "TerminalView.hpp"
#include "UiTheme.hpp"

#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QScrollBar>

namespace zephyr::gui {

// ---- SearchBar -----------------------------------------------------------------

SearchBar::SearchBar(QWidget* parent) : QFrame(parent) {
    setObjectName(QStringLiteral("SearchBar"));
    setAttribute(Qt::WA_StyledBackground, true);
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 3, 4, 3);
    lay->setSpacing(2);

    edit_ = new QLineEdit(this);
    edit_->setPlaceholderText(tr("Find in output"));
    edit_->setMinimumWidth(200);
    edit_->setAccessibleName(tr("Search text"));
    count_ = new QLabel(this);
    count_->setObjectName(QStringLiteral("Muted"));
    count_->setMinimumWidth(56);
    count_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    case_btn_ = new IconButton(QStringLiteral("case-sensitive"), tr("Match case"), this);
    case_btn_->setCheckable(true);
    auto* prev = new IconButton(QStringLiteral("chevron-up"), tr("Previous match  (Shift+Enter)"), this);
    auto* next = new IconButton(QStringLiteral("chevron-down"), tr("Next match  (Enter)"), this);
    auto* close = new IconButton(QStringLiteral("x"), tr("Close  (Esc)"), this);

    lay->addWidget(edit_, 1);
    lay->addWidget(count_);
    lay->addWidget(case_btn_);
    lay->addWidget(prev);
    lay->addWidget(next);
    lay->addWidget(close);

    connect(edit_, &QLineEdit::textChanged, this, [this](const QString& q) {
        emit queryChanged(q, case_btn_->isChecked());
    });
    connect(case_btn_, &QAbstractButton::toggled, this, [this](bool on) { emit queryChanged(edit_->text(), on); });
    connect(prev, &QAbstractButton::clicked, this, &SearchBar::previous);
    connect(next, &QAbstractButton::clicked, this, &SearchBar::next);
    connect(close, &QAbstractButton::clicked, this, &SearchBar::closed);
    hide();
}

void SearchBar::open(const QString& initial) {
    if (!initial.isEmpty() && !initial.contains('\n')) edit_->setText(initial);
    adjustSize();
    if (!isVisible()) motion::popIn(this, -6, motion::kFast);
    edit_->setFocus();
    edit_->selectAll();
    if (!edit_->text().isEmpty()) emit queryChanged(edit_->text(), case_btn_->isChecked());
}

QString SearchBar::query() const { return edit_->text(); }
bool SearchBar::caseSensitive() const { return case_btn_->isChecked(); }

void SearchBar::setResult(int current, int total) {
    if (edit_->text().isEmpty()) count_->clear();
    else if (total == 0) count_->setText(tr("No results"));
    else count_->setText(QStringLiteral("%1 / %2").arg(current + 1).arg(total));
}

void SearchBar::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) { emit closed(); return; }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        if (e->modifiers() & Qt::ShiftModifier) emit previous();
        else emit next();
        return;
    }
    QFrame::keyPressEvent(e);
}

// ---- TerminalPane ---------------------------------------------------------------

TerminalPane::TerminalPane(const LaunchSpec& spec, QWidget* parent) : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(1, 1, 1, 1);
    lay->setSpacing(0);
    view_ = new TerminalView(spec, this);
    scrollbar_ = new QScrollBar(Qt::Vertical, this);
    scrollbar_->setAccessibleName(tr("Scrollback"));
    lay->addWidget(view_, 1);
    lay->addWidget(scrollbar_);
    search_ = new SearchBar(this);

    connect(view_, &TerminalView::viewChanged, this, &TerminalPane::syncScrollbar);
    connect(scrollbar_, &QScrollBar::valueChanged, this, [this](int v) {
        if (syncing_) return;
        view_->setViewOffset(scrollbar_->maximum() - v);
    });
    connect(view_, &TerminalView::focused, this, [this]() {
        update();
        emit focused(this);
    });
    connect(view_, &TerminalView::finished, this, [this](int code) { emit finished(this, code); });
    connect(view_, &TerminalView::titleChanged, this, [this]() { emit titleChanged(this); });
    connect(view_, &TerminalView::contextMenuRequested, this,
            [this](const QPoint& pos) { emit contextMenuRequested(this, pos); });

    connect(search_, &SearchBar::queryChanged, view_, &TerminalView::setSearch);
    connect(search_, &SearchBar::next, view_, &TerminalView::findNext);
    connect(search_, &SearchBar::previous, view_, &TerminalView::findPrevious);
    connect(search_, &SearchBar::closed, this, &TerminalPane::closeSearch);
    connect(view_, &TerminalView::searchResultsChanged, search_, &SearchBar::setResult);
    syncScrollbar();
}

QString TerminalPane::title() const {
    const QString osc = view_->bridge()->title();
    if (!osc.isEmpty()) return osc;
    const auto& spec = view_->bridge()->spec();
    if (spec.is_ssh) return spec.profile_name.mid(4);
    QString dir = view_->bridge()->currentDirectory();
    if (dir == QDir::homePath()) return QStringLiteral("~");
    return QFileInfo(dir).fileName().isEmpty() ? dir : QFileInfo(dir).fileName();
}

void TerminalPane::setMultiPane(bool multi) {
    multi_pane_ = multi;
    update();
}

void TerminalPane::openSearch() {
    placeSearchBar();
    search_->open(view_->selectedText());
}

void TerminalPane::closeSearch() {
    view_->clearSearch();
    motion::fadeOut(search_, motion::kFast);
    view_->setFocus();
}

void TerminalPane::applySettings() {
    view_->applySettings();
    update();
}

void TerminalPane::syncScrollbar() {
    syncing_ = true;
    const int history = view_->historySize();
    scrollbar_->setRange(0, history);
    scrollbar_->setPageStep(std::max(1, view_->bridge()->screen().rows()));
    scrollbar_->setValue(history - view_->viewOffset());
    scrollbar_->setVisible(history > 0);
    syncing_ = false;
}

void TerminalPane::placeSearchBar() {
    search_->adjustSize();
    const int w = std::min(search_->sizeHint().width() + 40, width() - 24);
    search_->setFixedWidth(std::max(220, w));
    search_->move(width() - search_->width() - 18, 10);
}

void TerminalPane::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (search_->isVisible()) placeSearchBar();
}

void TerminalPane::paintEvent(QPaintEvent*) {
    if (!multi_pane_ || !view_->hasFocus()) return;
    QPainter p(this);
    QColor ring = UiTheme::colors().accent;
    ring.setAlpha(170);
    p.setPen(QPen(ring, 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

} // namespace zephyr::gui
