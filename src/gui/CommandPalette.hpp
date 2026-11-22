#pragma once
// src/gui/CommandPalette.hpp
//
// Ctrl+Shift+P overlay: fuzzy-search every window action, profile, SSH
// connection and theme, then run the chosen one. Ranking reuses
// dev::CommandPalette::fuzzy_score so the CLI palette and the GUI agree.

#include <QFrame>
#include <QList>
#include <QPointer>

class QAction;
class QLineEdit;
class QListWidget;

namespace zephyr::gui {

class CommandPalette : public QFrame {
    Q_OBJECT
public:
    explicit CommandPalette(QWidget* parent);

    // Actions are not owned. Group is shown as a dim prefix ("Tab: New Tab").
    void setActions(const QList<QPair<QString, QAction*>>& grouped_actions);
    void popup();
    void dismiss();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void refilter(const QString& query);
    void runCurrent();
    void reposition();

    QLineEdit* edit_;
    QListWidget* list_;
    QList<QPair<QString, QPointer<QAction>>> actions_;
};

} // namespace zephyr::gui
