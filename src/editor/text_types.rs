use egui::emath::*;
use egui::TextureId;

#[derive(Copy, Clone, Debug, PartialEq)]
#[cfg_attr(feature = "serde", derive(serde::Deserialize, serde::Serialize))]
pub struct ImageGlyph {
    texture_id: TextureId,
    /// Baseline position, relative to the galley.
    /// Logical position: pos.y is the same for all chars of the same [`TextFormat`].
    pub pos: Pos2,

    /// `ascent` value
    pub ascent: f32,
    /// `descent` value
    pub descent: f32,

    /// Advance width and line height.
    ///
    /// Does not control the visual size of the glyph (see [`Self::uv_rect`] for that).
    pub size: Vec2,

    /// Position and size of the glyph in the font texture, in texels.
    // pub uv_rect: UvRect,

    /// Index into [`LayoutJob::sections`]. Decides color etc.
    pub section_index: u32,
}
