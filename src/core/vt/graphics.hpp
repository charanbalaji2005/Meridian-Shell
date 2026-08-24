#pragma once
// meridian-terminal / core / vt / graphics.hpp
//
// High-performance Sixel and Kitty Graphics protocol decoder.
// Enables inline image rendering, image previews, and rich graphical output.

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace meridian::vt {

enum class ImageFormat {
    Rgba32,
    Rgb24,
    Png,
    Sixel
};

struct GraphicImage {
    uint32_t id = 0;
    int width = 0;
    int height = 0;
    ImageFormat format = ImageFormat::Rgba32;
    std::vector<uint8_t> rgba_data; // 32-bit RGBA pixel buffer
};

struct ImagePlacement {
    uint32_t image_id = 0;
    int row = 0;
    int col = 0;
    int cols_spanned = 1;
    int rows_spanned = 1;
    int z_index = 0;
};

class GraphicsEngine {
public:
    GraphicsEngine() = default;

    // Kitty Graphics Protocol APC parser: "\033_Ga=T,f=32,s=100,v=100;...base64...\033\\"
    bool handle_kitty_graphics(std::string_view control_and_payload);

    // Sixel raster graphics decoder: "\033Pq...data...\033\\"
    bool handle_sixel(std::string_view sixel_stream, int cursor_row = 0, int cursor_col = 0);

    const GraphicImage* find_image(uint32_t id) const;
    const std::vector<ImagePlacement>& placements() const { return placements_; }
    void clear_placements() { placements_.clear(); }
    void clear_all();

    static std::vector<uint8_t> decode_base64(std::string_view in);

private:
    uint32_t next_image_id_ = 1;
    std::unordered_map<uint32_t, GraphicImage> images_;
    std::vector<ImagePlacement> placements_;
};

} // namespace meridian::vt

