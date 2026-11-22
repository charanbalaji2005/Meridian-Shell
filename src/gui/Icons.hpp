#pragma once
// src/gui/Icons.hpp
//
// Theme-aware SVG icons. Every icon is a stroke-only 24x24 SVG (Lucide
// geometry, resources/icons/ui/) using `currentColor`; we substitute the
// requested color and render crisply at the target size and device pixel
// ratio. Results are cached, so repeated lookups are cheap.

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

namespace zephyr::gui {

class Icons {
public:
    static QPixmap pixmap(const QString& name, const QColor& color, int logical_size, qreal dpr);
    // Icon with Normal (muted), Active (text) and Disabled states derived
    // from the current chrome colors.
    static QIcon icon(const QString& name);
    static QIcon icon(const QString& name, const QColor& color);
    static void clearCache(); // call after theme changes
};

} // namespace zephyr::gui
