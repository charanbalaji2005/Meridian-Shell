#include "Motion.hpp"

#include "AppContext.hpp"

#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>
#include <QWidget>

namespace zephyr::gui::motion {

namespace {

int duration(int ms) { return AppContext::instance().motionMs(ms); }

QPropertyAnimation* opacityAnim(QWidget* w, qreal from, qreal to, int ms) {
    auto* effect = new QGraphicsOpacityEffect(w);
    effect->setOpacity(from);
    w->setGraphicsEffect(effect); // takes ownership; replaces any previous effect
    auto* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(ms);
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    return anim;
}

} // namespace

void fadeIn(QWidget* w, int ms) {
    w->show();
    const int d = duration(ms);
    if (d == 0) return;
    QPointer<QWidget> guard(w);
    auto* anim = opacityAnim(w, 0.0, 1.0, d);
    QObject::connect(anim, &QPropertyAnimation::finished, w, [guard]() {
        if (guard) guard->setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeOut(QWidget* w, int ms, std::function<void()> done) {
    const int d = duration(ms);
    if (d == 0 || !w->isVisible()) {
        w->hide();
        if (done) done();
        return;
    }
    QPointer<QWidget> guard(w);
    auto* anim = opacityAnim(w, 1.0, 0.0, d);
    QObject::connect(anim, &QPropertyAnimation::finished, w, [guard, done]() {
        if (guard) {
            guard->hide();
            guard->setGraphicsEffect(nullptr);
        }
        if (done) done();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void popIn(QWidget* w, int dy, int ms) {
    w->show();
    w->raise();
    const int d = duration(ms);
    if (d == 0) return;
    QPointer<QWidget> guard(w);
    const QPoint end = w->pos();
    auto* group = new QParallelAnimationGroup(w);
    auto* fade = opacityAnim(w, 0.0, 1.0, d);
    auto* slide = new QPropertyAnimation(w, "pos");
    slide->setDuration(d);
    slide->setStartValue(end + QPoint(0, dy));
    slide->setEndValue(end);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(fade);
    group->addAnimation(slide);
    QObject::connect(group, &QParallelAnimationGroup::finished, w, [guard]() {
        if (guard) guard->setGraphicsEffect(nullptr);
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void animateWidth(QWidget* w, int to, int ms, std::function<void()> done) {
    const int d = duration(ms);
    if (d == 0) {
        w->setMaximumWidth(to);
        if (done) done();
        return;
    }
    auto* anim = new QPropertyAnimation(w, "maximumWidth");
    anim->setDuration(d);
    anim->setStartValue(w->isVisible() ? w->width() : 0);
    anim->setEndValue(to);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    if (done) QObject::connect(anim, &QPropertyAnimation::finished, w, done);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace zephyr::gui::motion
