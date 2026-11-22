// zephyr-config / profiles.cpp
#include "profiles.hpp"
#include "../core/config.hpp"

#include <cstdlib>

namespace zephyr::config {

namespace {

bool has_control_or_space(const std::string& s) {
    for (unsigned char c : s) {
        if (c <= 0x20 || c == 0x7F) return true;
    }
    return false;
}

bool has_newline(const std::string& s) {
    return s.find_first_of("\r\n") != std::string::npos;
}

// Config values are stored one per line; never let a newline in.
std::string one_line(const std::string& s) {
    std::string out;
    for (char c : s) out.push_back(c == '\n' || c == '\r' ? ' ' : c);
    return out;
}

} // namespace

bool valid_env_name(const std::string& name) {
    if (name.empty()) return false;
    auto alpha = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; };
    if (!alpha(name[0])) return false;
    for (char c : name) {
        if (!alpha(c) && !(c >= '0' && c <= '9')) return false;
    }
    return true;
}

bool SshProfile::validate(std::string* error) const {
    auto fail = [&](const std::string& msg) {
        if (error) *error = msg;
        return false;
    };
    if (name.empty()) return fail("Name is required");
    if (host.empty()) return fail("Host is required");
    if (host[0] == '-' || has_control_or_space(host)) return fail("Host must not start with '-' or contain spaces");
    if (!user.empty() && (user[0] == '-' || has_control_or_space(user) || user.find('@') != std::string::npos)) {
        return fail("User must not start with '-' or contain spaces or '@'");
    }
    if (port < 1 || port > 65535) return fail("Port must be between 1 and 65535");
    if (has_newline(identity_file)) return fail("Identity file path is invalid");
    for (const auto& a : extra_args) {
        if (has_newline(a)) return fail("Extra arguments must not contain newlines");
    }
    return true;
}

std::vector<std::string> SshProfile::ssh_args() const {
    std::vector<std::string> out;
    if (port != 22) {
        out.push_back("-p");
        out.push_back(std::to_string(port));
    }
    if (!identity_file.empty()) {
        out.push_back("-i");
        out.push_back(identity_file);
    }
    for (const auto& a : extra_args) out.push_back(a);
    out.push_back("--"); // everything after this is the destination, never an option
    out.push_back(user.empty() ? host : user + "@" + host);
    return out;
}

std::vector<std::string> split_args(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool in_token = false;
    char quote = 0;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (quote) {
            if (c == quote) { quote = 0; continue; }
            if (c == '\\' && quote == '"' && i + 1 < line.size()) { cur.push_back(line[++i]); continue; }
            cur.push_back(c);
            continue;
        }
        if (c == '\'' || c == '"') { quote = c; in_token = true; continue; }
        if (c == '\\' && i + 1 < line.size()) { cur.push_back(line[++i]); in_token = true; continue; }
        if (c == ' ' || c == '\t' || c == '\n') {
            if (in_token) { out.push_back(cur); cur.clear(); in_token = false; }
            continue;
        }
        cur.push_back(c);
        in_token = true;
    }
    if (in_token) out.push_back(cur);
    return out;
}

std::string join_args(const std::vector<std::string>& args) {
    std::string out;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i) out.push_back(' ');
        const auto& a = args[i];
        bool needs_quote = a.empty() || a.find_first_of(" \t\"'\\") != std::string::npos;
        if (!needs_quote) { out += a; continue; }
        out.push_back('"');
        for (char c : a) {
            if (c == '"' || c == '\\') out.push_back('\\');
            out.push_back(c);
        }
        out.push_back('"');
    }
    return out;
}

ProfileStore::ProfileStore(std::string path) : path_(std::move(path)) {
    profiles_.push_back(default_profile());
}

Profile ProfileStore::default_profile() {
    Profile p;
    p.name = "Default";
    const char* shell = std::getenv("SHELL");
    p.program = (shell && shell[0] == '/') ? shell : "/bin/bash";
    return p;
}

