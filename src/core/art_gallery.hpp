#pragma once
// src/core/art_gallery.hpp
//
// Curated high-resolution anime, cyberpunk, synthwave & space artwork gallery
// for Meridian Terminal. Rotates to a new crystal-clear theme every time the terminal launches.

#include "terminal_image.hpp"
#include <string>
#include <vector>

namespace meridian::core {

struct ArtworkTheme {
    std::string id;
    std::string title;
    TerminalImage image;
};

class ArtGallery {
public:
    // Returns a fresh rotated/random artwork theme on terminal launch
    static ArtworkTheme get_next_artwork(int width = 56, int height = 22);

    // Returns a specific artwork by index
    static ArtworkTheme get_artwork_by_index(size_t index, int width = 56, int height = 22);

    // Total count of curated themes
    static size_t theme_count();

    // Renders the artwork as clean, high-definition TrueColor halfblock lines (28 cols x 11 rows)
    static std::vector<std::string> render_artwork_lines(const TerminalImage& img, int target_cols = 28, int target_rows = 11);
};

} // namespace meridian::core
