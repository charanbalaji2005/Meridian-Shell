#pragma once
// src/gui/SidebarPanels.hpp
//
// The individual sidebar panels. Each is self-contained and talks to the
// rest of the app only through signals.

#include <QTimer>
#include <QWidget>

class QComboBox;
class QFileSystemModel;
class QLabel;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;
class QTreeView;
class QVBoxLayout;

namespace zephyr::dev { class SystemMonitor; }

namespace zephyr::gui {

class ProfilesPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProfilesPanel(QWidget* parent = nullptr);
    void refresh();
signals:
    void launchProfile(const QString& name);
    void launchSsh(const QString& name);
    void manage(const QString& page);
private:
    QListWidget* list_;
};

class FilesPanel : public QWidget {
    Q_OBJECT
public:
    explicit FilesPanel(QWidget* parent = nullptr);
    void setTerminalDirectory(const QString& dir);
signals:
    void insertText(const QString& text);
private:
    void navigate(const QString& dir);
    QFileSystemModel* model_;
    QTreeView* tree_;
    QLabel* path_label_;
    QString terminal_dir_;
    QString current_dir_;
};

class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);
signals:
    void insertText(const QString& text);
protected:
    void showEvent(QShowEvent* e) override;
private:
    void reload();
    void applyFilter(const QString& text);
    QLineEdit* filter_;
    QListWidget* list_;
    QLabel* source_label_;
    bool loaded_ = false;
};

class SystemPanel : public QWidget {
    Q_OBJECT
public:
    explicit SystemPanel(QWidget* parent = nullptr);
    ~SystemPanel() override;
protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
private:
    void sample();
    QProgressBar* addMeter(QVBoxLayout* lay, const QString& label, QLabel** value_out);
    dev::SystemMonitor* monitor_;
    QTimer timer_;
    QLabel* os_;
    QLabel* kernel_;
    QLabel* host_;
    QLabel* uptime_;
    QLabel* desktop_;
    QProgressBar* cpu_bar_; QLabel* cpu_val_;
    QProgressBar* mem_bar_; QLabel* mem_val_;
    QProgressBar* swap_bar_; QLabel* swap_val_;
    QProgressBar* disk_bar_; QLabel* disk_val_;
    QLabel* net_;
};

class PackagesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PackagesPanel(QWidget* parent = nullptr);
signals:
    void insertText(const QString& text);
private:
    void emitCommand(const QString& which);
    QComboBox* manager_;
    QLineEdit* package_;
    QLabel* status_;
    QLabel* distro_;
};

class AssistantPanel : public QWidget {
    Q_OBJECT
public:
    explicit AssistantPanel(QWidget* parent = nullptr);
    void setWorkingDirectory(const QString& dir) { cwd_ = dir; }
signals:
    void insertText(const QString& text);
private:
    void ask();
    QLineEdit* input_;
    QLabel* command_;
    QLabel* explanation_;
    QLabel* risk_;
    QListWidget* alternatives_;
    QPushButton* insert_;
    QWidget* card_;
    QString cwd_;
};

} // namespace zephyr::gui