void ProfileStore::set_profiles(std::vector<Profile> p) {
    profiles_ = std::move(p);
    if (profiles_.empty()) profiles_.push_back(default_profile());
}

const Profile* ProfileStore::find(const std::string& name) const {
    for (const auto& p : profiles_) if (p.name == name) return &p;
    return nullptr;
}

const SshProfile* ProfileStore::find_ssh(const std::string& name) const {
    for (const auto& p : ssh_profiles_) if (p.name == name) return &p;
    return nullptr;
}

bool ProfileStore::load() {
    Config cfg(path_);
    bool existed = cfg.load();
    if (!existed) return false;

    std::vector<Profile> loaded;
    int count = std::atoi(cfg.get("profile.count", "0").c_str());
    for (int i = 0; i < count && i < 256; ++i) {
        std::string k = "profile." + std::to_string(i) + ".";
        Profile p;
        p.name = cfg.get(k + "name");
        p.program = cfg.get(k + "program");
        if (p.name.empty() || p.program.empty()) continue;
        p.args = split_args(cfg.get(k + "args"));
        p.cwd = cfg.get(k + "cwd");
        int env_count = std::atoi(cfg.get(k + "env.count", "0").c_str());
        for (int e = 0; e < env_count && e < 256; ++e) {
            std::string ek = k + "env." + std::to_string(e) + ".";
            EnvVar v{cfg.get(ek + "name"), cfg.get(ek + "value")};
            if (valid_env_name(v.name)) p.env.push_back(v);
        }
        loaded.push_back(std::move(p));
    }
    set_profiles(std::move(loaded));

    ssh_profiles_.clear();
    int ssh_count = std::atoi(cfg.get("ssh.count", "0").c_str());
    for (int i = 0; i < ssh_count && i < 256; ++i) {
        std::string k = "ssh." + std::to_string(i) + ".";
        SshProfile s;
        s.name = cfg.get(k + "name");
        s.host = cfg.get(k + "host");
        s.user = cfg.get(k + "user");
        s.port = std::atoi(cfg.get(k + "port", "22").c_str());
        s.identity_file = cfg.get(k + "identity_file");
        s.extra_args = split_args(cfg.get(k + "extra_args"));
        if (s.validate()) ssh_profiles_.push_back(std::move(s)); // drop tampered/invalid entries
    }
    return true;
}

bool ProfileStore::save() const {
    Config cfg(path_);
    cfg.set("profile.count", std::to_string(profiles_.size()));
    for (std::size_t i = 0; i < profiles_.size(); ++i) {
        const auto& p = profiles_[i];
        std::string k = "profile." + std::to_string(i) + ".";
        cfg.set(k + "name", one_line(p.name));
        cfg.set(k + "program", one_line(p.program));
        cfg.set(k + "args", one_line(join_args(p.args)));
        cfg.set(k + "cwd", one_line(p.cwd));
        cfg.set(k + "env.count", std::to_string(p.env.size()));
        for (std::size_t e = 0; e < p.env.size(); ++e) {
            std::string ek = k + "env." + std::to_string(e) + ".";
            cfg.set(ek + "name", one_line(p.env[e].name));
            cfg.set(ek + "value", one_line(p.env[e].value));
        }
    }
    cfg.set("ssh.count", std::to_string(ssh_profiles_.size()));
    for (std::size_t i = 0; i < ssh_profiles_.size(); ++i) {
        const auto& s = ssh_profiles_[i];
        std::string k = "ssh." + std::to_string(i) + ".";
        cfg.set(k + "name", one_line(s.name));
        cfg.set(k + "host", one_line(s.host));
        cfg.set(k + "user", one_line(s.user));
        cfg.set(k + "port", std::to_string(s.port));
        cfg.set(k + "identity_file", one_line(s.identity_file));
        cfg.set(k + "extra_args", one_line(join_args(s.extra_args)));
    }
    return cfg.save();
}

} // namespace zephyr::config
