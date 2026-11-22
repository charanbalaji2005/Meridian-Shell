#include "TerminalView.hpp"

#include "AppContext.hpp"
#include "UiTheme.hpp"

#include "../core/vt/input_encoder.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyleHints>
#include <QUrl>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <optional>

namespace zephyr::gui {

namespace {

bool is_word_char(char32_t cp) {
    if (cp > 0x7F) return true; // treat non-ASCII letters as word characters
    return (cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z') || (cp >= '0' && cp <= '9') ||
           cp == '_' || cp == '-' || cp == '.' || cp == '/' || cp == '~' || cp == ':' || cp == '@' ||
           cp == '+' || cp == '%' || cp == '=';
}

// Glyphs that must sit exactly on the cell grid (box drawing, blocks,
// powerline / Nerd Font private-use icons) are drawn one cell at a time.
bool needs_exact_cell(char32_t cp) {
    return cp >= 0x2500 || cp > 0xFFFF;
}

std::optional<vt::Key> map_key(int key) {
    switch (key) {
        case Qt::Key_Up: return vt::Key::Up;
        case Qt::Key_Down: return vt::Key::Down;
        case Qt::Key_Right: return vt::Key::Right;
        case Qt::Key_Left: return vt::Key::Left;
        case Qt::Key_Home: return vt::Key::Home;
        case Qt::Key_End: return vt::Key::End;
        case Qt::Key_Insert: return vt::Key::Insert;
        case Qt::Key_Delete: return vt::Key::Delete;
        case Qt::Key_PageUp: return vt::Key::PageUp;
        case Qt::Key_PageDown: return vt::Key::PageDown;
        case Qt::Key_F1: return vt::Key::F1;
        case Qt::Key_F2: return vt::Key::F2;
        case Qt::Key_F3: return vt::Key::F3;
        case Qt::Key_F4: return vt::Key::F4;
        case Qt::Key_F5: return vt::Key::F5;
        case Qt::Key_F6: return vt::Key::F6;
        case Qt::Key_F7: return vt::Key::F7;
        case Qt::Key_F8: return vt::Key::F8;
        case Qt::Key_F9: return vt::Key::F9;
        case Qt::Key_F10: return vt::Key::F10;
        case Qt::Key_F11: return vt::Key::F11;
        case Qt::Key_F12: return vt::Key::F12;
        case Qt::Key_Backspace: return vt::Key::Backspace;
        case Qt::Key_Return: case Qt::Key_Enter: return vt::Key::Enter;
        case Qt::Key_Tab: case Qt::Key_Backtab: return vt::Key::Tab;
        case Qt::Key_Escape: return vt::Key::Escape;
        default: return std::nullopt;
    }
}

} // namespace

bool TerminalView::Selection::contains(int line, int col) const {
    if (!active) return false;
    int l0 = a_line, c0 = a_col, l1 = b_line, c1 = b_col;
    if (l0 > l1 || (l0 == l1 && c0 > c1)) { std::swap(l0, l1); std::swap(c0, c1); }
    if (line < l0 || line > l1) return false;
    if (line == l0 && col < c0) return false;
    if (line == l1 && col > c1) return false;
    return true;
}

TerminalView::TerminalView(const LaunchSpec& spec, QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMouseTracking(false);
    setCursor(Qt::IBeamCursor);
    setAccessibleName(tr("Terminal"));

    const auto& s = AppContext::instance().settings();
    bridge_ = new PtyBridge(24, 80, static_cast<std::size_t>(s.scrollback_lines), this);

    repaint_timer_.setSingleShot(true);
    connect(&repaint_timer_, &QTimer::timeout, this, [this]() {
        update();
        emit viewChanged();
    });
    blink_timer_.setInterval(QGuiApplication::styleHints()->cursorFlashTime() > 0
                                 ? QGuiApplication::styleHints()->cursorFlashTime() / 2 : 530);
    connect(&blink_timer_, &QTimer::timeout, this, [this]() {
        blink_on_ = !blink_on_;
        update(cursorRect().toAlignedRect().adjusted(-2, -2, 2, 2));
    });
    search_timer_.setSingleShot(true);
    search_timer_.setInterval(150);
    connect(&search_timer_, &QTimer::timeout, this, &TerminalView::recomputeSearch);

    connect(bridge_, &PtyBridge::outputReceived, this, [this]() {
        if (!query_.isEmpty()) search_timer_.start();
        scheduleRepaint();
    });
    connect(bridge_, &PtyBridge::titleChanged, this, &TerminalView::titleChanged);
    connect(bridge_, &PtyBridge::finished, this, &TerminalView::finished);

    applySettings();
    if (!bridge_->start(spec)) {
        start_failed_ = true;
        // Show the failure in the terminal itself instead of a blank pane.
        std::string msg = "\x1b[1;31mZephyr: could not start '" + spec.program.toStdString() + "': " +
                          bridge_->errorString().toStdString() + "\x1b[0m\r\n";
        vt::AnsiParser(bridge_->screen()).feed(msg);
    }
}

TerminalView::~TerminalView() = default;

void TerminalView::applySettings() {
    const auto& s = AppContext::instance().settings();
    const auto& theme = UiTheme::terminalTheme();

    // Colors: 16 from the theme, the standard 6x6x6 cube and grey ramp.
    for (int i = 0; i < 16; ++i) palette_[i] = UiTheme::toQColor(theme.ansi[static_cast<std::size_t>(i)]);
    static const int cube[6] = {0, 95, 135, 175, 215, 255};
    for (int i = 0; i < 216; ++i) palette_[16 + i] = QColor(cube[i / 36], cube[(i / 6) % 6], cube[i % 6]);
    for (int i = 0; i < 24; ++i) palette_[232 + i] = QColor(8 + i * 10, 8 + i * 10, 8 + i * 10);
    bg_ = UiTheme::toQColor(theme.background);
    fg_ = UiTheme::toQColor(theme.foreground);
    cursor_color_ = UiTheme::toQColor(theme.cursor);
    selection_color_ = UiTheme::toQColor(theme.selection, 200);
    const auto chrome = UiTheme::colors();
    match_color_ = QColor(chrome.warning.red(), chrome.warning.green(), chrome.warning.blue(), 90);
    current_match_color_ = QColor(chrome.accent.red(), chrome.accent.green(), chrome.accent.blue(), 170);
    window_alpha_ = s.window_opacity * 255 / 100;
    paint_background_ = !s.glow_border;

    bg_image_ = QPixmap();
    bg_scaled_ = QPixmap();
    if (!s.background_image.empty()) bg_image_.load(QString::fromStdString(s.background_image));
    bg_image_opacity_ = s.background_image_opacity / 100.0;

    cursor_style_ = QString::fromStdString(s.cursor_style);
    cursor_blink_setting_ = s.cursor_blink;
    padding_ = s.padding;
    copy_on_select_ = s.copy_on_select;
    scroll_multiplier_ = s.scroll_multiplier;
    warn_multiline_paste_ = s.warn_multiline_paste;
    ligatures_ = s.font_ligatures;
    repaint_timer_.setInterval(std::max(1, 1000 / std::max(1, s.max_fps)));
    bridge_->screen().set_scrollback_limit(static_cast<std::size_t>(s.scrollback_lines));

    base_font_ = QFont(QString::fromStdString(s.font_family));
    base_font_.setStyleHint(QFont::Monospace, QFont::PreferMatch);
    base_font_.setFixedPitch(true);
    base_font_.setKerning(false);
    base_font_.setPointSizeF(std::clamp(s.font_size + zoom_delta_, 6, 72));
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    if (!ligatures_) {
        base_font_.setFeature(QFont::Tag("liga"), 0);
        base_font_.setFeature(QFont::Tag("calt"), 0);
    }
#else
    // Pre-6.7 Qt has no per-feature switch; disabling shaping turns off
    // ligatures (and contextual alternates) for Latin text.
    base_font_.setStyleStrategy(ligatures_ ? QFont::PreferDefault : QFont::PreferNoShaping);
#endif
    style_ready_.fill(false);
    updateFontMetrics();
    resetCursorBlink();
    update();
}

void TerminalView::updateFontMetrics() {
    QFontMetricsF fm(base_font_);
    cell_w_ = std::max<qreal>(1.0, fm.horizontalAdvance(QLatin1Char('M')));
    const qreal natural = std::max<qreal>(1.0, fm.height());
    const int percent = AppContext::instance().settings().line_height_percent;
    cell_h_ = std::ceil(natural * percent / 100.0);
    baseline_ = (cell_h_ - natural) / 2.0 + fm.ascent();
    updateGridSize();
}

void TerminalView::updateGridSize() {
    const int cols = std::max(2, static_cast<int>((width() - 2 * padding_) / cell_w_));
    const int rows = std::max(1, static_cast<int>((height() - 2 * padding_) / cell_h_));
    bridge_->resize(rows, cols);
}

const QFont& TerminalView::styledFont(bool bold, bool italic, bool underline, bool strike) {
    const int key = (bold ? 1 : 0) | (italic ? 2 : 0) | (underline ? 4 : 0) | (strike ? 8 : 0);
    if (!style_ready_[static_cast<std::size_t>(key)]) {
        QFont f = base_font_;
        f.setBold(bold);
        f.setItalic(italic);
        f.setUnderline(underline);
        f.setStrikeOut(strike);
        style_fonts_[static_cast<std::size_t>(key)] = f;
        style_ready_[static_cast<std::size_t>(key)] = true;
    }
    return style_fonts_[static_cast<std::size_t>(key)];
}

QColor TerminalView::resolve(const vt::Color& c, bool is_fg, bool bold) const {
    switch (c.kind) {
        case vt::Color::Kind::Default: return is_fg ? fg_ : bg_;
        case vt::Color::Kind::Indexed: {
            int idx = c.index;
            if (is_fg && bold && idx < 8) idx += 8; // classic "bold is bright"
            return palette_[static_cast<std::size_t>(idx)];
        }
        case vt::Color::Kind::Rgb: return QColor(c.r, c.g, c.b);
    }
    return is_fg ? fg_ : bg_;
}

void TerminalView::scheduleRepaint() {
    if (!repaint_timer_.isActive()) repaint_timer_.start();
}

void TerminalView::resetCursorBlink() {
    blink_on_ = true;
    const auto& m = bridge_->screen().modes();
    if (cursor_blink_setting_ && m.cursor_blink && hasFocus()) blink_timer_.start();
    else blink_timer_.stop();
}

// ---- Painting ---------------------------------------------------------------

QRectF TerminalView::cursorRect() const {
    const auto& sb = bridge_->screen();
    return QRectF(padding_ + sb.cursor_col() * cell_w_, padding_ + sb.cursor_row() * cell_h_, cell_w_, cell_h_);
}

void TerminalView::paintEvent(QPaintEvent* e) {
    QPainter p(this);
    const auto& sb = bridge_->screen();
    const int rows = sb.rows();
    const int cols = sb.cols();

    // Background: theme color at the configured window opacity. With the glow
    // frame enabled the frame already painted the translucent body behind us.
    if (paint_background_) {
        QColor bg = bg_;
        bg.setAlpha(window_alpha_);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(e->rect(), bg);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    if (!bg_image_.isNull() && bg_image_opacity_ > 0.0) {
        if (bg_scaled_.size() != size() * devicePixelRatioF()) {
            bg_scaled_ = bg_image_.scaled(size() * devicePixelRatioF(), Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
            bg_scaled_.setDevicePixelRatio(devicePixelRatioF());
        }
        p.setOpacity(bg_image_opacity_);
        const QSizeF logical = bg_scaled_.deviceIndependentSize();
        p.drawPixmap(QPointF((width() - logical.width()) / 2, (height() - logical.height()) / 2), bg_scaled_);
        p.setOpacity(1.0);
    }

    p.translate(padding_, padding_);
    const int first_row = std::max(0, static_cast<int>((e->rect().top() - padding_) / cell_h_));
    const int last_row = std::min(rows - 1, static_cast<int>((e->rect().bottom() - padding_) / cell_h_) + 1);
    const std::size_t match_start = static_cast<std::size_t>(
        std::lower_bound(matches_.begin(), matches_.end(), sb.viewport_to_line(first_row),
                         [](const vt::SearchMatch& m, int line) { return m.line < line; }) - matches_.begin());

    for (int r = first_row; r <= last_row; ++r) {
        const int line = sb.viewport_to_line(r);
        const qreal y = r * cell_h_;

        // Pass 1: backgrounds, selection, search highlights.
        for (int c = 0; c < cols; ++c) {
            const vt::Cell& cell = sb.visible_cell(r, c);
            if (cell.width == 0) continue;
            const qreal w = cell_w_ * (cell.width == 2 ? 2 : 1);
            QColor cbg = cell.attrs.reverse ? resolve(cell.attrs.fg, true, cell.attrs.bold)
                                            : resolve(cell.attrs.bg, false, false);
            const bool is_default_bg = !cell.attrs.reverse && cell.attrs.bg.kind == vt::Color::Kind::Default;
            if (!is_default_bg) p.fillRect(QRectF(c * cell_w_, y, w + 0.5, cell_h_), cbg);
            if (sel_.contains(line, c)) p.fillRect(QRectF(c * cell_w_, y, w + 0.5, cell_h_), selection_color_);
        }
        for (std::size_t i = match_start; i < static_cast<std::size_t>(matches_.size()); ++i) {
            const auto& m = matches_[static_cast<int>(i)];
            if (m.line < line) continue;
            if (m.line > line) break;
            p.fillRect(QRectF(m.col * cell_w_, y, m.length * cell_w_, cell_h_),
                       static_cast<int>(i) == current_match_ ? current_match_color_ : match_color_);
        }

        // Pass 2: text, batched into runs of identical style.
        QString run;
        int run_col = -1;
        QColor run_fg;
        int run_style = -1;
        auto flush = [&]() {
            if (run_col < 0) return;
            p.setFont(styledFont(run_style & 1, run_style & 2, run_style & 4, run_style & 8));
            p.setPen(run_fg);
            p.drawText(QPointF(run_col * cell_w_, y + baseline_), run);
            run.clear();
            run_col = -1;
        };
        for (int c = 0; c < cols; ++c) {
            const vt::Cell& cell = sb.visible_cell(r, c);
            if (cell.width == 0) continue;
            const char32_t cp = cell.codepoint;
            if (cp == 0 || cp == U' ' || cell.attrs.hidden) { flush(); continue; }
            QColor cfg = cell.attrs.reverse ? resolve(cell.attrs.bg, false, false)
                                            : resolve(cell.attrs.fg, true, cell.attrs.bold);
            if (cell.attrs.dim) cfg.setAlphaF(cfg.alphaF() * 0.6);
            const int style = (cell.attrs.bold ? 1 : 0) | (cell.attrs.italic ? 2 : 0) |
                              (cell.attrs.underline ? 4 : 0) | (cell.attrs.strikethrough ? 8 : 0);
            const QString glyph = QString::fromUcs4(&cp, 1);
            if (cell.width == 2 || needs_exact_cell(cp)) {
                flush();
                p.setFont(styledFont(style & 1, style & 2, style & 4, style & 8));
                p.setPen(cfg);
                p.drawText(QPointF(c * cell_w_, y + baseline_), glyph);
                continue;
            }
            if (run_col >= 0 && (cfg != run_fg || style != run_style)) flush();
            if (run_col < 0) { run_col = c; run_fg = cfg; run_style = style; }
            run += glyph;
        }
        flush();
    }

    // Inline images (Kitty graphics protocol / Sixel) on the live screen.
    if (sb.view_offset() == 0) {
        for (const auto& pl : sb.graphics().placements()) {
            const auto* img = sb.graphics().find_image(pl.image_id);
            if (!img || img->rgba_data.empty() || img->width <= 0 || img->height <= 0) continue;
            QImage qimg(img->rgba_data.data(), img->width, img->height, img->width * 4, QImage::Format_RGBA8888);
            p.drawImage(QRectF(pl.col * cell_w_ + pl.x_offset, pl.row * cell_h_ + pl.y_offset,
                               pl.cols_spanned * cell_w_, pl.rows_spanned * cell_h_), qimg);
        }
    }

    // Cursor
    const auto& m = sb.modes();
    const bool blinking = cursor_blink_setting_ && m.cursor_blink;
    if (m.cursor_visible && sb.view_offset() == 0 && (!blinking || blink_on_ || !hasFocus())) {
        QString shape = cursor_style_;
        switch (m.cursor_shape) {
            case vt::AppCursorShape::Block: shape = QStringLiteral("block"); break;
            case vt::AppCursorShape::Underline: shape = QStringLiteral("underline"); break;
            case vt::AppCursorShape::Bar: shape = QStringLiteral("bar"); break;
            case vt::AppCursorShape::Default: break;
        }
        const QRectF cr = cursorRect().translated(-padding_, -padding_);
        if (!hasFocus()) {
            p.setPen(QPen(cursor_color_, 1));
            p.setBrush(Qt::NoBrush);
            p.drawRect(cr.adjusted(0.5, 0.5, -0.5, -0.5));
        } else if (shape == QLatin1String("underline")) {
            p.fillRect(QRectF(cr.left(), cr.bottom() - 2, cr.width(), 2), cursor_color_);
        } else if (shape == QLatin1String("bar")) {
            p.fillRect(QRectF(cr.left(), cr.top(), 2, cr.height()), cursor_color_);
        } else {
            p.fillRect(cr, cursor_color_);
            const vt::Cell& cell = sb.cell_at(sb.cursor_row(), sb.cursor_col());
            if (cell.codepoint != 0 && cell.codepoint != U' ') {
                p.setFont(styledFont(cell.attrs.bold, cell.attrs.italic, false, false));
                p.setPen(bg_);
                p.drawText(QPointF(cr.left(), cr.top() + baseline_), QString::fromUcs4(&cell.codepoint, 1));
            }
        }
    }
}

void TerminalView::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    updateGridSize();
    emit viewChanged();
}

// ---- Keyboard ---------------------------------------------------------------

bool TerminalView::event(QEvent* e) {
    // Tab/Backtab must reach the shell (completion), not move widget focus.
    if (e->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return true;
        }
    }
    return QWidget::event(e);
}

void TerminalView::writeInput(const QByteArray& bytes) {
    if (bytes.isEmpty()) return;
    snapToBottom();
    if (sel_.active) {
        sel_.active = false;
        update();
    }
    resetCursorBlink();
    bridge_->write(bytes);
}

void TerminalView::snapToBottom() {
    if (bridge_->screen().view_offset() != 0) {
        bridge_->screen().reset_view();
        update();
        emit viewChanged();
    }
}

void TerminalView::keyPressEvent(QKeyEvent* e) {
    const Qt::KeyboardModifiers qm = e->modifiers();
    vt::KeyMods mods;
    mods.shift = qm & Qt::ShiftModifier;
    mods.ctrl = qm & Qt::ControlModifier;
    mods.alt = qm & Qt::AltModifier;
    const bool app_cursor = bridge_->screen().modes().application_cursor_keys;

    // Shift+PageUp/PageDown/Home/End scroll the history locally (standard
    // terminal behaviour) instead of being sent to the program.
    if (mods.shift && !mods.ctrl && !mods.alt && !bridge_->screen().in_alt_screen()) {
        const int page = std::max(1, bridge_->screen().rows() - 1);
        switch (e->key()) {
            case Qt::Key_PageUp: setViewOffset(viewOffset() + page); return;
            case Qt::Key_PageDown: setViewOffset(viewOffset() - page); return;
            case Qt::Key_Home: setViewOffset(historySize()); return;
            case Qt::Key_End: setViewOffset(0); return;
            default: break;
        }
    }

    if (auto key = map_key(e->key())) {
        if (e->key() == Qt::Key_Backtab) mods.shift = true;
        writeInput(QByteArray::fromStdString(vt::encode_key(*key, mods, app_cursor)));
        e->accept();
        return;
    }

    QString text = e->text();
    if (mods.ctrl && text.isEmpty()) {
        // Some layouts deliver no text for Ctrl+<key>: derive it from the key code.
        const int k = e->key();
        if (k >= Qt::Key_A && k <= Qt::Key_Z) text = QChar('a' + (k - Qt::Key_A));
        else if (k == Qt::Key_Space) text = QStringLiteral(" ");
        else if (k >= Qt::Key_BracketLeft && k <= Qt::Key_Underscore) text = QChar(k);
        else if (k == Qt::Key_Slash || k == Qt::Key_Question) text = QChar(k);
        else if (k >= Qt::Key_2 && k <= Qt::Key_8) text = QChar(k);
    }
    if (text.isEmpty()) {
        QWidget::keyPressEvent(e);
        return;
    }
    // Qt already turned Ctrl+letter into a control char in text() on some
    // platforms; only re-encode printable text.
    if (text.size() == 1 && text.at(0).unicode() < 0x20) {
        writeInput(text.toUtf8());
    } else {
        writeInput(QByteArray::fromStdString(vt::encode_text(text.toStdString(), mods)));
    }
    e->accept();
}

void TerminalView::inputMethodEvent(QInputMethodEvent* e) {
    if (!e->commitString().isEmpty()) writeInput(e->commitString().toUtf8());
    e->accept();
}

QVariant TerminalView::inputMethodQuery(Qt::InputMethodQuery q) const {
    if (q == Qt::ImCursorRectangle) return cursorRect().toRect();
    if (q == Qt::ImFont) return base_font_;
    return QWidget::inputMethodQuery(q);
}

// ---- Clipboard ----------------------------------------------------------------

QString TerminalView::selectedText() const {
    if (!sel_.active) return {};
    return QString::fromStdString(bridge_->screen().text_range(sel_.a_line, sel_.a_col, sel_.b_line, sel_.b_col));
}

void TerminalView::copySelection() {
    const QString t = selectedText();
    if (!t.isEmpty()) QApplication::clipboard()->setText(t, QClipboard::Clipboard);
}

void TerminalView::paste() { pasteText(QApplication::clipboard()->text(QClipboard::Clipboard)); }

void TerminalView::pasteText(const QString& text) {
    if (text.isEmpty()) return;
    const std::string utf8 = text.toStdString();
    const bool bracketed = bridge_->screen().modes().bracketed_paste;
    if (!bracketed && warn_multiline_paste_ && vt::is_multiline_paste(utf8)) {
        QStringList lines = text.split('\n');
        QString preview = QStringList(lines.mid(0, 6)).join('\n');
        if (lines.size() > 6) preview += QStringLiteral("\n…");
        QMessageBox box(QMessageBox::Warning, tr("Paste multiple lines?"),
                        tr("The clipboard contains %n line(s). Each line may run as a command "
                           "as soon as it is pasted.", nullptr, static_cast<int>(lines.size())),
                        QMessageBox::Cancel, this);
        box.setInformativeText(preview);
        QAbstractButton* ok = box.addButton(tr("Paste"), QMessageBox::AcceptRole);
        box.setDefaultButton(QMessageBox::Cancel);
        box.exec();
        if (box.clickedButton() != ok) return;
    }
    writeInput(QByteArray::fromStdString(vt::prepare_paste(utf8, bracketed)));
}

void TerminalView::insertText(const QString& text) {
    QString one_line = text;
    one_line.replace(QLatin1String("\r\n"), QLatin1String(" "));
    one_line.replace('\n', ' ');
    one_line.replace('\r', ' ');
    if (one_line.isEmpty()) return;
    // Bracketed when the shell supports it, so it is inserted literally;
    // newlines were removed above, so nothing can execute.
    writeInput(QByteArray::fromStdString(
        vt::prepare_paste(one_line.toStdString(), bridge_->screen().modes().bracketed_paste)));
    setFocus();
}

void TerminalView::selectAll() {
    const auto& sb = bridge_->screen();
    sel_ = Selection{true, 0, 0, sb.total_lines() - 1, sb.cols() - 1};
    finishSelection();
    update();
}

void TerminalView::clearScrollback() {
    bridge_->screen().clear_scrollback();
    sel_.active = false;
    recomputeSearch();
    update();
    emit viewChanged();
}

// ---- Zoom ------------------------------------------------------------------------

void TerminalView::zoomIn() { zoom_delta_ = std::min(zoom_delta_ + 1, 40); applySettings(); }
void TerminalView::zoomOut() { zoom_delta_ = std::max(zoom_delta_ - 1, -20); applySettings(); }
void TerminalView::zoomReset() { zoom_delta_ = 0; applySettings(); }

// ---- Mouse -------------------------------------------------------------------------

TerminalView::CellPos TerminalView::cellAt(const QPoint& pos, bool clamp) const {
    const auto& sb = bridge_->screen();
    int col = static_cast<int>((pos.x() - padding_) / cell_w_);
    int row = static_cast<int>(std::floor((pos.y() - padding_) / cell_h_));
    if (clamp) {
        col = std::clamp(col, 0, sb.cols() - 1);
        row = std::clamp(row, 0, sb.rows() - 1);
    }
    return {sb.viewport_to_line(row), col};
}

void TerminalView::mousePressEvent(QMouseEvent* e) {
    setFocus();
    const CellPos p = cellAt(e->pos());
    if (e->button() == Qt::LeftButton) {
        // Ctrl+click opens an OSC 8 hyperlink — explicit user action only,
        // and only for safe schemes.
        if (e->modifiers() & Qt::ControlModifier) {
            const auto& cell = bridge_->screen().line_cell(p.line, p.col);
            const QUrl url(QString::fromStdString(bridge_->screen().get_hyperlink(cell.attrs.hyperlink_id)));
            static const QStringList safe = {"http", "https", "file", "mailto"};
            if (url.isValid() && safe.contains(url.scheme())) {
                QDesktopServices::openUrl(url);
                return;
            }
        }
        if (last_double_click_.isValid() &&
            last_double_click_.elapsed() < QGuiApplication::styleHints()->mouseDoubleClickInterval()) {
            selectLineAt(p.line); // triple click
            last_double_click_.invalidate();
            return;
        }
        if (e->modifiers() & Qt::ShiftModifier && sel_.active) {
            sel_.b_line = p.line;
            sel_.b_col = p.col;
        } else {
            sel_ = Selection{false, p.line, p.col, p.line, p.col};
        }
        selecting_ = true;
        update();
    } else if (e->button() == Qt::MiddleButton) {
        QClipboard* cb = QApplication::clipboard();
        pasteText(cb->supportsSelection() ? cb->text(QClipboard::Selection) : cb->text());
    }
}

void TerminalView::mouseMoveEvent(QMouseEvent* e) {
    if (!selecting_) return;
    // Dragging past the edges scrolls through history.
    if (e->pos().y() < 0) setViewOffset(viewOffset() + 1);
    else if (e->pos().y() > height()) setViewOffset(viewOffset() - 1);
    const CellPos p = cellAt(e->pos());
    sel_.b_line = p.line;
    sel_.b_col = p.col;
    sel_.active = sel_.a_line != sel_.b_line || sel_.a_col != sel_.b_col;
    update();
}

void TerminalView::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && selecting_) {
        selecting_ = false;
        finishSelection();
    }
}

