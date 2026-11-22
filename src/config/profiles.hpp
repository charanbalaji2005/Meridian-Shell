#pragma once
// zephyr-config / profiles.hpp
//
// Terminal profiles (which program a new tab runs, with what arguments,
// working directory and extra environment) and SSH connection profiles.
// Persisted in the same flat key=value format as the rest of Zephyr's
// config (see core/config.hpp).
//
// Security notes:
//  * Nothing here ever goes through a shell: a profile becomes an argv
//    vector handed to PtyManager::spawn (fork + execve).
//  * SSH host/user fields are validated so a value such as
//    "-oProxyCommand=..." can never be interpreted as an ssh option.

#include <string>
#include <vector>

namespace zephyr::config {

struct EnvVar {
    std::string name;
    std::string value;
    bool operator==(const EnvVar& o) const { return name == o.name && value == o.value; }
};

struct Profile {
    std::string name;                 // unique, shown in the UI
    std::string program;              // absolute path or bare name resolved via PATH
    std::vector<std::string> args;
    std::string cwd;                  // empty = inherit / home
    std::vector<EnvVar> env;          // added on top of the inherited environment
};

struct SshProfile {
    std::string name;
    std::string host;
    std::string user;                 // optional
    int port = 22;
    std::string identity_file;        // optional
    std::vector<std::string> extra_args; // advanced, passed before "--"

    // Returns false and fills *error if a field is unsafe or malformed.
    bool validate(std::string* error = nullptr) const;
    // Arguments for /usr/bin/ssh (program itself excluded). Only call on a
    // profile that validate()s.
    std::vector<std::string> ssh_args() const;
};

// Shell-like argument splitting/joining for the one-line "arguments" field
// in the settings UI: whitespace separates, '...' and "..." quote, and a
// backslash escapes the next character. No expansion of any kind.
std::vector<std::string> split_args(const std::string& line);
std::string join_args(const std::vector<std::string>& args);

// POSIX-portable environment variable name: [A-Za-z_][A-Za-z0-9_]*
bool valid_env_name(const std::string& name);

class ProfileStore {
public:
    explicit ProfileStore(std::string path);

    bool load();
    bool save() const;

    // Always contains at least one profile ("Default", running the user's
    // login shell) after load().
    const std::vector<Profile>& profiles() const { return profiles_; }
    const std::vector<SshProfile>& ssh_profiles() const { return ssh_profiles_; }
    void set_profiles(std::vector<Profile> p);
    void set_ssh_profiles(std::vector<SshProfile> p) { ssh_profiles_ = std::move(p); }

    const Profile* find(const std::string& name) const;
    const SshProfile* find_ssh(const std::string& name) const;

    // Profile running $SHELL (or /bin/bash if unset/invalid).
    static Profile default_profile();

    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::vector<Profile> profiles_;
    std::vector<SshProfile> ssh_profiles_;
};

} // namespace zephyr::config
