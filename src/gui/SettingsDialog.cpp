#include "SettingsDialog.hpp"

#include "Actions.hpp"
#include "AppContext.hpp"
#include "Icons.hpp"
#include "Motion.hpp"
#include "UiTheme.hpp"

#include "../config/paths.hpp"
#include "../config/theme.hpp"
#include "../dev/package_managers.hpp"
#include "../dev/ssh_manager.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTableWidget>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

#include <memory>

namespace zephyr::gui {

namespace {

constexpr const char* kDesktopId = "org.zephyr_terminal.ZephyrTerminal";
constexpr const char* kRepoUrl = "https://github.com/charanbalaji2005/Meridian-Shell";

// Builds the standard page skeleton: heading, subtitle, then "cards".
struct PageBuilder {
    QScrollArea* scroll;
    QVBoxLayout* lay;

    PageBuilder(const QString& title, const QString& subtitle) {
        scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        auto* body = new QWidget;
        lay = new QVBoxLayout(body);
        lay->setContentsMargins(28, 22, 28, 22);
        lay->setSpacing(14);
        auto* h = new QLabel(title);
        h->setObjectName(QStringLiteral("Heading"));
        lay->addWidget(h);
        if (!subtitle.isEmpty()) {
            auto* sub = new QLabel(subtitle);
            sub->setObjectName(QStringLiteral("Muted"));
            sub->setWordWrap(true);
            lay->addWidget(sub);
        }
        scroll->setWidget(body);
    }
    QFormLayout* card(const QString& title = QString()) {
        auto* frame = new QFrame;
        frame->setObjectName(QStringLiteral("Card"));
        frame->setAttribute(Qt::WA_StyledBackground, true);
        auto* v = new QVBoxLayout(frame);
        v->setContentsMargins(16, 14, 16, 14);
        if (!title.isEmpty()) {
            auto* t = new QLabel(title);
            t->setStyleSheet(QStringLiteral("font-weight:600;"));
            v->addWidget(t);
        }
        auto* form = new QFormLayout;
        form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        form->setHorizontalSpacing(18);
        form->setVerticalSpacing(10);
        v->addLayout(form);
        lay->addWidget(frame);
        return form;
    }
    QVBoxLayout* plainCard() {
        auto* frame = new QFrame;
        frame->setObjectName(QStringLiteral("Card"));
        frame->setAttribute(Qt::WA_StyledBackground, true);
        auto* v = new QVBoxLayout(frame);
        v->setContentsMargins(16, 14, 16, 14);
        lay->addWidget(frame);
        return v;
    }
    QWidget* done() {
        lay->addStretch(1);
        return scroll;
    }
};

QLabel* note(const QString& text) {
    auto* l = new QLabel(text);
    l->setObjectName(QStringLiteral("Muted"));
    l->setWordWrap(true);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    l->setOpenExternalLinks(true);
    return l;
}

QWidget* row(std::initializer_list<QWidget*> widgets, int stretch_index = 0) {
    auto* w = new QWidget;
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    int i = 0;
    for (auto* x : widgets) h->addWidget(x, i++ == stretch_index ? 1 : 0);
    return w;
}

// Small swatch strip used in the theme list.
QIcon theme_swatch(const config::Theme& t) {
    QPixmap pm(64, 16);
    pm.fill(UiTheme::toQColor(t.background));
    QPainter p(&pm);
    // red, green, yellow, blue, magenta, cyan, then the foreground color
    for (std::size_t i = 1; i <= 6; ++i) p.fillRect(4 + static_cast<int>(i - 1) * 8, 4, 7, 8, UiTheme::toQColor(t.ansi[i]));
    p.fillRect(4 + 6 * 8, 4, 7, 8, UiTheme::toQColor(t.foreground));
    return QIcon(pm);
}

class ThemePreview : public QWidget {
public:
    explicit ThemePreview(QWidget* parent = nullptr) : QWidget(parent) { setMinimumHeight(150); }
    void setTheme(const config::Theme* t) { theme_ = t; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        if (!theme_) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(0, 0, -1, -1), 10, 10);
        p.fillPath(path, UiTheme::toQColor(theme_->background));
        QFont f(QString::fromStdString(AppContext::instance().settings().font_family));
        f.setStyleHint(QFont::Monospace);
        f.setPointSize(10);
        p.setFont(f);
        const int lh = QFontMetrics(f).height() + 2;
        int y = 16 + lh;
        auto put = [&](int x, const QString& s, const config::Rgb& c) {
            p.setPen(UiTheme::toQColor(c));
            p.drawText(x, y, s);
            return x + QFontMetrics(f).horizontalAdvance(s);
        };
        int x = put(14, QStringLiteral("me@zephyr"), theme_->ansi[2]);
        x = put(x, QStringLiteral(":"), theme_->foreground);
        x = put(x, QStringLiteral("~/src"), theme_->ansi[4]);
        put(x, QStringLiteral("$ git status"), theme_->foreground);
        y += lh;
        put(14, QStringLiteral("modified:   src/main.cpp"), theme_->ansi[1]);
        y += lh;
        put(14, QStringLiteral("new file:   README.md"), theme_->ansi[2]);
        y += lh;
        x = put(14, QStringLiteral("warning: "), theme_->ansi[3]);
        put(x, QStringLiteral("2 files changed"), theme_->foreground);
        y += lh / 2;
        for (int i = 0; i < 16; ++i) {
            QRectF r(14 + (i % 8) * 26, y + (i / 8) * 18, 22, 14);
            QPainterPath sw;
            sw.addRoundedRect(r, 3, 3);
            p.fillPath(sw, UiTheme::toQColor(theme_->ansi[static_cast<std::size_t>(i)]));
        }
    }
private:
    const config::Theme* theme_ = nullptr;
};

QString user_desktop_file() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
           QStringLiteral("/applications/%1.desktop").arg(QLatin1String(kDesktopId));
}

