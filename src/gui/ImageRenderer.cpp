#include "ImageRenderer.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>

namespace meridian::gui {

ImageRenderer::ImageRenderer(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

bool ImageRenderer::loadImage(const QString& path) {
    if (path.isEmpty()) return false;

    QFileInfo info(path);
    if (!info.exists()) {
        // Try fallback to standard resources
        if (!path.startsWith(":/")) {
            return loadImage(":/resources/images/artwork.png");
        }
        return false;
    }

    QString ext = info.suffix().toLower();
    if (ext == "gif" || ext == "webp") {
        movie_ = std::make_unique<QMovie>(path);
        if (movie_->isValid()) {
            is_animated_ = true;
            connect(movie_.get(), &QMovie::frameChanged, this, [this](int) {
                update();
            });
            movie_->start();
            return true;
        }
    }

    is_animated_ = false;
    if (movie_) {
        movie_->stop();
        movie_.reset();
    }

    pixmap_.load(path);
    update();
    return !pixmap_.isNull();
}

void ImageRenderer::setCornerRadius(int radius) {
    corner_radius_ = radius;
    update();
}

void ImageRenderer::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPixmap current_pix = is_animated_ && movie_ ? movie_->currentPixmap() : pixmap_;
    if (current_pix.isNull()) {
        return;
    }

    // Keep aspect ratio centered in widget bounds
    QSize scaled_size = current_pix.size().scaled(size(), Qt::KeepAspectRatio);
    QRect target_rect((width() - scaled_size.width()) / 2,
                      (height() - scaled_size.height()) / 2,
                      scaled_size.width(),
                      scaled_size.height());

    QPainterPath path;
    path.addRoundedRect(target_rect, corner_radius_, corner_radius_);
    painter.setClipPath(path);

    painter.drawPixmap(target_rect, current_pix);

    // Subtle dark border around image
    painter.setClipping(false);
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1.0));
    painter.drawRoundedRect(target_rect, corner_radius_, corner_radius_);
}

} // namespace meridian::gui

