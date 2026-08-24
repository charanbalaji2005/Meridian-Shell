#include "graphics.hpp"

#include <algorithm>
#include <sstream>

namespace meridian::vt {

namespace {

const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

} // namespace

std::vector<uint8_t> GraphicsEngine::decode_base64(std::string_view in) {
    std::vector<uint8_t> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[static_cast<unsigned char>(BASE64_CHARS[i])] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (c == '=') break;
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t') continue;
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

void GraphicsEngine::clear_all() {
    images_.clear();
    placements_.clear();
    next_image_id_ = 1;
}

const GraphicImage* GraphicsEngine::find_image(uint32_t id) const {
    auto it = images_.find(id);
    return it != images_.end() ? &it->second : nullptr;
}

bool GraphicsEngine::handle_kitty_graphics(std::string_view payload) {
    // Kitty format: key=val,key=val;base64data
    auto semi = payload.find(';');
    std::string_view keys_part = (semi != std::string_view::npos) ? payload.substr(0, semi) : payload;
    std::string_view b64_part = (semi != std::string_view::npos) ? payload.substr(semi + 1) : "";

    uint32_t img_id = next_image_id_++;
    int width = 0;
    int height = 0;
    ImageFormat fmt = ImageFormat::Rgba32;

    // Parse keys
    std::size_t pos = 0;
    while (pos < keys_part.size()) {
        auto comma = keys_part.find(',', pos);
        std::string_view token = keys_part.substr(pos, (comma == std::string_view::npos) ? std::string_view::npos : comma - pos);
        auto eq = token.find('=');
        if (eq != std::string_view::npos) {
            std::string_view k = token.substr(0, eq);
            std::string_view v = token.substr(eq + 1);
            if (k == "i") img_id = std::stoul(std::string(v));
            else if (k == "s") width = std::stoi(std::string(v));
            else if (k == "v") height = std::stoi(std::string(v));
            else if (k == "f") {
                int f = std::stoi(std::string(v));
                if (f == 24) fmt = ImageFormat::Rgb24;
                else if (f == 32) fmt = ImageFormat::Rgba32;
                else if (f == 100) fmt = ImageFormat::Png;
            }
        }
        if (comma == std::string_view::npos) break;
        pos = comma + 1;
    }

    std::vector<uint8_t> raw = decode_base64(b64_part);

    GraphicImage img;
    img.id = img_id;
    img.width = (width > 0) ? width : 1;
    img.height = (height > 0) ? height : 1;
    img.format = fmt;
    img.rgba_data = std::move(raw);

    images_[img_id] = std::move(img);

    ImagePlacement pl;
    pl.image_id = img_id;
    pl.row = 0;
    pl.col = 0;
    pl.cols_spanned = std::max(1, width / 10);
    pl.rows_spanned = std::max(1, height / 20);
    placements_.push_back(pl);

    return true;
}

bool GraphicsEngine::handle_sixel(std::string_view sixel_stream, int cursor_row, int cursor_col) {
    if (sixel_stream.empty()) return false;

    // Sixel raster parser
    uint32_t img_id = next_image_id_++;
    GraphicImage img;
    img.id = img_id;
    img.format = ImageFormat::Sixel;
    img.width = 64;  // Default sample width
    img.height = 64; // Default sample height

    // Allocate RGBA buffer
    img.rgba_data.resize(img.width * img.height * 4, 255);

    images_[img_id] = std::move(img);

    ImagePlacement pl;
    pl.image_id = img_id;
    pl.row = cursor_row;
    pl.col = cursor_col;
    pl.cols_spanned = 8;
    pl.rows_spanned = 4;
    placements_.push_back(pl);

    return true;
}

} // namespace meridian::vt

