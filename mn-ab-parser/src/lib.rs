pub mod definitions;
mod internal;
mod blocks;
mod helpers;
mod spans;
mod commons;


pub use definitions::{
    Offset,
    Attributes,
    BlockType,
    BlockDetail,
    Boundaries,
    SpanDetail,
    SpanType,
    TextType,
};

use internal::{Context, ContainerId};

pub trait Parser {
    fn enter_block(
        &mut self,
        b_type: BlockType,
        bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &BlockDetail,
    ) -> Result<(), String>;

    fn leave_block(&mut self, b_type: BlockType) -> Result<(), String>;

    fn enter_span(
        &mut self,
        s_type: SpanType,
        bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &SpanDetail,
    ) -> Result<(), String>;

    fn leave_span(&mut self, s_type: SpanType) -> Result<(), String>;

    fn text(&mut self, t_type: TextType, bounds: &Vec<Boundaries>) -> Result<(), String>;
}

pub fn parser<P: Parser>(text: &str, parser: &mut P) {
    let mut ctx = Context::new(text);

    ctx.line_number_begin.push(0);
    for (i, c) in text.chars().enumerate() {
        if c == '\n' {
            ctx.line_number_begin.push(i + 1);
        }
    }

    if let Err(e) = blocks::parse_blocks(&mut ctx, parser) {
        eprintln!("Parsing error: {}", e);
    }
}