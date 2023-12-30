mod parser;
use markdown::{
    // mdast::{MdxjsEsm, Node, Root},
    to_mdast,
    Constructs,
    ParseOptions,
};
use parser::boundaries::{calc_boundaries, print_boundaries};
use parser::to_ast::{parse_mdx, parse_nomdx};

fn print_node(node: &markdown::mdast::Node, level: usize) {
    print!("{: <1$}", "", level);
    println!("{:?}", node);
    match node.children() {
        Some(value) => value.iter().for_each(|child| print_node(child, level + 1)),
        None => {}
    }
    ()
}

fn main() {
    let text = "\n- >> [ae *bc*\n  def](x.com)\n";
    // let text = "- a\n     - b\n  - c\n     - d\n  e \n   - f";
    // let text = " - a\n     - b\n   - c\n     - d";
    println!("find: {}", text.find("\n").unwrap());
    let result = parse_mdx(text);

    match result {
        Ok(node) => {
            let result = calc_boundaries(&node, text);
            let text = text.to_owned();
            print_boundaries(&text, &result);
        }
        Err(error) => {
            println!("Error: {}", error);
        }
    }
}
