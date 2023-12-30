mod parser;
use markdown::{
    // mdast::{MdxjsEsm, Node, Root},
    to_mdast,
    Constructs,
    ParseOptions,
};
use parser::boundaries::calc_boundaries;
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
    let text = "\n- >> [a€bc\n  def](x.com)\n";
    println!("find: {}", text.find("\n").unwrap());
    let result = parse_mdx(text);

    match result {
        Ok(node) => {
            let nodes = calc_boundaries(&node, text);
            println!("{:?}", nodes);
        }
        Err(error) => {
            println!("Error: {}", error);
        }
    }
}
