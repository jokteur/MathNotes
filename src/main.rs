mod parser;
use markdown::{
    // mdast::{MdxjsEsm, Node, Root},
     to_mdast,
    Constructs,  ParseOptions,
};
use parser::to_ast::{parse_mdx, parse_nomdx};

fn print_node(node: &markdown::mdast::Node, level: usize) {
    print!("{: <1$}", "", level);
    println!("{:?}", node);
    match node.children() {
        Some(value) => {
            value.iter().for_each(|child| print_node(child,level+1))
        },
        None => {},
    }()
}

fn main() {
    let result = parse_mdx("<fig>Hello {ref}</fig>");

   match result {
       Ok(node) => {
        //    print_node(&node, 0);
       },
       Err(error) => {
           println!("Error: {}", error);
       }
    }
}