QString system_desktop_file() {
    for (const auto& dir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString f = dir + QStringLiteral("/applications/%1.desktop").arg(QLatin1String(kDesktopId));
        if (QFileInfo::exists(f) && f != user_desktop_file()) return f;
    }
    return {};
}

} // namespace

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Zephyr Settings"));
    setMinimumSize(860, 600);
    resize(960, 680);
    s_ = AppContext::instance().settings();
    const auto& store = AppContext::instance().profiles();
    profiles_ = store.profiles();
    ssh_ = store.ssh_profiles();

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    nav_ = new QListWidget(this);
    nav_->setObjectName(QStringLiteral("SettingsNav"));
    nav_->setFixedWidth(210);
    nav_->setIconSize(QSize(18, 18));
    stack_ = new QStackedWidget(this);
    lay->addWidget(nav_);
    lay->addWidget(stack_, 1);

    addPage(QStringLiteral("appearance"), QStringLiteral("monitor"), tr("Appearance"), buildAppearance());
    addPage(QStringLiteral("colors"), QStringLiteral("palette"), tr("Colors & Themes"), buildColors());
    addPage(QStringLiteral("terminal"), QStringLiteral("terminal"), tr("Terminal"), buildTerminal());
    addPage(QStringLiteral("fonts"), QStringLiteral("type"), tr("Fonts"), buildFonts());
    addPage(QStringLiteral("animations"), QStringLiteral("activity"), tr("Animations"), buildAnimations());
    addPage(QStringLiteral("keybindings"), QStringLiteral("keyboard"), tr("Keybindings"), buildKeybindings());
    addPage(QStringLiteral("profiles"), QStringLiteral("user"), tr("Shell Profiles"), buildProfiles());
    addPage(QStringLiteral("ssh"), QStringLiteral("server"), tr("SSH Connections"), buildSsh());
    addPage(QStringLiteral("startup"), QStringLiteral("power"), tr("Startup"), buildStartup());
    addPage(QStringLiteral("linux"), QStringLiteral("app-window"), tr("Linux Integration"), buildLinux());
    addPage(QStringLiteral("nix"), QStringLiteral("snowflake"), tr("NixOS"), buildNix());
    addPage(QStringLiteral("performance"), QStringLiteral("zap"), tr("Performance"), buildPerformance());

    connect(nav_, &QListWidget::currentRowChanged, this, [this](int row) {
        stack_->setCurrentIndex(row);
        if (auto* w = stack_->currentWidget()) motion::fadeIn(w, motion::kFast);
    });
    nav_->setCurrentRow(0);
}

void SettingsDialog::addPage(const QString& id, const QString& icon, const QString& title, QWidget* page) {
    page_ids_ << id;
    new QListWidgetItem(Icons::icon(icon), title, nav_);
    stack_->addWidget(page);
}

void SettingsDialog::showPage(const QString& id) {
    const int idx = page_ids_.indexOf(id);
    if (idx >= 0) nav_->setCurrentRow(idx);
}

void SettingsDialog::commit() { AppContext::instance().setSettings(s_); }

// ---- Appearance ------------------------------------------------------------------

QWidget* SettingsDialog::buildAppearance() {
    PageBuilder pb(tr("Appearance"), tr("Window chrome, accent color, transparency and background."));
    auto* form = pb.card(tr("Interface"));

    auto* mode = new QComboBox;
    mode->addItem(Icons::icon(QStringLiteral("moon")), tr("Dark"), QStringLiteral("dark"));
    mode->addItem(Icons::icon(QStringLiteral("sun")), tr("Light"), QStringLiteral("light"));
    mode->addItem(Icons::icon(QStringLiteral("monitor")), tr("Follow system"), QStringLiteral("system"));
    mode->setCurrentIndex(std::max(0, mode->findData(QString::fromStdString(s_.ui_mode))));
    connect(mode, &QComboBox::currentIndexChanged, this, [this, mode](int) {
        s_.ui_mode = mode->currentData().toString().toStdString();
        commit();
    });
    form->addRow(tr("Mode"), mode);

    // Accent: presets + custom picker.
    auto* accents = new QWidget;
    auto* ah = new QHBoxLayout(accents);
    ah->setContentsMargins(0, 0, 0, 0);
    ah->setSpacing(6);
    const QStringList presets = {"#8b7cf6", "#3b82f6", "#06b6d4", "#10b981", "#f59e0b", "#f43f5e", "#ec4899"};
    auto* custom = new QPushButton(tr("Custom…"));
    for (const auto& hex : presets) {
        auto* b = new QPushButton;
        b->setFixedSize(26, 26);
        b->setCursor(Qt::PointingHandCursor);
        b->setToolTip(hex);
        b->setStyleSheet(QStringLiteral("QPushButton{background:%1;border-radius:13px;border:2px solid transparent;}"
                                        "QPushButton:hover{border:2px solid palette(text);}").arg(hex));
        connect(b, &QPushButton::clicked, this, [this, hex]() {
            s_.accent_color = hex.toStdString();
            commit();
        });
        ah->addWidget(b);
    }
    connect(custom, &QPushButton::clicked, this, [this]() {
        const QColor c = QColorDialog::getColor(QColor(QString::fromStdString(s_.accent_color)), this, tr("Accent color"));
        if (c.isValid()) {
            s_.accent_color = c.name().toStdString();
            commit();
        }
    });
    ah->addWidget(custom);
    ah->addStretch(1);
    form->addRow(tr("Accent color"), accents);

    auto* tabs = new QCheckBox(tr("Show the tab bar"));
    tabs->setChecked(s_.tab_bar_visible);
    connect(tabs, &QCheckBox::toggled, this, [this](bool on) { s_.tab_bar_visible = on; commit(); });
    form->addRow(QString(), tabs);
    auto* side = new QCheckBox(tr("Open the sidebar panel when a window starts"));
    side->setChecked(s_.sidebar_visible);
    connect(side, &QCheckBox::toggled, this, [this](bool on) { s_.sidebar_visible = on; commit(); });
    form->addRow(QString(), side);

    auto* bg = pb.card(tr("Transparency & background"));
    auto* opacity = new QSlider(Qt::Horizontal);
    opacity->setRange(30, 100);
    opacity->setValue(s_.window_opacity);
    auto* opacity_val = new QLabel(QStringLiteral("%1%").arg(s_.window_opacity));
    connect(opacity, &QSlider::valueChanged, this, [this, opacity_val](int v) {
        s_.window_opacity = v;
        opacity_val->setText(QStringLiteral("%1%").arg(v));
        commit();
    });
    bg->addRow(tr("Background opacity"), row({opacity, opacity_val}));
    bg->addRow(QString(), note(tr("Translucency needs a compositing desktop (GNOME, KDE Plasma, Hyprland, Sway, …). "
                                  "Text always stays fully opaque.")));

    auto* image = new QLineEdit(QString::fromStdString(s_.background_image));
    image->setPlaceholderText(tr("No image"));
    image->setReadOnly(true);
    auto* browse = new QPushButton(tr("Choose…"));
    auto* clear = new QPushButton(tr("Clear"));
    connect(browse, &QPushButton::clicked, this, [this, image]() {
        const QString f = QFileDialog::getOpenFileName(this, tr("Background image"), QDir::homePath(),
                                                       tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)"));
        if (f.isEmpty()) return;
        image->setText(f);
        s_.background_image = f.toStdString();
        commit();
    });
    connect(clear, &QPushButton::clicked, this, [this, image]() {
        image->clear();
        s_.background_image.clear();
        commit();
    });
    bg->addRow(tr("Background image"), row({image, browse, clear}));
    auto* img_op = new QSlider(Qt::Horizontal);
    img_op->setRange(0, 100);
    img_op->setValue(s_.background_image_opacity);
    connect(img_op, &QSlider::valueChanged, this, [this](int v) { s_.background_image_opacity = v; commit(); });
    bg->addRow(tr("Image strength"), img_op);
    auto* pad = new QSpinBox;
    pad->setRange(0, 64);
    pad->setSuffix(tr(" px"));
    pad->setValue(s_.padding);
    connect(pad, &QSpinBox::valueChanged, this, [this](int v) { s_.padding = v; commit(); });
    bg->addRow(tr("Inner padding"), pad);

    // ---- Window style ("rice" look) ----
    auto* ws = pb.card(tr("Window style"));
    auto* preset = new QPushButton(Icons::icon(QStringLiteral("palette")), tr("Apply Neon Night look"));
    preset->setObjectName(QStringLiteral("Primary"));
    connect(preset, &QPushButton::clicked, this, [this]() {
        s_ = config::neon_night_preset(s_);
        commit();
        emit reopenRequested(QStringLiteral("appearance")); // refresh every control
    });
    ws->addRow(QString(), preset);
    ws->addRow(QString(), note(tr("Translucent purple-navy terminal, glowing pink→violet rounded border, "
                                  "no header or sidebar. Ctrl+Shift+P or right-click brings the chrome back.")));

    auto* glow = new QCheckBox(tr("Rounded window with a glowing border"));
    glow->setChecked(s_.glow_border);
    connect(glow, &QCheckBox::toggled, this, [this](bool on) { s_.glow_border = on; commit(); });
    ws->addRow(QString(), glow);

    auto color_button = [this](std::string* target) {
        auto* b = new QPushButton(QString::fromStdString(*target));
        auto paint = [b, target]() {
            b->setText(QString::fromStdString(*target));
            b->setStyleSheet(QStringLiteral("QPushButton{border-left:18px solid %1;}").arg(QString::fromStdString(*target)));
        };
        paint();
        connect(b, &QPushButton::clicked, this, [this, target, paint]() {
            const QColor c = QColorDialog::getColor(QColor(QString::fromStdString(*target)), this, tr("Glow color"));
            if (!c.isValid()) return;
            *target = c.name().toStdString();
            paint();
            commit();
        });
        return b;
    };
    ws->addRow(tr("Glow colors"), row({color_button(&s_.glow_color), color_button(&s_.glow_color2)}, -1));

    auto* radius = new QSpinBox;
    radius->setRange(0, 32);
    radius->setSuffix(tr(" px"));
    radius->setValue(s_.corner_radius);
    connect(radius, &QSpinBox::valueChanged, this, [this](int v) { s_.corner_radius = v; commit(); });
    ws->addRow(tr("Corner radius"), radius);

    auto* minimal = new QCheckBox(tr("Minimal window: hide the header and sidebar"));
    minimal->setChecked(s_.minimal_chrome);
    connect(minimal, &QCheckBox::toggled, this, [this](bool on) { s_.minimal_chrome = on; commit(); });
    ws->addRow(QString(), minimal);

    auto* frameless = new QCheckBox(tr("Frameless window (no title bar; drag the header or border to move)"));
    frameless->setChecked(s_.frameless_window);
    connect(frameless, &QCheckBox::toggled, this, [this](bool on) { s_.frameless_window = on; commit(); });
    ws->addRow(QString(), frameless);
    ws->addRow(QString(), note(tr("On Hyprland, Sway and other tiling compositors you can instead let the "
                                  "compositor draw the border and blur — see docs/ricing.md.")));
    return pb.done();
}

