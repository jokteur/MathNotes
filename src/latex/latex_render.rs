use rex::{
    cairo::CairoBackend,
    font::{backend::ttf_parser::TtfMathFont, FontContext},
    layout::{LayoutSettings, Style},
    render::Renderer,
};
use ttf_parser::svg;

pub fn load_font<'a>(file: &'a [u8]) -> TtfMathFont<'a> {
    let font = ttf_parser::Face::parse(file, 0).unwrap();
    TtfMathFont::new(font).unwrap()
}

pub fn render_image<'a>(text: &str, font_size: f64, dpi_factor: f64, font: TtfMathFont) {
    let ctx = FontContext::new(&font).unwrap();
    // 12pt = 16px
    let layout_settings = LayoutSettings::new(&ctx, font_size, Style::Display);

    // -- parse
    let parse_nodes = rex::parser::parse(&text).unwrap();

    // -- layout
    let layout = rex::layout::engine::layout(&parse_nodes, layout_settings).unwrap();

    // -- create Cairo surface & context
    let dims = layout.size();
    let svg_surface =
        cairo::SvgSurface::new(dims.width, dims.height - dims.depth, Some("test.svg")).unwrap();
    let context = cairo::Context::new(&svg_surface).unwrap();
    // So that top-left corner of SVG is aligned with top of formula
    context.translate(0., dims.height);

    // -- Render to Cairo backend
    let mut backend = CairoBackend::new(context);
    let mut renderer = Renderer::new();
    // renderer.debug = debug;
    renderer.render(&layout, &mut backend);
    // svg_surface.write_to_png(stream).unwrap();
}
