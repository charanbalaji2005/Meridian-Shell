#include "WindowFrame.hpp"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWindow>

#include <algorithm>
#include <cmath>

namespace zephyr::gui {

namespace {
constexpr int kResizeGrip = 8; // px from the outer edge that resizes instead of moves
}

WindowFrame::WindowFrame(QWidget* content, QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("WindowFrame"));
    setMouseTracking(true);
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    layout_->addWidget(content);
}

QRectF WindowFrame::borderRect() const {
    return QRectF(rect()).adjusted(kGlow, kGlow, -kGlow, -kGlow);
}

void WindowFrame::setFrameStyle(const Style& s) {
    style_ = s;
    int margin = 0;
    if (s.glow) {
        // Inset the content so its square corners stay inside the rounded
        // border: 2px border + ~30% of the radius (1 - 1/sqrt(2)).
        margin = kGlow + 2 + std::max(3, static_cast<int>(std::ceil(s.radius * 0.3)));
    } else if (s.frameless) {
        margin = 4; // thin grip so a frameless window can still be resized
    }
    layout_->setContentsMargins(margin, margin, margin, margin);
    update();
}

void WindowFrame::paintEvent(QPaintEvent* e) {
    if (!style_.glow) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = borderRect();
    const qreal radius = style_.radius;

    QPainterPath body;
    body.addRoundedRect(r, radius, radius);
    p.fillPath(body, style_.fill);

    // Repaints caused by terminal output only cover the content area, which
    // lies entirely inside the border (setFrameStyle insets it past the
    // corner curve), so the glow strokes can be skipped for them.
    const QMargins m = layout_->contentsMargins();
    if (rect().marginsRemoved(m).contains(e->rect())) return;

    // Soft outer glow: concentric strokes fading out (quadratic falloff).
    for (int i = kGlow; i >= 1; --i) {
        const qreal t = 1.0 - static_cast<qreal>(i) / (kGlow + 1);
        const int alpha = static_cast<int>(110 * t * t);
        QColor a = style_.color1, b = style_.color2;
        a.setAlpha(alpha);
        b.setAlpha(alpha);
        const QRectF ring = r.adjusted(-i, -i, i, i);
        QLinearGradient g(ring.topLeft(), ring.bottomRight());
        g.setColorAt(0, a);
        g.setColorAt(1, b);
        p.setPen(QPen(QBrush(g), 2.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(ring, radius + i, radius + i);
    }
    // Crisp border on top.
    QLinearGradient g(r.topLeft(), r.bottomRight());
    g.setColorAt(0, style_.color1);
    g.setColorAt(1, style_.color2);
    p.setPen(QPen(QBrush(g), 1.6));
    p.drawRoundedRect(r.adjusted(0.8, 0.8, -0.8, -0.8), radius, radius);
}

Qt::Edges WindowFrame::edgesAt(const QPoint& pt) const {
    Qt::Edges edges;
    if (pt.x() <= kResizeGrip) edges |= Qt::LeftEdge;
    if (pt.x() >= width() - kResizeGrip) edges |= Qt::RightEdge;
    if (pt.y() <= kResizeGrip) edges |= Qt::TopEdge;
    if (pt.y() >= height() - kResizeGrip) edges |= Qt::BottomEdge;
    return edges;
}

void WindowFrame::mousePressEvent(QMouseEvent* e) {
    QWindow* win = window()->windowHandle();
    if (!style_.frameless || !win || e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }
    const Qt::Edges edges = edgesAt(e->pos());
    if (edges) win->startSystemResize(edges);
    else win->startSystemMove(); // the glow margin is a move handle
    e->accept();
}

void WindowFrame::mouseMoveEvent(QMouseEvent* e) {
    if (style_.frameless && !(e->buttons() & Qt::LeftButton)) {
        const Qt::Edges edges = edgesAt(e->pos());
        const bool h = edges & (Qt::LeftEdge | Qt::RightEdge);
        const bool v = edges & (Qt::TopEdge | Qt::BottomEdge);
        if (h && v) {
            // top-left / bottom-right corners: "\" cursor; the other two: "/".
            const bool main_diag = ((edges & Qt::LeftEdge) && (edges & Qt::TopEdge)) ||
                                   ((edges & Qt::RightEdge) && (edges & Qt::BottomEdge));
            setCursor(main_diag ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
        } else if (h) {
            setCursor(Qt::SizeHorCursor);
        } else if (v) {
            setCursor(Qt::SizeVerCursor);
        } else {
            unsetCursor();
        }
    }
    QWidget::mouseMoveEvent(e);
}

void WindowFrame::mouseDoubleClickEvent(QMouseEvent* e) {
    if (style_.frameless && !edgesAt(e->pos())) {
        QWidget* w = window();
        w->isMaximized() ? w->showNormal() : w->showMaximized();
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

void WindowFrame::leaveEvent(QEvent* e) {
    unsetCursor();
    QWidget::leaveEvent(e);
}

} // namespace zephyr::gui
