#include "ClockWidget.hpp"

#include <QDateTime>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <pwd.h>
#include <unistd.h>

namespace meridian::gui {

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(60);

    // Initial username detection
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        username_ = QString::fromUtf8(pw->pw_name);
    }
    command_idx_ = static_cast<int>(getpid());

    updateTime();
    connect(&timer_, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer_.start(1000);
}

void ClockWidget::updateTime() {
    QDateTime now = QDateTime::currentDateTime();
    date_time_str_ = now.toString("ddd d MMM  -  hh:mm");
    update();
}

void ClockWidget::setCurrentDirectory(const QString& dir) {
    current_dir_ = dir;
    update();
}

void ClockWidget::setUsername(const QString& user) {
    username_ = user;
    update();
}

void ClockWidget::setCommandIndex(int idx) {
    command_idx_ = idx;
    update();
}

QSize ClockWidget::sizeHint() const {
    return QSize(400, 60);
}

void ClockWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QFont font("JetBrains Mono", 10);
    font.setStyleHint(QFont::Monospace);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);

    int y1 = 4;
    int h1 = 24;
    int arrowW = 12;

    // --- ROW 1: Date/Time Badge (Dark Cyan) + Dir Badge (Muted Olive) ---
    int padX = 10;
    int textW1 = fm.horizontalAdvance(date_time_str_) + padX * 2;

    // Segment 1 (Date/Time)
    QPainterPath p1;
    p1.moveTo(0, y1);
    p1.lineTo(textW1, y1);
    p1.lineTo(textW1 + arrowW, y1 + h1 / 2.0);
    p1.lineTo(textW1, y1 + h1);
    p1.lineTo(0, y1 + h1);
    p1.closeSubpath();

    painter.fillPath(p1, QBrush(QColor("#2d6575"))); // Dark Cyan
    painter.setPen(QColor("#e5e9f0"));
    painter.drawText(QRect(padX, y1, textW1 - padX * 2, h1), Qt::AlignVCenter | Qt::AlignLeft, date_time_str_);

    // Segment 2 (Current Directory)
    int textW2 = fm.horizontalAdvance(current_dir_) + padX * 2;
    int startX2 = textW1;

    QPainterPath p2;
    p2.moveTo(startX2, y1);
    p2.lineTo(startX2 + arrowW, y1 + h1 / 2.0);
    p2.lineTo(startX2, y1 + h1);
    p2.lineTo(startX2 + textW2, y1 + h1);
    p2.lineTo(startX2 + textW2 + arrowW, y1 + h1 / 2.0);
    p2.lineTo(startX2 + textW2, y1);
    p2.closeSubpath();

    painter.fillPath(p2, QBrush(QColor("#7d8471"))); // Muted Olive
    painter.setPen(QColor("#1e222a"));
    painter.drawText(QRect(startX2 + arrowW + padX / 2, y1, textW2, h1), Qt::AlignVCenter | Qt::AlignLeft, current_dir_);

    // --- ROW 2: Username (Dusty Rose) + ~ (Muted Slate) + PID ---
    int y2 = y1 + h1 + 4;
    int h2 = 24;

    int textW3 = fm.horizontalAdvance(username_) + padX * 2;
    QPainterPath p3;
    p3.moveTo(0, y2);
    p3.lineTo(textW3, y2);
    p3.lineTo(textW3 + arrowW, y2 + h2 / 2.0);
    p3.lineTo(textW3, y2 + h2);
    p3.lineTo(0, y2 + h2);
    p3.closeSubpath();

    painter.fillPath(p3, QBrush(QColor("#6b4452"))); // Dusty Rose / Maroon
    painter.setPen(QColor("#e5e9f0"));
    painter.drawText(QRect(padX, y2, textW3 - padX * 2, h2), Qt::AlignVCenter | Qt::AlignLeft, username_);

    // Sub-badge ~
    int startX4 = textW3;
    int textW4 = fm.horizontalAdvance(" ~ ") + padX;
    QPainterPath p4;
    p4.moveTo(startX4, y2);
    p4.lineTo(startX4 + arrowW, y2 + h2 / 2.0);
    p4.lineTo(startX4, y2 + h2);
    p4.lineTo(startX4 + textW4, y2 + h2);
    p4.lineTo(startX4 + textW4 + arrowW, y2 + h2 / 2.0);
    p4.lineTo(startX4 + textW4, y2);
    p4.closeSubpath();

    painter.fillPath(p4, QBrush(QColor("#454854"))); // Muted Slate
    painter.setPen(QColor("#abb2bf"));
    painter.drawText(QRect(startX4 + arrowW, y2, textW4, h2), Qt::AlignVCenter | Qt::AlignCenter, "~");

    // Trailing PID / counter
    int startX5 = startX4 + textW4 + arrowW + 8;
    QString pidStr = QString::number(command_idx_);
    painter.setPen(QColor("#5c6370"));
    painter.drawText(startX5, y2 + h2 - 6, pidStr);
}

} // namespace meridian::gui

