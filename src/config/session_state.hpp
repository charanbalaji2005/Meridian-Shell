#pragma once
// zephyr-config / session_state.hpp
//
// What the GUI needs to restore a window after restart: the tabs, the
// split layout inside each tab, and which profile + working directory each
// pane was running. Only layout is restored — never process state or
// commands, so restoring a session can not re-run anything.
//
// A pane tree is serialized to one line, e.g.
//   S1[50,50](P(Default|/home/me),P(Default|/tmp))
// where S1 = side-by-side split, S0 = stacked split, P(profile|cwd) a leaf.
// Text fields are percent-encoded so they can contain any character.

#include <string>
#include <vector>

namespace zephyr::config {

struct PaneLayout {
    bool is_split = false;
    bool side_by_side = false;           // split only: children laid out left→right
    std::vector<int> sizes;              // split only: relative sizes
    std::vector<PaneLayout> children;    // split only
    std::string profile;                 // leaf only
    std::string cwd;                     // leaf only

    static PaneLayout leaf(std::string profile, std::string cwd) {
        PaneLayout p;
        p.profile = std::move(profile);
        p.cwd = std::move(cwd);
        return p;
    }
    int leaf_count() const;
};

struct TabLayout {
    std::string title;
    PaneLayout root;
};

struct SessionState {
    std::vector<TabLayout> tabs;
    int active_tab = 0;
};

std::string serialize_layout(const PaneLayout& layout);
// Returns false on malformed input (out is then unspecified).
bool parse_layout(const std::string& text, PaneLayout& out);

class SessionStore {
public:
    explicit SessionStore(std::string path) : path_(std::move(path)) {}
    bool load(SessionState& out) const;
    bool save(const SessionState& state) const;
    bool clear() const;
private:
    std::string path_;
};

} // namespace zephyr::config