void TerminalView::mouseDoubleClickEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    selectWordAt(cellAt(e->pos()));
    last_double_click_.start();
}

void TerminalView::finishSelection() {
    if (!sel_.active) return;
    const QString t = selectedText();
    QClipboard* cb = QApplication::clipboard();
    if (cb->supportsSelection()) cb->setText(t, QClipboard::Selection); // X11/Wayland primary
    if (copy_on_select_) cb->setText(t, QClipboard::Clipboard);
}

void TerminalView::selectWordAt(const CellPos& p) {
    const auto& sb = bridge_->screen();
    auto cp_at = [&](int col) { return sb.line_cell(p.line, col).codepoint; };
    if (!is_word_char(cp_at(p.col))) return;
    int c0 = p.col, c1 = p.col;
    while (c0 > 0 && is_word_char(cp_at(c0 - 1))) --c0;
    while (c1 + 1 < sb.line_length(p.line) && is_word_char(cp_at(c1 + 1))) ++c1;
    sel_ = Selection{true, p.line, c0, p.line, c1};
    finishSelection();
    update();
}

void TerminalView::selectLineAt(int line) {
    sel_ = Selection{true, line, 0, line, std::max(0, bridge_->screen().line_length(line) - 1)};
    finishSelection();
    update();
}

void TerminalView::wheelEvent(QWheelEvent* e) {
    wheel_accum_ += e->angleDelta().y();
    const int notches = wheel_accum_ / 120;
    if (notches == 0) return;
    wheel_accum_ -= notches * 120;
    const int lines = notches * scroll_multiplier_;
    auto& sb = bridge_->screen();
    if (sb.in_alt_screen()) {
        // "Alternate scroll": full-screen apps (less, man, vim) receive arrows.
        const vt::Key k = lines > 0 ? vt::Key::Up : vt::Key::Down;
        QByteArray seq = QByteArray::fromStdString(vt::encode_key(k, {}, sb.modes().application_cursor_keys));
        bridge_->write(seq.repeated(std::abs(lines)));
    } else {
        setViewOffset(viewOffset() + lines);
    }
    e->accept();
}