// ---- Colors -------------------------------------------------------------------------

QWidget* SettingsDialog::buildColors() {
    PageBuilder pb(tr("Colors & Themes"), tr("The color scheme used inside terminals."));
    auto* card = pb.plainCard();
    auto* list = new QListWidget;
    list->setIconSize(QSize(64, 16));
    list->setMinimumHeight(220);
    auto* preview = new ThemePreview;
    auto* auto_item = new QListWidgetItem(theme_swatch(UiTheme::terminalTheme()),
                                          tr("Zephyr (follows light/dark mode)"), list);
    auto_item->setData(Qt::UserRole, QStringLiteral("zephyr-auto"));
    for (const auto& t : config::builtin_themes()) {
        auto* item = new QListWidgetItem(theme_swatch(t),
                                         QStringLiteral("%1   %2").arg(QString::fromStdString(t.display_name),
                                                                       t.dark ? tr("dark") : tr("light")), list);
        item->setData(Qt::UserRole, QString::fromStdString(t.id));
    }
    for (int i = 0; i < list->count(); ++i) {
        if (list->item(i)->data(Qt::UserRole).toString().toStdString() == s_.theme_name) list->setCurrentRow(i);
    }
    preview->setTheme(&UiTheme::terminalTheme());
    connect(list, &QListWidget::currentItemChanged, this, [this, preview](QListWidgetItem* item) {
        if (!item) return;
        s_.theme_name = item->data(Qt::UserRole).toString().toStdString();
        commit();
        preview->setTheme(&UiTheme::terminalTheme());
    });
    card->addWidget(list);
    card->addWidget(preview);
    return pb.done();
}

// ---- Terminal ------------------------------------------------------------------------

