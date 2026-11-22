#pragma once
// src/gui/WindowFrame.hpp
//
// Optional "rice" window style: a rounded, translucent window with a soft
// glowing gradient border (like a Hyprland/Picom border with shadow), drawn
// by Zephyr itself so it looks the same on every desktop. When the window is
// frameless the frame's margin also acts as the move/resize handle, using
// the compositor's own interactive move/resize (works on X11 and Wayland).

#include <QColor>
#include <QWidget>

class QVBoxLayout;

namespace zephyr::gui {

class WindowFrame : public QWidget {
    Q_OBJECT
public:
    WindowFrame(QWidget* content, QWidget* parent = nullptr);

    struct Style {
        bool glow = false;
        QColor color1;
        QColor color2;
        int radius = 14;
        QColor fill;          // window background (theme bg at the configured opacity)
        bool frameless = false;
    };
    void setFrameStyle(const Style& s);
    bool glowEnabled() const { return style_.glow; }

    static constexpr int kGlow = 12; // glow width in px

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    Qt::Edges edgesAt(const QPoint& p) const;
    QRectF borderRect() const;

    QVBoxLayout* layout_;
    Style style_;
};

} // namespace zephyr::gui