void TerminalView::contextMenuEvent(QContextMenuEvent* e) { emit contextMenuRequested(e->globalPos()); }

void TerminalView::focusInEvent(QFocusEvent* e) {
    QWidget::focusInEvent(e);
    resetCursorBlink();
    update();
    emit focused();
}

void TerminalView::focusOutEvent(QFocusEvent* e) {
    QWidget::focusOutEvent(e);
    blink_timer_.stop();
    blink_on_ = true;
    update();
}

// ---- Scrollback viewport ---------------------------------------------------------

int TerminalView::historySize() const { return static_cast<int>(bridge_->screen().scrollback().size()); }
int TerminalView::viewOffset() const { return bridge_->screen().view_offset(); }

void TerminalView::setViewOffset(int offset) {
    auto& sb = bridge_->screen();
    const int before = sb.view_offset();
    sb.scroll_view(offset - before);
    if (sb.view_offset() != before) {
        update();
        emit viewChanged();
    }
}

// ---- Search ---------------------------------------------------------------------

int TerminalView::setSearch(const QString& query, bool case_sensitive) {
    query_ = query;
    query_case_ = case_sensitive;
    current_match_ = -1;
    recomputeSearch();
    if (!matches_.isEmpty()) {
        // Start from the match nearest the bottom of the screen.
        current_match_ = static_cast<int>(matches_.size()) - 1;
        scrollToMatch(current_match_);
    }
    emit searchResultsChanged(current_match_, static_cast<int>(matches_.size()));
    return static_cast<int>(matches_.size());
}

