use markdown::{
  // mdast::{MdxjsEsm, Node, Root},
  mdast::Node,
  to_mdast,
  Constructs,  ParseOptions,
};
use crate::parser::ast::NodeExt;

use super::ast::RootExt;

pub fn parse_mdx(value: &str, ) -> Result<NodeExt, String> {
    let opts_with_mdx = ParseOptions{
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
        ..Constructs::mdx()
      },
      gfm_strikethrough_single_tilde: true,
      math_text_single_dollar: true,
      mdx_expression_parse: None,
      mdx_esm_parse: None,
    };

    let result = to_mdast(value, &opts_with_mdx);
    if result.is_ok() {
      let ast = result.unwrap();
      Ok(convert_to_extended_ast(&ast))
    }
    else {
      Err(result.err().unwrap())
    }
}
pub fn parse_nomdx(value: &str, ) -> Result<Node, String> {
    let opts_without_mdx = ParseOptions{
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
        ..Constructs::default()
      },
      gfm_strikethrough_single_tilde: true,
      math_text_single_dollar: true,
      mdx_expression_parse: None,
      mdx_esm_parse: None,
    };

    to_mdast(value, &opts_without_mdx)
}

fn convert_to_extended_ast(node: &Node) -> NodeExt {
  match node {
    Node::Root(n) => {
      let root_node = RootExt{data: n, boundaries: None};
      println!("Root node: {:?}", root_node);
      NodeExt::Root(root_node)
    },
    _ => unreachable!("Root node should be the first node in the AST")
  }
  // let root_node = RootExt{data: , boundaries: None};
  // root_node
}