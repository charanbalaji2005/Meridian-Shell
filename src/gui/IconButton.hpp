#pragma once
// src/gui/IconButton.hpp
//
// Icon-only button used across the chrome: a rounded hover fill that fades
// in/out (or snaps when motion is reduced), a subtle press state, an icon
// recolored from muted -> text on hover -> accent when checked, and a
// tooltip that always names the action and its shortcut.

#include <QAbstractButton>
#include <QVariantAnimation>

class QAction;

namespace zephyr::gui {

class IconButton : public QAbstractButton {
    Q_OBJECT

public:
    explicit IconButton(const QString& icon_name, const QString& tooltip, QWidget* parent = nullptr);

    // Mirrors an action: triggers it on click, tracks enabled/checked
    // state, and builds "Text  (Shortcut)" tooltips that update when
    // keybindings change.
    void setAction(QAction* action);
    void setIconName(const QString& name);
    void setIconSize(int px) { icon_px_ = px; update(); }

    QSize sizeHint() const override { return {32, 32}; }

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void animateHover(qreal to);
    void syncFromAction();

    QString icon_name_;
    QString base_tooltip_;
    QAction* action_ = nullptr;
    QVariantAnimation hover_anim_;
    qreal hover_ = 0.0;
    int icon_px_ = 18;
};

} // namespace zephyr::gui
