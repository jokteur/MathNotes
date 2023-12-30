use markdown::{
    // mdast::{MdxjsEsm, Node, Root},
    mdast::Node,
    to_mdast,
    Constructs,
    ParseOptions,
};

pub fn parse_mdx(value: &str) -> Result<Node, String> {
    let opts_with_mdx = ParseOptions {
        constructs: Constructs {
            math_flow: true,
            math_text: true,
            character_reference: false,
            gfm_autolink_literal: true,
            gfm_footnote_definition: true,
            gfm_label_start_footnote: true,
            gfm_strikethrough: true,
            gfm_table: true,
            gfm_task_list_item: true,
            heading_setext: false,
            ..Constructs::mdx()
        },
        gfm_strikethrough_single_tilde: true,
        math_text_single_dollar: true,
        mdx_expression_parse: None,
        mdx_esm_parse: None,
    };

    to_mdast(value, &opts_with_mdx)
}
pub fn parse_nomdx(value: &str) -> Result<Node, String> {
    let opts_without_mdx = ParseOptions {
        constructs: Constructs {
            math_flow: true,
            math_text: true,
            character_reference: false,
            gfm_autolink_literal: true,
            gfm_footnote_definition: true,
            gfm_label_start_footnote: true,
            gfm_strikethrough: true,
            gfm_table: true,
            gfm_task_list_item: true,
            heading_setext: false,
            ..Constructs::default()
        },
        gfm_strikethrough_single_tilde: true,
        math_text_single_dollar: true,
        mdx_expression_parse: None,
        mdx_esm_parse: None,
    };

    to_mdast(value, &opts_without_mdx)
}
