#include "mini_test.hpp"
#include "../src/core/renderer/damage_tracker.hpp"
#include "../src/core/renderer/glyph_atlas.hpp"
#include "../src/core/renderer/render_pipeline.hpp"
#include "../src/core/vt/graphics.hpp"
#include "../src/core/vt/screen_buffer.hpp"

using namespace meridian::renderer;
using namespace meridian::vt;

MTEST(damage_tracker_basic) {
    DamageTracker tracker(24, 80);
    // Initially all dirty after creation/resize
    ASSERT_TRUE(tracker.is_dirty());
    auto full = tracker.consume_damage();
    ASSERT_EQ(full.width, 80);
    ASSERT_EQ(full.height, 24);
    ASSERT_FALSE(tracker.is_dirty());

    // Mark single cell dirty
    tracker.mark_dirty(5, 10, 1);
    ASSERT_TRUE(tracker.is_dirty());
    ASSERT_TRUE(tracker.is_row_dirty(5));
    ASSERT_FALSE(tracker.is_row_dirty(4));

    auto rect = tracker.consume_damage();
    ASSERT_EQ(rect.x, 10);
    ASSERT_EQ(rect.y, 5);
    ASSERT_EQ(rect.width, 1);
    ASSERT_EQ(rect.height, 1);
    ASSERT_FALSE(tracker.is_dirty());
}

MTEST(glyph_atlas_features) {
    GlyphAtlas atlas(1024, 1024);
    ASSERT_EQ(atlas.width(), 1024);
    ASSERT_EQ(atlas.height(), 1024);

    // Box drawing & symbols
    ASSERT_TRUE(GlyphAtlas::is_box_drawing(0x2500)); // ─
    ASSERT_TRUE(GlyphAtlas::is_powerline_symbol(0xE0B0)); // 
    ASSERT_TRUE(GlyphAtlas::is_emoji(0x1F600)); // 😀
    ASSERT_FALSE(GlyphAtlas::is_emoji(U'A'));

    // Ligatures
    std::u32string code = U"const a === b && c !== d;";
    ASSERT_EQ(GlyphAtlas::detect_ligature(code, 8), "===");
    ASSERT_EQ(GlyphAtlas::detect_ligature(code, 19), "!==");

    // Texture coordinate allocation
    const auto& entry1 = atlas.get_or_create(U'A', FontStyle::Regular, 10, 20);
    ASSERT_GT(entry1.u1, entry1.u0);
    ASSERT_GT(entry1.v1, entry1.v0);
    ASSERT_EQ(atlas.cached_glyph_count(), 1u);

    // Reusing same glyph returns cached entry
    const auto& entry2 = atlas.get_or_create(U'A', FontStyle::Regular, 10, 20);
    ASSERT_EQ(entry1.u0, entry2.u0);
    ASSERT_EQ(atlas.cached_glyph_count(), 1u);
}

MTEST(render_pipeline_batching) {
    ScreenBuffer screen(24, 80);
    GlyphAtlas atlas(1024, 1024);
    RenderPipeline pipeline(10, 20);

    // Color resolution
    auto red = pipeline.resolve_color(Color::indexed(1), /*is_fg=*/true);
    ASSERT_EQ(red.r, 220);

    auto rgb = pipeline.resolve_color(Color::rgb(100, 150, 200), /*is_fg=*/false);
    ASSERT_EQ(rgb.r, 100);
    ASSERT_EQ(rgb.g, 150);
    ASSERT_EQ(rgb.b, 200);

    // Put some characters
    Attributes attrs;
    attrs.fg = Color::indexed(2); // Green
    screen.put_codepoint(U'H', attrs);
    screen.put_codepoint(U'i', attrs);

    SelectionRange sel;
    sel.active = true;
    sel.start_row = 0;
    sel.start_col = 0;
    sel.end_row = 0;
    sel.end_col = 1;
    ASSERT_TRUE(sel.contains(0, 0));
    ASSERT_TRUE(sel.contains(0, 1));
    ASSERT_FALSE(sel.contains(0, 2));

    auto batch = pipeline.generate_batch(screen, atlas, nullptr, &sel, CursorShape::Block, true, 1.0f);
    ASSERT_GT(batch.glyph_vertices.size(), 0u);
    ASSERT_GT(batch.background_vertices.size(), 0u);
    ASSERT_GT(batch.cursor_vertices.size(), 0u);
}

MTEST(graphics_sixel_and_kitty) {
    GraphicsEngine engine;

    // Base64 decode
    std::string b64 = "SGVsbG8gV29ybGQ=";
    auto decoded = GraphicsEngine::decode_base64(b64);
    std::string str(decoded.begin(), decoded.end());
    ASSERT_EQ(str, "Hello World");

    // Sixel parsing
    bool sixel_ok = engine.handle_sixel("~-~-~-", 2, 4);
    ASSERT_TRUE(sixel_ok);
    ASSERT_EQ(engine.placements().size(), 1u);
    ASSERT_EQ(engine.placements()[0].row, 2);
    ASSERT_EQ(engine.placements()[0].col, 4);

    // Kitty Graphics parsing
    bool kitty_ok = engine.handle_kitty_graphics("a=T,f=32,s=20,v=40;AAAA");
    ASSERT_TRUE(kitty_ok);
    ASSERT_EQ(engine.placements().size(), 2u);
}

