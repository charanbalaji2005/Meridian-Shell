#pragma once
// src/gui/Motion.hpp
//
// Small, consistent transitions. Each helper:
//   * respects AppContext::animationsEnabled() (reduced motion => instant),
//   * attaches a QGraphicsOpacityEffect only for the animation's duration
//     and removes it afterwards, because a permanent effect forces
//     offscreen rendering of the widget on every paint.

#include <functional>

class QWidget;

namespace zephyr::gui::motion {

constexpr int kFast = 120;    // hovers, small reveals
constexpr int kNormal = 180;  // panels, popups
constexpr int kSlow = 240;    // page transitions

void fadeIn(QWidget* w, int ms = kNormal);
void fadeOut(QWidget* w, int ms = kNormal, std::function<void()> done = {});
// Fade + short vertical slide (popups, the search bar, the command palette).
void popIn(QWidget* w, int dy = -8, int ms = kNormal);
// Animates maximumWidth between the current value and `to` (sidebar).
void animateWidth(QWidget* w, int to, int ms = kNormal, std::function<void()> done = {});

} // namespace zephyr::gui::motion
