#pragma once
// src/core/graphics/graphics_protocol_parser.hpp
//
// Protocol-independent graphics stream parser for Meridian Terminal.
// Supports Kitty Graphics Protocol, iTerm2 OSC 1337, and Sixel formats.

#include "image_object.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace meridian::graphics {

enum class GraphicsProtocolType {
    None,
    Kitty,
    ITerm2,
    Sixel
};

struct ParsedGraphicsCommand {
    GraphicsProtocolType protocol = GraphicsProtocolType::None;
    std::string action;          // "transmit", "display", "delete", "query"
    int format = 32;             // 32 = RGBA, 24 = RGB, 100 = PNG/JPEG
    int width = 0;
    int height = 0;
    int cols = 0;
    int rows = 0;
    int x_offset = 0;
    int y_offset = 0;
    int z_index = 0;
    uint32_t image_id = 0;
    uint32_t placement_id = 0;
    bool more_chunks = false;
    bool is_inline = true;
    bool preserve_aspect = true;
    std::string filename;
    std::vector<uint8_t> payload; // Raw binary payload or decoded RGBA8
};

class GraphicsProtocolParser {
public:
    GraphicsProtocolParser();

    // Identifies protocol type from leading escape bytes
    static GraphicsProtocolType detect_protocol(const std::string& escape_sequence);

    // Parses a Kitty graphics escape sequence (\033_G... \033\)
    bool parse_kitty_sequence(const std::string& seq, ParsedGraphicsCommand& out_cmd);

    // Parses an iTerm2 inline image sequence (\033]1337;File=... \007)
    bool parse_iterm2_sequence(const std::string& seq, ParsedGraphicsCommand& out_cmd);

    // Parses a Sixel escape sequence (\033Pq... \033\)
    bool parse_sixel_sequence(const std::string& seq, ParsedGraphicsCommand& out_cmd);

    // Base64 helper
    static bool decode_base64(const std::string& input, std::vector<uint8_t>& output);
    static std::string encode_base64(const uint8_t* data, size_t length);
};

} // namespace meridian::graphics