void TerminalView::recomputeSearch() {
    if (query_.isEmpty()) {
        matches_.clear();
    } else {
        auto found = bridge_->screen().find_all(query_.toStdString(), query_case_);
        matches_ = QVector<vt::SearchMatch>(found.begin(), found.end());
    }
    if (current_match_ >= static_cast<int>(matches_.size())) current_match_ = static_cast<int>(matches_.size()) - 1;
    emit searchResultsChanged(current_match_, static_cast<int>(matches_.size()));
    update();
}

void TerminalView::scrollToMatch(int index) {
    if (index < 0 || index >= static_cast<int>(matches_.size())) return;
    auto& sb = bridge_->screen();
    const int line = matches_[index].line;
    const int top = sb.viewport_to_line(0);
    if (line >= top && line < top + sb.rows()) { update(); return; }
    const int history = historySize();
    // Place the match roughly in the middle of the viewport.
    const int desired = std::clamp(history - line + sb.rows() / 2, 0, history);
    setViewOffset(desired);
}

void TerminalView::findNext() {
    if (matches_.isEmpty()) return;
    current_match_ = (current_match_ + 1) % static_cast<int>(matches_.size());
    scrollToMatch(current_match_);
    emit searchResultsChanged(current_match_, static_cast<int>(matches_.size()));
    update();
}

void TerminalView::findPrevious() {
    if (matches_.isEmpty()) return;
    current_match_ = (current_match_ - 1 + static_cast<int>(matches_.size())) % static_cast<int>(matches_.size());
    scrollToMatch(current_match_);
    emit searchResultsChanged(current_match_, static_cast<int>(matches_.size()));
    update();
}

void TerminalView::clearSearch() {
    query_.clear();
    matches_.clear();
    current_match_ = -1;
    search_timer_.stop();
    update();
}

} // namespace zephyr::gui
