use crate::parser::ast::node_name;
use markdown::mdast::Node;
use std::{
    collections::{BTreeMap, BTreeSet},
    str::Bytes,
};

#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "root")
)]
pub struct Boundary {
    pub pre: usize,
    pub start: usize,
    pub end: usize,
    pub post: usize,
}
pub type Boundaries = Vec<Boundary>;

#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "root")
)]
pub struct ExtendedNode<'a> {
    node: &'a Node,
    children: Vec<ExtendedNode<'a>>,
    boundaries: Boundaries,
}

struct Context<'a> {
    text: &'a str,
    // map of start -> end line
    lines: BTreeMap<usize, usize>,
    used_offsets: BTreeSet<usize>,
}

pub fn calc_boundaries<'a>(node: &'a Node, text: &'a str) -> ExtendedNode<'a> {
    let mut root_node = ExtendedNode {
        node,
        children: vec![],
        boundaries: vec![],
    };

    if node.position().is_none() {
        return root_node;
    }
    let start = node.position().unwrap().start.offset;
    let end = node.position().unwrap().end.offset;

    let mut context = Context {
        text,
        lines: find_lines_start_end(text.as_bytes(), start, end),
        used_offsets: BTreeSet::new(),
    };

    calc_boundaries_inner(node, &mut root_node, &mut context);

    root_node
}

fn calc_boundaries_inner<'a>(
    node: &'a Node,
    node_ext: &mut ExtendedNode<'a>,
    context: &mut Context<'a>,
) {
    if node.position().is_none() {
        return;
    }
    // TODO: What about empty lines?

    let curr_position = node.position().unwrap();
    let node_lines = context
        .lines
        .range(curr_position.start.offset..curr_position.end.offset);

    for (i, (line_start, line_end)) in node_lines.enumerate() {
        let mut pre = *line_start;
        let mut start = *line_start;
        let mut end = *line_end;
        let mut post = *line_end;

        let mut line_range = context.used_offsets.range(pre..post);
        // Update pre and start for used offsets
        if line_range.to_owned().count() > 0 {
            let first = *line_range.next().unwrap();
            let last = *line_range.last().unwrap();
            if first > pre {
                pre = first + 1;
                start = first + 1;
            }
            if last < post {
                end = last;
                post = last;
            }
        }

        if node.children().is_some() && i == 0 {
            let first_child = node.children().unwrap().first().unwrap();
            let first_child_position = first_child.position().unwrap();
            let first_child_start = first_child_position.start.offset;
            start = first_child_start;
        } else {
            // We now go char by char to find the start and end of the delimiters of the node
            match node {
                Node::ListItem(_) => {
                    let mut counter = 0;
                    for (idx, byte) in context.text[pre..post].bytes().enumerate() {
                        match byte {
                            b' ' => counter += 1,
                            b'\t' => break,
                            b'-' => {}
                            _ => {}
                        }
                    }
                }
                Node::BlockQuote(_) => {}
                Node::Code(_) => {}
                Node::Link(_) => {}
                Node::Paragraph(_) => {}
                _ => {}
            }
        }

        node_ext.boundaries.push(Boundary {
            pre: pre,
            start: start,
            end: end,
            post: post,
        })
    }
    println!(
        "Node: {:?}, Boundaries{:?}, Position: {:?}",
        node_name(node),
        node_ext.boundaries,
        node.position()
    );
    if node.children().is_some() {
        for child in node.children().unwrap() {
            let mut child_ext = ExtendedNode {
                node: child,
                children: vec![],
                boundaries: vec![],
            };
            calc_boundaries_inner(child, &mut child_ext, context);
            // node_ext.children.push(child_ext);
        }
    }
}

/// Find start and end lines in a slice of text. Considers that start is the first character of a line and end is the last character of a line.
fn find_lines_start_end(text: &[u8], start: usize, end: usize) -> BTreeMap<usize, usize> {
    let mut end_lines: BTreeMap<usize, usize> = BTreeMap::new();

    let mut curr_index = start;
    let mut line_start = start;
    while curr_index < end {
        if text[curr_index] == b'\n' {
            end_lines.insert(line_start, curr_index);
            line_start = curr_index + 1;
        }
        curr_index += 1;
    }

    if text[end - 1] != b'\n' {
        end_lines.insert(line_start, end);
    }

    end_lines
}