QWidget* SettingsDialog::buildTerminal() {
    PageBuilder pb(tr("Terminal"), tr("Cursor, scrollback, clipboard and safety behaviour."));
    auto* form = pb.card(tr("Cursor"));
    auto* shape = new QComboBox;
    shape->addItem(tr("Block  █"), QStringLiteral("block"));
    shape->addItem(tr("Underline  _"), QStringLiteral("underline"));
    shape->addItem(tr("Bar  ▏"), QStringLiteral("bar"));
    shape->setCurrentIndex(std::max(0, shape->findData(QString::fromStdString(s_.cursor_style))));
    connect(shape, &QComboBox::currentIndexChanged, this, [this, shape](int) {
        s_.cursor_style = shape->currentData().toString().toStdString();
        commit();
    });
    form->addRow(tr("Style"), shape);
    auto* blink = new QCheckBox(tr("Blinking cursor"));
    blink->setChecked(s_.cursor_blink);
    connect(blink, &QCheckBox::toggled, this, [this](bool on) { s_.cursor_blink = on; commit(); });
    form->addRow(QString(), blink);
    form->addRow(QString(), note(tr("Programs such as vim may request their own cursor shape; that request wins while they run.")));

    auto* sb = pb.card(tr("Scrollback"));
    auto* lines = new QSpinBox;
    lines->setRange(0, 1000000);
    lines->setSingleStep(1000);
    lines->setValue(s_.scrollback_lines);
    lines->setSuffix(tr(" lines"));
    connect(lines, &QSpinBox::valueChanged, this, [this](int v) { s_.scrollback_lines = v; commit(); });
    sb->addRow(tr("History size"), lines);
    auto* wheel = new QSpinBox;
    wheel->setRange(1, 20);
    wheel->setValue(s_.scroll_multiplier);
    wheel->setSuffix(tr(" lines per notch"));
    connect(wheel, &QSpinBox::valueChanged, this, [this](int v) { s_.scroll_multiplier = v; commit(); });
    sb->addRow(tr("Mouse wheel"), wheel);

    auto* clip = pb.card(tr("Clipboard & safety"));
    auto* cos = new QCheckBox(tr("Copy selected text to the clipboard automatically"));
    cos->setChecked(s_.copy_on_select);
    connect(cos, &QCheckBox::toggled, this, [this](bool on) { s_.copy_on_select = on; commit(); });
    clip->addRow(QString(), cos);
    auto* warn = new QCheckBox(tr("Confirm before pasting multiple lines into a shell without bracketed paste"));
    warn->setChecked(s_.warn_multiline_paste);
    connect(warn, &QCheckBox::toggled, this, [this](bool on) { s_.warn_multiline_paste = on; commit(); });
    clip->addRow(QString(), warn);
    auto* confirm = new QCheckBox(tr("Confirm before closing a tab or window with a running program"));
    confirm->setChecked(s_.confirm_close_running);
    connect(confirm, &QCheckBox::toggled, this, [this](bool on) { s_.confirm_close_running = on; commit(); });
    clip->addRow(QString(), confirm);
    clip->addRow(QString(), note(tr("Pasted text always has control characters removed, so a clipboard can't "
                                    "inject escape sequences.")));
    return pb.done();
}

// ---- Fonts ----------------------------------------------------------------------------

QWidget* SettingsDialog::buildFonts() {
    PageBuilder pb(tr("Fonts"), tr("Monospaced font used in terminals."));
    auto* form = pb.card();
    auto* family = new QFontComboBox;
    family->setFontFilters(QFontComboBox::MonospacedFonts);
    family->setCurrentFont(QFont(QString::fromStdString(s_.font_family)));
    auto* size = new QSpinBox;
    size->setRange(6, 72);
    size->setValue(s_.font_size);
    size->setSuffix(tr(" pt"));
    auto* lig = new QCheckBox(tr("Enable font ligatures (e.g. -> != === in fonts that provide them)"));
    lig->setChecked(s_.font_ligatures);
    auto* lh = new QSpinBox;
    lh->setRange(80, 200);
    lh->setSingleStep(5);
    lh->setSuffix(QStringLiteral(" %"));
    lh->setValue(s_.line_height_percent);
    auto* preview = new QLabel(QStringLiteral("fn main() -> Result<(), Error> { x != y && a >= b }  0O 1lI"));
    preview->setWordWrap(true);
    preview->setMinimumHeight(48);
    auto refresh_preview = [this, preview]() {
        QFont f(QString::fromStdString(s_.font_family));
        f.setStyleHint(QFont::Monospace);
        f.setPointSize(s_.font_size);
        preview->setFont(f);
    };
    refresh_preview();
    connect(family, &QFontComboBox::currentFontChanged, this, [this, refresh_preview](const QFont& f) {
        s_.font_family = f.family().toStdString();
        commit();
        refresh_preview();
    });
    connect(size, &QSpinBox::valueChanged, this, [this, refresh_preview](int v) {
        s_.font_size = v;
        commit();
        refresh_preview();
    });
    connect(lig, &QCheckBox::toggled, this, [this](bool on) { s_.font_ligatures = on; commit(); });
    connect(lh, &QSpinBox::valueChanged, this, [this](int v) { s_.line_height_percent = v; commit(); });
    form->addRow(tr("Family"), family);
    form->addRow(tr("Size"), size);
    form->addRow(tr("Line height"), lh);
    form->addRow(QString(), lig);
    form->addRow(tr("Preview"), preview);
    form->addRow(QString(), note(tr("Ctrl+= / Ctrl+- zoom the focused terminal without changing this setting.")));
    return pb.done();
}

// ---- Animations ---------------------------------------------------------------------------

QWidget* SettingsDialog::buildAnimations() {
    PageBuilder pb(tr("Animations"), tr("Motion in the interface. Terminal output itself is never animated."));
    auto* form = pb.card();
    auto* on = new QCheckBox(tr("Enable interface animations"));
    on->setChecked(s_.animations_enabled);
    auto* reduced = new QCheckBox(tr("Reduce motion"));
    reduced->setChecked(s_.reduced_motion);
    connect(on, &QCheckBox::toggled, this, [this](bool v) { s_.animations_enabled = v; commit(); });
    connect(reduced, &QCheckBox::toggled, this, [this](bool v) { s_.reduced_motion = v; commit(); });
    form->addRow(QString(), on);
    form->addRow(QString(), reduced);
    form->addRow(QString(), note(tr("Setting the environment variable ZEPHYR_REDUCED_MOTION=1 also disables all "
                                    "animations, regardless of these options. Animations are short (120–240 ms) "
                                    "and only run while something changes.")));
    return pb.done();
}

// ---- Keybindings ---------------------------------------------------------------------------

