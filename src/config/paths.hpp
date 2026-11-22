#pragma once
// zephyr-config / paths.hpp
//
// Single place that decides where Zephyr keeps its per-user files, so the
// CLI and the GUI never disagree. Resolution order:
//   1. $ZEPHYR_CONFIG_HOME            (explicit override, used by tests)
//   2. $XDG_CONFIG_HOME/zephyr        (XDG Base Directory spec)
//   3. $HOME/.config/zephyr

#include <string>

namespace zephyr::config {

std::string config_dir();
std::string config_file(const std::string& name); // config_dir() + "/" + name

} // namespace zephyr::config
