// zephyr-config / paths.cpp
#include "paths.hpp"

#include <cstdlib>

namespace zephyr::config {

std::string config_dir() {
    if (const char* o = std::getenv("ZEPHYR_CONFIG_HOME"); o && *o) return o;
    if (const char* x = std::getenv("XDG_CONFIG_HOME"); x && *x == '/') return std::string(x) + "/zephyr";
    const char* home = std::getenv("HOME");
    return std::string(home && *home ? home : ".") + "/.config/zephyr";
}

std::string config_file(const std::string& name) {
    return config_dir() + "/" + name;
}

} // namespace zephyr::config
