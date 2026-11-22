#include "IconButton.hpp"

#include "AppContext.hpp"
#include "Icons.hpp"
#include "UiTheme.hpp"

#include <QAction>
#include <QPainter>
#include <QPainterPath>

namespace zephyr::gui {

IconButton::IconButton(const QString& icon_name, const QString& tooltip, QWidget* parent)
    : QAbstractButton(parent), icon_name_(icon_name), base_tooltip_(tooltip) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);
    setToolTip(tooltip);
    setAccessibleName(tooltip);
    setFixedSize(32, 32);
    hover_anim_.setEasingCurve(QEasingCurve::OutCubic);
    connect(&hover_anim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        hover_ = v.toReal();
        update();
    });
}

void IconButton::setIconName(const QString& name) {
    icon_name_ = name;
    update();
}

void IconButton::setAction(QAction* action) {
    action_ = action;
    if (!action) return;
    base_tooltip_ = action->text().remove('&');
    setCheckable(action->isCheckable());
    connect(this, &QAbstractButton::clicked, action, [action]() { action->trigger(); });
    connect(action, &QAction::changed, this, &IconButton::syncFromAction);
    syncFromAction();
}

void IconButton::syncFromAction() {
    if (!action_) return;
    setEnabled(action_->isEnabled());
    if (action_->isCheckable()) setChecked(action_->isChecked());
    const QString shortcut = action_->shortcut().toString(QKeySequence::NativeText);
    setToolTip(shortcut.isEmpty() ? base_tooltip_ : QStringLiteral("%1  (%2)").arg(base_tooltip_, shortcut));
    setAccessibleName(base_tooltip_);
}

void IconButton::animateHover(qreal to) {
    const int ms = AppContext::instance().motionMs(140);
    hover_anim_.stop();
    if (ms == 0) {
        hover_ = to;
        update();
        return;
    }
    hover_anim_.setDuration(ms);
    hover_anim_.setStartValue(hover_);
    hover_anim_.setEndValue(to);
    hover_anim_.start();
}

void IconButton::enterEvent(QEnterEvent* e) {
    animateHover(1.0);
    QAbstractButton::enterEvent(e);
}

void IconButton::leaveEvent(QEvent* e) {
    animateHover(0.0);
    QAbstractButton::leaveEvent(e);
}

void IconButton::paintEvent(QPaintEvent*) {
    const auto c = UiTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF r = rect().adjusted(2, 2, -2, -2);
    if (isDown()) r.adjust(0.5, 0.5, -0.5, -0.5); // tactile press
    QPainterPath path;
    path.addRoundedRect(r, 8, 8);

    if (isChecked()) {
        QColor fill = c.accent;
        fill.setAlpha(46);
        p.fillPath(path, fill);
    }
    if (hover_ > 0.0) {
        QColor fill = c.hover;
        fill.setAlphaF(fill.alphaF() * hover_ * (isDown() ? 1.8 : 1.0));
        p.fillPath(path, fill);
    }
    if (hasFocus()) {
        QColor ring = c.accent;
        ring.setAlpha(160);
        p.setPen(QPen(ring, 1.2));
        p.drawPath(path);
    }

    QColor icon_color = !isEnabled() ? QColor(c.muted.red(), c.muted.green(), c.muted.blue(), 110)
                      : isChecked()  ? c.accent
                                     : QColor::fromRgbF(c.muted.redF() + (c.text.redF() - c.muted.redF()) * hover_,
                                                        c.muted.greenF() + (c.text.greenF() - c.muted.greenF()) * hover_,
                                                        c.muted.blueF() + (c.text.blueF() - c.muted.blueF()) * hover_);
    // Quantize so the pixmap cache stays small during the fade.
    icon_color = QColor(icon_color.red() & ~7, icon_color.green() & ~7, icon_color.blue() & ~7, icon_color.alpha());
    const QPixmap pm = Icons::pixmap(icon_name_, icon_color, icon_px_, devicePixelRatioF());
    const QPointF tl((width() - icon_px_) / 2.0, (height() - icon_px_) / 2.0);
    p.drawPixmap(tl, pm);
}

} // namespace zephyr::gui
