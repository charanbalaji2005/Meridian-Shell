// zephyr-dev / package_managers.cpp
#include "package_managers.hpp"

#include <sstream>

namespace zephyr::dev {

const std::vector<PackageManager>& known_package_managers() {
    static const std::vector<PackageManager> pms = {
        {"dnf", "DNF (Fedora, RHEL)", "dnf", "dnf search {pkg}", "sudo dnf install {pkg}",
         "sudo dnf remove {pkg}", "sudo dnf upgrade --refresh"},
        {"apt", "APT (Debian, Ubuntu)", "apt", "apt search {pkg}", "sudo apt install {pkg}",
         "sudo apt remove {pkg}", "sudo apt update && sudo apt upgrade"},
        {"pacman", "pacman (Arch)", "pacman", "pacman -Ss {pkg}", "sudo pacman -S {pkg}",
         "sudo pacman -Rs {pkg}", "sudo pacman -Syu"},
        {"zypper", "zypper (openSUSE)", "zypper", "zypper search {pkg}", "sudo zypper install {pkg}",
         "sudo zypper remove {pkg}", "sudo zypper update"},
        {"apk", "apk (Alpine)", "apk", "apk search {pkg}", "sudo apk add {pkg}", "sudo apk del {pkg}",
         "sudo apk upgrade"},
        {"nix", "Nix profile", "nix", "nix search nixpkgs {pkg}", "nix profile install nixpkgs#{pkg}",
         "nix profile remove {pkg}", "nix profile upgrade --all"},
    };
    return pms;
}

const PackageManager* find_package_manager(const std::string& id) {
    for (const auto& pm : known_package_managers()) {
        if (pm.id == id) return &pm;
    }
    return nullptr;
}

std::map<std::string, std::string> parse_os_release(const std::string& content) {
    std::map<std::string, std::string> out;
    std::istringstream in(content);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (!value.empty() && value.back() == '\r') value.pop_back();
        if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front()) {
            value = value.substr(1, value.size() - 2);
        }
        out[key] = value;
    }
    return out;
}

std::vector<std::string> package_managers_for_distro(const std::map<std::string, std::string>& osr) {
    std::string ids;
    if (auto it = osr.find("ID"); it != osr.end()) ids += " " + it->second;
    if (auto it = osr.find("ID_LIKE"); it != osr.end()) ids += " " + it->second;
    ids += " ";
    auto has = [&](const char* id) { return ids.find(std::string(" ") + id + " ") != std::string::npos; };

    std::vector<std::string> out;
    if (has("nixos")) out.push_back("nix");
    if (has("fedora") || has("rhel") || has("centos")) out.push_back("dnf");
    if (has("debian") || has("ubuntu")) out.push_back("apt");
    if (has("arch")) out.push_back("pacman");
    if (has("suse") || has("opensuse")) out.push_back("zypper");
    if (has("alpine")) out.push_back("apk");
    return out;
}

bool valid_package_name(const std::string& name) {
    if (name.empty() || name.size() > 128) return false;
    auto alnum = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); };
    if (!alnum(name[0])) return false;
    for (char c : name) {
        if (!alnum(c) && c != '.' && c != '_' && c != '+' && c != '-' && c != '@' && c != ':') return false;
    }
    return true;
}

std::string package_command(const std::string& tmpl, const std::string& pkg) {
    if (!valid_package_name(pkg)) return "";
    std::string out = tmpl;
    const std::string marker = "{pkg}";
    for (auto pos = out.find(marker); pos != std::string::npos; pos = out.find(marker, pos + pkg.size())) {
        out.replace(pos, marker.size(), pkg);
    }
    return out;
}

} // namespace zephyr::dev