QWidget* SettingsDialog::buildKeybindings() {
    PageBuilder pb(tr("Keybindings"), tr("Click a shortcut and press a new key combination. "
                                         "Plain Ctrl+letter combos are sent to the shell and can't be used."));
    auto* card = pb.plainCard();
    auto* table = new QTableWidget(static_cast<int>(actionCatalog().size()), 3);
    table->setHorizontalHeaderLabels({tr("Action"), tr("Shortcut"), QString()});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->setColumnWidth(1, 220);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMinimumHeight(420);
    auto* status = note(QString());
    auto& kb = AppContext::instance().keybindings();

    // Shared by every row's handler so conflict checks always see all rows.
    auto editors = std::make_shared<QList<QKeySequenceEdit*>>();
    for (int r = 0; r < actionCatalog().size(); ++r) {
        const auto& info = actionCatalog()[r];
        auto* label = new QTableWidgetItem(Icons::icon(QString::fromLatin1(info.icon)),
                                           QCoreApplication::translate("Actions", info.label));
        label->setFlags(Qt::ItemIsEnabled);
        table->setItem(r, 0, label);
        auto* edit = new QKeySequenceEdit(QKeySequence::fromString(QString::fromStdString(kb.get(info.id)),
                                                                   QKeySequence::PortableText));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        edit->setMaximumSequenceLength(1);
#endif
        editors->append(edit);
        table->setCellWidget(r, 1, edit);
        auto* reset = new QPushButton(tr("Reset"));
        table->setCellWidget(r, 2, reset);

        const std::string id = info.id;
        connect(edit, &QKeySequenceEdit::editingFinished, this, [this, edit, id, status, editors, r]() {
            auto& kb = AppContext::instance().keybindings();
            const QString combo = edit->keySequence().toString(QKeySequence::PortableText);
            // Reject plain Ctrl+<letter>: those are control characters the shell needs.
            const QKeyCombination kc = edit->keySequence().isEmpty() ? QKeyCombination() : edit->keySequence()[0];
            if (kc.keyboardModifiers() == Qt::ControlModifier && kc.key() >= Qt::Key_A && kc.key() <= Qt::Key_Z) {
                status->setText(tr("%1 is a shell control key (e.g. Ctrl+C interrupts). Add Shift or Alt.").arg(combo));
                edit->setKeySequence(QKeySequence::fromString(QString::fromStdString(kb.get(id)), QKeySequence::PortableText));
                return;
            }
            for (int other = 0; other < editors->size(); ++other) {
                if (other == r || combo.isEmpty()) continue;
                if ((*editors)[other]->keySequence() == edit->keySequence()) {
                    status->setText(tr("%1 is already used by \"%2\".")
                                        .arg(combo, QCoreApplication::translate("Actions", actionCatalog()[other].label)));
                    edit->setKeySequence(QKeySequence::fromString(QString::fromStdString(kb.get(id)), QKeySequence::PortableText));
                    return;
                }
            }
            kb.set(id, combo.toStdString());
            AppContext::instance().notifyKeybindingsChanged();
            status->setText(combo.isEmpty() ? tr("Shortcut removed.") : tr("Saved."));
        });
        connect(reset, &QPushButton::clicked, this, [edit, id, status]() {
            auto& kb = AppContext::instance().keybindings();
            kb.reset(id);
            edit->setKeySequence(QKeySequence::fromString(QString::fromStdString(kb.get(id)), QKeySequence::PortableText));
            AppContext::instance().notifyKeybindingsChanged();
            status->setText(tr("Restored default."));
        });
    }
    card->addWidget(table);
    card->addWidget(status);
    return pb.done();
}

// ---- Profiles ---------------------------------------------------------------------------------

void SettingsDialog::saveProfiles() {
    AppContext::instance().setProfiles(profiles_, ssh_);
}

QWidget* SettingsDialog::buildProfiles() {
    PageBuilder pb(tr("Shell Profiles"), tr("What a new tab runs: program, arguments, starting folder and extra "
                                            "environment variables. Programs are started directly, never through a shell string."));
    auto* card = pb.plainCard();
    auto* split = new QHBoxLayout;
    card->addLayout(split);

    auto* left = new QVBoxLayout;
    auto* list = new QListWidget;
    list->setFixedWidth(200);
    auto* add = new QPushButton(Icons::icon(QStringLiteral("plus")), tr("Add"));
    auto* del = new QPushButton(Icons::icon(QStringLiteral("trash")), tr("Delete"));
    auto* make_default = new QPushButton(tr("Make default"));
    left->addWidget(list, 1);
    left->addWidget(row({add, del}, -1));
    left->addWidget(make_default);
    split->addLayout(left);

    auto* form_w = new QWidget;
    auto* form = new QFormLayout(form_w);
    auto* name = new QLineEdit;
    auto* program = new QLineEdit;
    program->setPlaceholderText(QStringLiteral("/bin/zsh"));
    auto* browse = new QPushButton(tr("Browse…"));
    auto* args = new QLineEdit;
    args->setPlaceholderText(tr("e.g. -l   (quote arguments containing spaces)"));
    auto* cwd = new QLineEdit;
    cwd->setPlaceholderText(tr("Home folder"));
    auto* cwd_browse = new QPushButton(tr("Browse…"));
    auto* env = new QTableWidget(0, 2);
    env->setHorizontalHeaderLabels({tr("Variable"), tr("Value")});
    env->horizontalHeader()->setStretchLastSection(true);
    env->verticalHeader()->setVisible(false);
    env->setMinimumHeight(140);
    auto* env_add = new QPushButton(tr("Add variable"));
    auto* env_del = new QPushButton(tr("Remove variable"));
    auto* save = new QPushButton(tr("Save profile"));
    save->setObjectName(QStringLiteral("Primary"));
    auto* status = note(QString());
    form->addRow(tr("Name"), name);
    form->addRow(tr("Program"), row({program, browse}));
    form->addRow(tr("Arguments"), args);
    form->addRow(tr("Start in"), row({cwd, cwd_browse}));
    form->addRow(tr("Environment"), env);
    form->addRow(QString(), row({env_add, env_del}, -1));
    form->addRow(QString(), row({status, save}));
    split->addWidget(form_w, 1);

    auto refresh_list = [this, list]() {
        const int keep = std::max(0, list->currentRow());
        list->clear();
        for (const auto& p : profiles_) {
            QString label = QString::fromStdString(p.name);
            if (p.name == s_.default_profile) label += tr("  (default)");
            new QListWidgetItem(Icons::icon(QStringLiteral("terminal")), label, list);
        }
        list->setCurrentRow(std::min(keep, list->count() - 1));
    };
    auto load = [this, list, name, program, args, cwd, env, status]() {
        const int i = list->currentRow();
        status->clear();
        if (i < 0 || i >= static_cast<int>(profiles_.size())) return;
        const auto& p = profiles_[static_cast<std::size_t>(i)];
        name->setText(QString::fromStdString(p.name));
        program->setText(QString::fromStdString(p.program));
        args->setText(QString::fromStdString(config::join_args(p.args)));
        cwd->setText(QString::fromStdString(p.cwd));
        env->setRowCount(0);
        for (const auto& e : p.env) {
            const int r = env->rowCount();
            env->insertRow(r);
            env->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(e.name)));
            env->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(e.value)));
        }
    };
    connect(list, &QListWidget::currentRowChanged, this, load);
    connect(browse, &QPushButton::clicked, this, [this, program]() {
        const QString f = QFileDialog::getOpenFileName(this, tr("Program"), QStringLiteral("/usr/bin"));
        if (!f.isEmpty()) program->setText(f);
    });
    connect(cwd_browse, &QPushButton::clicked, this, [this, cwd]() {
        const QString d = QFileDialog::getExistingDirectory(this, tr("Start in"), QDir::homePath());
        if (!d.isEmpty()) cwd->setText(d);
    });
    connect(env_add, &QPushButton::clicked, this, [env]() {
        env->insertRow(env->rowCount());
        env->setItem(env->rowCount() - 1, 0, new QTableWidgetItem(QStringLiteral("NAME")));
        env->setItem(env->rowCount() - 1, 1, new QTableWidgetItem(QString()));
        env->editItem(env->item(env->rowCount() - 1, 0));
    });
    connect(env_del, &QPushButton::clicked, this, [env]() {
        if (env->currentRow() >= 0) env->removeRow(env->currentRow());
    });
    connect(save, &QPushButton::clicked, this, [=, this]() {
        const int i = list->currentRow();
        if (i < 0) return;
        config::Profile p;
        p.name = name->text().trimmed().toStdString();
        p.program = program->text().trimmed().toStdString();
        p.args = config::split_args(args->text().toStdString());
        p.cwd = cwd->text().trimmed().toStdString();
        if (p.name.empty() || p.program.empty()) {
            status->setText(tr("Name and program are required."));
            return;
        }
        for (std::size_t k = 0; k < profiles_.size(); ++k) {
            if (static_cast<int>(k) != i && profiles_[k].name == p.name) {
                status->setText(tr("Another profile already uses that name."));
                return;
            }
        }
        for (int r = 0; r < env->rowCount(); ++r) {
            const std::string var = env->item(r, 0) ? env->item(r, 0)->text().trimmed().toStdString() : "";
            const std::string val = env->item(r, 1) ? env->item(r, 1)->text().toStdString() : "";
            if (var.empty()) continue;
            if (!config::valid_env_name(var)) {
                status->setText(tr("\"%1\" is not a valid variable name.").arg(QString::fromStdString(var)));
                return;
            }
            p.env.push_back({var, val});
        }
        const std::string old_name = profiles_[static_cast<std::size_t>(i)].name;
        if (old_name == s_.default_profile && old_name != p.name) {
            s_.default_profile = p.name;
            commit();
        }
        profiles_[static_cast<std::size_t>(i)] = p;
        saveProfiles();
        refresh_list();
        status->setText(tr("Saved."));
    });
    connect(add, &QPushButton::clicked, this, [this, list, refresh_list]() {
        config::Profile p = config::ProfileStore::default_profile();
        p.name = tr("New profile %1").arg(profiles_.size() + 1).toStdString();
        profiles_.push_back(p);
        saveProfiles();
        refresh_list();
        list->setCurrentRow(list->count() - 1);
    });
    connect(del, &QPushButton::clicked, this, [this, list, refresh_list, status]() {
        const int i = list->currentRow();
        if (i < 0) return;
        if (profiles_.size() == 1) {
            status->setText(tr("At least one profile is required."));
            return;
        }
        profiles_.erase(profiles_.begin() + i);
        saveProfiles();
        refresh_list();
    });
    connect(make_default, &QPushButton::clicked, this, [this, list, refresh_list]() {
        const int i = list->currentRow();
        if (i < 0) return;
        s_.default_profile = profiles_[static_cast<std::size_t>(i)].name;
        commit();
        refresh_list();
    });
    refresh_list();
    list->setCurrentRow(0);
    return pb.done();
}

