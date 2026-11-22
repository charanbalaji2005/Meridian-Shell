#include "Icons.hpp"

#include "UiTheme.hpp"

#include <QFile>
#include <QHash>
#include <QPainter>
#include <QSvgRenderer>

namespace zephyr::gui {

namespace {

QHash<QString, QByteArray>& sources() {
    static QHash<QString, QByteArray> s;
    return s;
}
QHash<QString, QPixmap>& cache() {
    static QHash<QString, QPixmap> c;
    return c;
}

QByteArray load_svg(const QString& name) {
    auto it = sources().constFind(name);
    if (it != sources().constEnd()) return *it;
    QFile f(QStringLiteral(":/icons/%1.svg").arg(name));
    QByteArray data;
    if (f.open(QIODevice::ReadOnly)) data = f.readAll();
    // Slightly lighter stroke than Lucide's default 2px reads better at 16-18px.
    data.replace("stroke-width=\"2\"", "stroke-width=\"1.75\"");
    sources().insert(name, data);
    return data;
}

} // namespace

QPixmap Icons::pixmap(const QString& name, const QColor& color, int logical_size, qreal dpr) {
    const QString key = QStringLiteral("%1|%2|%3|%4").arg(name, color.name(QColor::HexArgb)).arg(logical_size).arg(dpr);
    auto it = cache().constFind(key);
    if (it != cache().constEnd()) return *it;

    QByteArray svg = load_svg(name);
    svg.replace("currentColor", color.name(QColor::HexRgb).toUtf8());
    const int px = qRound(logical_size * dpr);
    QPixmap pm(px, px);
    pm.fill(Qt::transparent);
    if (!svg.isEmpty()) {
        QSvgRenderer renderer(svg);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setOpacity(color.alphaF());
        renderer.render(&p, QRectF(0, 0, px, px));
    }
    pm.setDevicePixelRatio(dpr);
    cache().insert(key, pm);
    return pm;
}

QIcon Icons::icon(const QString& name, const QColor& color) {
    QIcon ic;
    for (int size : {16, 18, 20, 24, 32}) {
        for (qreal dpr : {1.0, 2.0}) ic.addPixmap(pixmap(name, color, size, dpr), QIcon::Normal);
    }
    return ic;
}

QIcon Icons::icon(const QString& name) {
    const auto c = UiTheme::colors();
    QColor disabled = c.muted;
    disabled.setAlpha(110);
    QIcon ic;
    for (int size : {16, 18, 20, 24, 32}) {
        for (qreal dpr : {1.0, 2.0}) {
            ic.addPixmap(pixmap(name, c.muted, size, dpr), QIcon::Normal);
            ic.addPixmap(pixmap(name, c.text, size, dpr), QIcon::Active);
            ic.addPixmap(pixmap(name, c.text, size, dpr), QIcon::Selected);
            ic.addPixmap(pixmap(name, disabled, size, dpr), QIcon::Disabled);
        }
    }
    return ic;
}

void Icons::clearCache() { cache().clear(); }

} // namespace zephyr::gui
