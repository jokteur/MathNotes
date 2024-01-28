use egui::widgets::Image;
use rex::{
    cairo::CairoBackend,
    font::{backend::ttf_parser::TtfMathFont, FontContext},
    layout::{LayoutSettings, Style},
    render::Renderer,
};
use sha2::{Digest, Sha256};

#[derive(Clone, Debug)]
pub struct LatexImage<'a> {
    pub image: Image<'a>,
    pub width: f32,
    pub height: f32,
    pub ascent: f32,
}

pub enum LatexDefaultColor {
    Black,
    White,
    Gray,
    DarkGray,
    LightGray,
}

impl LatexImage<'_> {
    pub fn new<'a>(
        text: &str,
        font: TtfMathFont,
        font_size: f32,
        dpi_factor: f32,
        default_color: LatexDefaultColor,
    ) -> Self {
        let text = match default_color {
            LatexDefaultColor::Black => format!("\\color{{black}}{{{}}}", text),
            LatexDefaultColor::White => format!("\\color{{white}}{{{}}}", text),
            LatexDefaultColor::Gray => format!("\\color{{gray}}{{{}}}", text),
            LatexDefaultColor::DarkGray => format!("\\color{{darkgray}}{{{}}}", text),
            LatexDefaultColor::LightGray => format!("\\color{{lightgray}}{{{}}}", text),
        };

        // Generate hash from text
        let mut hasher = Sha256::new();
        hasher.update(text.as_bytes());
        let hash = hasher.finalize();
        let hash = format!("{:x}", hash);
        let path: String = format!("{}.svg", hash);

        // out stream
        let mut stream = Vec::new();

        let ctx = FontContext::new(&font).unwrap();

        // TODO[jokteur]: Allow user to change from inline to display
        let layout_settings =
            LayoutSettings::new(&ctx, (font_size * dpi_factor).into(), Style::Display);
        let parse_nodes = rex::parser::parse(&text).unwrap();
        let layout = rex::layout::engine::layout(&parse_nodes, layout_settings).unwrap();
        let dims = layout.size();

        let height = (dims.height + dims.depth.abs()) as f64;

        // Render to SVG
        unsafe {
            let svg_surface =
                cairo::SvgSurface::for_raw_stream(dims.width, height, &mut stream).unwrap();
            let context = cairo::Context::new(&svg_surface).unwrap();

            // So that top-left corner of SVG is aligned with top of formula
            context.translate(0., dims.height);

            // Render to Cairo backend
            let mut backend = CairoBackend::new(context);
            let renderer = Renderer::new();
            renderer.render(&layout, &mut backend);
            svg_surface.finish();
        }

        let image = egui::Image::from_bytes(path, stream);
        LatexImage {
            image,
            width: dims.width as f32,
            height: height as f32,
            ascent: layout.height.to_unitless() as f32,
        }
    }

    pub fn draw_at(
        &mut self,
        ui: &mut egui::Ui,
        ctx: &egui::Context,
        pos: egui::Pos2,
        tint: egui::Color32,
    ) {
        let rect = egui::Rect::from_min_size(pos, egui::Vec2::new(self.width, self.height));

        // Intentionnally load image at 2x size to a bit of oversampling
        let image_texture = self
            .image
            .load_for_size(ctx, egui::Vec2::new(2.0 * self.width, 2.0 * self.height));

        if image_texture.is_ok() {
            let image_options = self.image.image_options();
            match image_texture.unwrap().texture_id() {
                Some(texture_id) => {
                    ui.painter().image(texture_id, rect, image_options.uv, tint);
                }
                _ => {}
            }
        };
    }
}

pub fn load_font<'a>(file: &'a [u8]) -> TtfMathFont<'a> {
    let font = ttf_parser::Face::parse(file, 0).unwrap();
    TtfMathFont::new(font).unwrap()
}
