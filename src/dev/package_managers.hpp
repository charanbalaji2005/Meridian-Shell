#pragma once
// zephyr-dev / package_managers.hpp
//
// Knows the command shapes of common Linux package managers so the GUI
// can *type* (never run) e.g. "sudo dnf install <pkg>" at the prompt.
// Package names are validated so a crafted name can't smuggle in shell
// syntax should the user press Enter.

#include <map>
#include <string>
#include <vector>

namespace zephyr::dev {

struct PackageManager {
    std::string id;          // dnf | apt | pacman | zypper | apk | nix
    std::string display;     // "DNF (Fedora)"
    std::string executable;  // binary to look for on PATH
    std::string search;      // templates; "{pkg}" is replaced by the package name
    std::string install;
    std::string remove;
    std::string upgrade;     // no {pkg}
};

const std::vector<PackageManager>& known_package_managers();
const PackageManager* find_package_manager(const std::string& id);

// Parses /etc/os-release style KEY=value lines (quotes stripped).
std::map<std::string, std::string> parse_os_release(const std::string& content);

// Package manager ids in preference order for a distro (from ID/ID_LIKE).
std::vector<std::string> package_managers_for_distro(const std::map<std::string, std::string>& os_release);

// [A-Za-z0-9][A-Za-z0-9._+@:-]* , at most 128 chars.
bool valid_package_name(const std::string& name);

// Fills "{pkg}" in a template. Returns "" if the name is invalid.
std::string package_command(const std::string& tmpl, const std::string& pkg);

} // namespace zephyr::dev