// ---- SSH ------------------------------------------------------------------------------------------

QWidget* SettingsDialog::buildSsh() {
    PageBuilder pb(tr("SSH Connections"), tr("Saved hosts open in a new tab running OpenSSH. Your keys and "
                                             "~/.ssh/config keep working; Zephyr never stores passwords."));
    auto* card = pb.plainCard();
    auto* split = new QHBoxLayout;
    card->addLayout(split);

    auto* left = new QVBoxLayout;
    auto* list = new QListWidget;
    list->setFixedWidth(200);
    auto* add = new QPushButton(Icons::icon(QStringLiteral("plus")), tr("Add"));
    auto* del = new QPushButton(Icons::icon(QStringLiteral("trash")), tr("Delete"));
    auto* import = new QPushButton(Icons::icon(QStringLiteral("download")), tr("Import ~/.ssh/config"));
    left->addWidget(list, 1);
    left->addWidget(row({add, del}, -1));
    left->addWidget(import);
    split->addLayout(left);

    auto* form_w = new QWidget;
    auto* form = new QFormLayout(form_w);
    auto* name = new QLineEdit;
    auto* host = new QLineEdit;
    host->setPlaceholderText(QStringLiteral("server.example.com"));
    auto* user = new QLineEdit;
    user->setPlaceholderText(tr("(optional)"));
    auto* port = new QSpinBox;
    port->setRange(1, 65535);
    port->setValue(22);
    auto* identity = new QLineEdit;
    identity->setPlaceholderText(tr("(optional) ~/.ssh/id_ed25519"));
    auto* id_browse = new QPushButton(tr("Browse…"));
    auto* extra = new QLineEdit;
    extra->setPlaceholderText(tr("(advanced) e.g. -A -o ServerAliveInterval=30"));
    auto* status = note(QString());
    auto* save = new QPushButton(tr("Save"));
    save->setObjectName(QStringLiteral("Primary"));
    auto* connect_btn = new QPushButton(Icons::icon(QStringLiteral("play")), tr("Connect"));
    form->addRow(tr("Name"), name);
    form->addRow(tr("Host"), host);
    form->addRow(tr("User"), user);
    form->addRow(tr("Port"), port);
    form->addRow(tr("Identity file"), row({identity, id_browse}));
    form->addRow(tr("Extra arguments"), extra);
    form->addRow(QString(), row({status, connect_btn, save}));
    split->addWidget(form_w, 1);

    auto refresh_list = [this, list]() {
        const int keep = std::max(0, list->currentRow());
        list->clear();
        for (const auto& s : ssh_) new QListWidgetItem(Icons::icon(QStringLiteral("server")), QString::fromStdString(s.name), list);
        list->setCurrentRow(std::min(keep, list->count() - 1));
    };
    auto load = [=, this]() {
        const int i = list->currentRow();
        status->clear();
        const bool valid = i >= 0 && i < static_cast<int>(ssh_.size());
        form_w->setEnabled(valid);
        if (!valid) return;
        const auto& s = ssh_[static_cast<std::size_t>(i)];
        name->setText(QString::fromStdString(s.name));
        host->setText(QString::fromStdString(s.host));
        user->setText(QString::fromStdString(s.user));
        port->setValue(s.port);
        identity->setText(QString::fromStdString(s.identity_file));
        extra->setText(QString::fromStdString(config::join_args(s.extra_args)));
    };
    connect(list, &QListWidget::currentRowChanged, this, load);
    connect(id_browse, &QPushButton::clicked, this, [this, identity]() {
        const QString f = QFileDialog::getOpenFileName(this, tr("Identity file"), QDir::homePath() + "/.ssh");
        if (!f.isEmpty()) identity->setText(f);
    });
    connect(save, &QPushButton::clicked, this, [=, this]() {
        const int i = list->currentRow();
        if (i < 0) return;
        config::SshProfile s;
        s.name = name->text().trimmed().toStdString();
        s.host = host->text().trimmed().toStdString();
        s.user = user->text().trimmed().toStdString();
        s.port = port->value();
        s.identity_file = identity->text().trimmed().toStdString();
        s.extra_args = config::split_args(extra->text().toStdString());
        std::string err;
        if (!s.validate(&err)) {
            status->setText(QString::fromStdString(err));
            return;
        }
        ssh_[static_cast<std::size_t>(i)] = s;
        saveProfiles();
        refresh_list();
        status->setText(tr("Saved."));
    });
    connect(connect_btn, &QPushButton::clicked, this, [=, this]() {
        const int i = list->currentRow();
        if (i >= 0 && i < static_cast<int>(ssh_.size())) emit connectSsh(QString::fromStdString(ssh_[static_cast<std::size_t>(i)].name));
    });
    connect(add, &QPushButton::clicked, this, [=, this]() {
        config::SshProfile s;
        s.name = tr("New host %1").arg(ssh_.size() + 1).toStdString();
        s.host = "example.com";
        ssh_.push_back(s);
        saveProfiles();
        refresh_list();
        list->setCurrentRow(list->count() - 1);
        host->setFocus();
        host->selectAll();
    });
    connect(del, &QPushButton::clicked, this, [=, this]() {
        const int i = list->currentRow();
        if (i < 0) return;
        ssh_.erase(ssh_.begin() + i);
        saveProfiles();
        refresh_list();
        load();
    });
    connect(import, &QPushButton::clicked, this, [=, this]() {
        dev::SSHManager mgr; // reads ~/.ssh/config
        int added = 0, skipped = 0;
        for (const auto& h : mgr.list_hosts()) {
            if (h.alias.find_first_of("*?! ") != std::string::npos) continue; // patterns, not hosts
            config::SshProfile s;
            s.name = h.alias;
            // Connect by alias so ssh applies every option from ~/.ssh/config.
            s.host = h.alias;
            if (!s.validate()) { ++skipped; continue; }
            bool exists = false;
            for (const auto& e : ssh_) exists = exists || e.name == s.name;
            if (exists) { ++skipped; continue; }
            ssh_.push_back(s);
            ++added;
        }
        saveProfiles();
        refresh_list();
        status->setText(tr("Imported %1 host(s), skipped %2.").arg(added).arg(skipped));
    });
    refresh_list();
    load();
    return pb.done();
}

