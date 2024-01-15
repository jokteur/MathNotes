use egui::widgets::Image;
use rex::{
    cairo::CairoBackend,
    font::{backend::ttf_parser::TtfMathFont, FontContext},
    layout::{LayoutSettings, Style},
    render::Renderer,
};

#[derive(Clone, Debug)]
pub struct LatexImage<'a> {
    pub image: Image<'a>,
    pub width: f64,
    pub height: f64,
    pub ascent: f64,
}

pub fn load_font<'a>(file: &'a [u8]) -> TtfMathFont<'a> {
    let font = ttf_parser::Face::parse(file, 0).unwrap();
    TtfMathFont::new(font).unwrap()
}

pub fn render_image<'a>(
    text: &str,
    font: TtfMathFont,
    font_size: f64,
    dpi_factor: f64,
) -> LatexImage<'a> {
    let ctx = FontContext::new(&font).unwrap();

    // TODO[jokteur]: Allow user to change from inline to display
    let layout_settings = LayoutSettings::new(&ctx, font_size * dpi_factor, Style::Display);
    let parse_nodes = rex::parser::parse(&text).unwrap();
    let layout = rex::layout::engine::layout(&parse_nodes, layout_settings).unwrap();

    // TODO[jokteur]: Render directly to egui::Image
    // Currently, we are saving the SVG to a file, then loading it as an egui::Image
    // Temporary quick fix until I implent rendering directly
    let dims = layout.size();
    let svg_surface =
        cairo::SvgSurface::new(dims.width, dims.height - dims.depth, Some("out.svg")).unwrap();
    let context = cairo::Context::new(&svg_surface).unwrap();
    // So that top-left corner of SVG is aligned with top of formula
    context.translate(0., dims.height);

    // -- Render to Cairo backend
    let mut backend = CairoBackend::new(context);
    let mut renderer = Renderer::new();
    // renderer.debug = debug;
    renderer.render(&layout, &mut backend);
    let image = egui::Image::new("out.svg");
    LatexImage {
        image,
        width: dims.width,
        height: dims.height + dims.depth,
        ascent: layout.height.to_unitless(),
    }
}