// ---- Startup -------------------------------------------------------------------------------------

QWidget* SettingsDialog::buildStartup() {
    PageBuilder pb(tr("Startup"), tr("What happens when Zephyr opens."));
    auto* form = pb.card();
    auto* restore = new QCheckBox(tr("Restore tabs, split layout and folders from the last session"));
    restore->setChecked(s_.restore_session);
    connect(restore, &QCheckBox::toggled, this, [this](bool on) { s_.restore_session = on; commit(); });
    form->addRow(QString(), restore);
    form->addRow(QString(), note(tr("Only the layout is restored — commands are never re-run.")));

    auto* profile = new QComboBox;
    for (const auto& p : profiles_) profile->addItem(QString::fromStdString(p.name));
    profile->setCurrentText(QString::fromStdString(s_.default_profile));
    connect(profile, &QComboBox::currentTextChanged, this, [this](const QString& t) {
        s_.default_profile = t.toStdString();
        commit();
    });
    connect(&AppContext::instance(), &AppContext::profilesChanged, profile, [profile]() {
        const QString cur = profile->currentText();
        QSignalBlocker block(profile);
        profile->clear();
        for (const auto& p : AppContext::instance().profiles().profiles()) profile->addItem(QString::fromStdString(p.name));
        profile->setCurrentText(cur);
    });
    form->addRow(tr("Default profile"), profile);

    auto* dir = new QLineEdit(QString::fromStdString(s_.startup_directory));
    dir->setPlaceholderText(tr("Home folder"));
    auto* browse = new QPushButton(tr("Browse…"));
    connect(dir, &QLineEdit::editingFinished, this, [this, dir]() {
        s_.startup_directory = dir->text().trimmed().toStdString();
        commit();
    });
    connect(browse, &QPushButton::clicked, this, [this, dir]() {
        const QString d = QFileDialog::getExistingDirectory(this, tr("Start in"), QDir::homePath());
        if (d.isEmpty()) return;
        dir->setText(d);
        s_.startup_directory = d.toStdString();
        commit();
    });
    form->addRow(tr("New tabs start in"), row({dir, browse}));
    form->addRow(QString(), note(tr("Used when a profile doesn't set its own folder.")));
    return pb.done();
}

// ---- Linux integration --------------------------------------------------------------------------

QWidget* SettingsDialog::buildLinux() {
    PageBuilder pb(tr("Linux Integration"), tr("Desktop launcher, icon and configuration files."));
    auto* form = pb.card(tr("This system"));
    QString distro = QSysInfo::prettyProductName();
    QFile osr(QStringLiteral("/etc/os-release"));
    if (osr.open(QIODevice::ReadOnly)) {
        auto kv = dev::parse_os_release(osr.readAll().toStdString());
        if (kv.count("PRETTY_NAME")) distro = QString::fromStdString(kv["PRETTY_NAME"]);
    }
    form->addRow(tr("Distribution"), new QLabel(distro));
    form->addRow(tr("Desktop"), new QLabel(qEnvironmentVariable("XDG_CURRENT_DESKTOP", tr("unknown"))));
    form->addRow(tr("Session"), new QLabel(qEnvironmentVariable("XDG_SESSION_TYPE", tr("unknown"))));
    form->addRow(tr("Executable"), new QLabel(QCoreApplication::applicationFilePath()));

    auto* launcher = pb.card(tr("Application launcher"));
    auto* state = new QLabel;
    state->setWordWrap(true);
    auto* install = new QPushButton(Icons::icon(QStringLiteral("download")), tr("Install launcher for my user"));
    auto* remove = new QPushButton(Icons::icon(QStringLiteral("trash")), tr("Remove my launcher"));
    auto refresh = [state, remove]() {
        const QString sys = system_desktop_file();
        const bool user = QFileInfo::exists(user_desktop_file());
        if (!sys.isEmpty()) state->setText(tr("Installed system-wide by your package: %1").arg(sys));
        else if (user) state->setText(tr("Installed for your user: %1").arg(user_desktop_file()));
        else state->setText(tr("Not installed — Zephyr won't appear in your application menu yet."));
        remove->setEnabled(user);
    };
    connect(install, &QPushButton::clicked, this, [this, refresh]() {
        // An AppImage must point the launcher at the .AppImage file, not the
        // temporary mount it is running from.
        QString exec = qEnvironmentVariable("APPIMAGE");
        if (exec.isEmpty()) exec = QCoreApplication::applicationFilePath();
        const QString icon_dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                 QStringLiteral("/icons/hicolor/scalable/apps");
        QDir().mkpath(icon_dir);
        QFile::remove(icon_dir + QStringLiteral("/zephyr-terminal.svg"));
        QFile::copy(QStringLiteral(":/app/zephyr-terminal.svg"), icon_dir + QStringLiteral("/zephyr-terminal.svg"));
        QFile(icon_dir + QStringLiteral("/zephyr-terminal.svg")).setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        QDir().mkpath(QFileInfo(user_desktop_file()).absolutePath());
        QFile f(user_desktop_file());
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::warning(this, tr("Launcher"), tr("Could not write %1").arg(user_desktop_file()));
            return;
        }
        QTextStream out(&f);
        // Exec quoting per the Desktop Entry spec: inside a quoted argument
        // " ` $ \ are backslash-escaped, and the value itself is escaped
        // again, so one literal backslash is written as four; % is doubled.
        QString quoted = exec;
        quoted.replace('\\', QStringLiteral("\\\\\\\\"))
              .replace('"', QStringLiteral("\\\\\""))
              .replace('`', QStringLiteral("\\\\`"))
              .replace('$', QStringLiteral("\\\\$"))
              .replace('%', QStringLiteral("%%"));
        out << "[Desktop Entry]\nType=Application\nName=Zephyr Terminal\nGenericName=Terminal\n"
            << "Comment=Modern terminal emulator\nExec=\"" << quoted << "\"\nIcon=zephyr-terminal\n"
            << "Terminal=false\nCategories=System;TerminalEmulator;\nKeywords=shell;prompt;command;commandline;cmd;\n"
            << "StartupNotify=true\nStartupWMClass=zephyr-terminal\n";
        f.close();
        refresh();
    });
    connect(remove, &QPushButton::clicked, this, [refresh]() {
        QFile::remove(user_desktop_file());
        refresh();
    });
    refresh();
    launcher->addRow(QString(), state);
    launcher->addRow(QString(), row({install, remove}, -1));

    auto* files = pb.card(tr("Configuration"));
    const QString dir = QString::fromStdString(config::config_dir());
    files->addRow(tr("Folder"), new QLabel(dir));
    auto* open = new QPushButton(Icons::icon(QStringLiteral("folder")), tr("Open folder"));
    connect(open, &QPushButton::clicked, this, [dir]() {
        QDir().mkpath(dir);
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    files->addRow(QString(), open);
    files->addRow(QString(), note(tr("terminal.toml, keybindings.toml, profiles.toml and session.toml are plain text "
                                     "and safe to edit or keep in your dotfiles.")));
    return pb.done();
}

// ---- NixOS -------------------------------------------------------------------------------------------

QWidget* SettingsDialog::buildNix() {
    PageBuilder pb(tr("NixOS"), tr("Zephyr ships a Nix flake with a package, a dev shell and a NixOS module."));
    auto* form = pb.card(tr("Detected environment"));
    bool nixos = QFileInfo::exists(QStringLiteral("/etc/NIXOS"));
    QFile osr(QStringLiteral("/etc/os-release"));
    if (osr.open(QIODevice::ReadOnly)) {
        auto kv = dev::parse_os_release(osr.readAll().toStdString());
        nixos = nixos || kv["ID"] == "nixos";
    }
    const bool has_nix = !QStandardPaths::findExecutable(QStringLiteral("nix")).isEmpty();
    const bool from_store = QCoreApplication::applicationFilePath().startsWith(QLatin1String("/nix/store/"));
    auto yes_no = [this](bool v) { return v ? tr("Yes") : tr("No"); };
    form->addRow(tr("Running on NixOS"), new QLabel(yes_no(nixos)));
    form->addRow(tr("nix command available"), new QLabel(yes_no(has_nix)));
    form->addRow(tr("This Zephyr comes from the Nix store"), new QLabel(yes_no(from_store)));

    const QString repo = QStringLiteral("github:charanbalaji2005/Meridian-Shell");
    struct Snippet { QString title; QString text; };
    const QList<Snippet> snippets = {
        {tr("Try it without installing"), QStringLiteral("nix run %1").arg(repo)},
        {tr("Install into your user profile"), QStringLiteral("nix profile install %1").arg(repo)},
        {tr("NixOS configuration (flake)"),
         QStringLiteral("# flake.nix\n"
                        "inputs.zephyr.url = \"%1\";\n\n"
                        "# in nixosConfigurations.<host>.modules:\n"
                        "inputs.zephyr.nixosModules.default\n"
                        "{ programs.zephyr-terminal.enable = true; }").arg(repo)},
        {tr("Development shell"), QStringLiteral("nix develop %1").arg(repo)},
    };
    for (const auto& s : snippets) {
        auto* card = pb.plainCard();
        auto* title = new QLabel(s.title);
        title->setStyleSheet(QStringLiteral("font-weight:600;"));
        auto* code = new QPlainTextEdit(s.text);
        code->setReadOnly(true);
        QFont mono(QStringLiteral("monospace"));
        mono.setStyleHint(QFont::Monospace);
        code->setFont(mono);
        code->setFixedHeight(QFontMetrics(mono).height() * (static_cast<int>(s.text.count('\n')) + 1) + 20);
        auto* copy = new QPushButton(Icons::icon(QStringLiteral("copy")), tr("Copy"));
        const QString text = s.text;
        connect(copy, &QPushButton::clicked, this, [text, copy]() {
            QApplication::clipboard()->setText(text);
            copy->setText(QObject::tr("Copied"));
        });
        card->addWidget(row({title, copy}));
        card->addWidget(code);
    }
    pb.lay->addWidget(note(tr("See docs/packaging/nixos.md for details. Repository: <a href=\"%1\">%1</a>").arg(QLatin1String(kRepoUrl))));
    return pb.done();
}

// ---- Performance ----------------------------------------------------------------------------------------

QWidget* SettingsDialog::buildPerformance() {
    PageBuilder pb(tr("Performance"), tr("Zephyr only repaints when output arrives, coalescing bursts into frames."));
    auto* form = pb.card();
    auto* fps = new QComboBox;
    fps->addItem(tr("30 frames per second (lowest power)"), 30);
    fps->addItem(tr("60 frames per second (default)"), 60);
    fps->addItem(tr("120 frames per second (high refresh displays)"), 120);
    fps->setCurrentIndex(std::max(0, fps->findData(s_.max_fps)));
    connect(fps, &QComboBox::currentIndexChanged, this, [this, fps](int) {
        s_.max_fps = fps->currentData().toInt();
        commit();
    });
    form->addRow(tr("Maximum repaint rate"), fps);
    form->addRow(QString(), note(tr("When idle, the only timer is the cursor blink (disable blinking for zero "
                                    "wake-ups). Large scrollback uses more memory: roughly 40 bytes per cell.")));
    return pb.done();
}

} // namespace zephyr::gui
