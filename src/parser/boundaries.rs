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
    used_offsets_start: BTreeSet<usize>,
    used_offsets_end: BTreeSet<usize>,
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
        used_offsets_start: BTreeSet::new(),
        used_offsets_end: BTreeSet::new(),
    };

    println!("Lines: {:?}", context.lines);

    calc_boundaries_inner(node, &mut root_node, &mut context);

    root_node
}

fn calc_boundaries_inner<'a>(
    node: &'a Node,
    node_ext: &mut ExtendedNode<'a>,
    context: &mut Context<'a>,
) {
    // TODO: What about empty lines?
    if node.position().is_none() {
        return;
    }
    let node_position = node.position().unwrap();

    // Find the lines that are part of the node
    let min_key = context
        .lines
        .range(..=node_position.start.offset)
        .next_back()
        .unwrap()
        .0;
    let max_key = context
        .lines
        .range(..node_position.end.offset)
        .next_back()
        .unwrap()
        .0;
    let node_lines = context.lines.range(min_key..=max_key);

    for (i, (line_start, line_end)) in node_lines.enumerate() {
        let mut pre = *line_start;
        let mut start = *line_start;
        let mut end = *line_end;
        let mut post = *line_end;

        if node_position.start.offset > pre {
            pre = node_position.start.offset;
            start = node_position.start.offset;
        }
        if node_position.end.offset < post {
            end = node_position.end.offset;
            post = node_position.end.offset;
        }

        let mut start_rel_offset = 0;
        let mut end_rel_offset = 0;
        if node.children().is_some() && i == 0 {
            let first_child = node.children().unwrap().first().unwrap();
            let first_child_position = first_child.position().unwrap();
            let first_child_start = first_child_position.start.offset;
            start = first_child_start;
        } else {
            // Update pre and start for used offsets
            let mut used_start_range = context.used_offsets_start.range(*line_start..*line_end);
            let mut used_end_range = context.used_offsets_end.range(*line_start..*line_end);
            println!("    Used start: {:?}", used_start_range);
            if used_start_range.to_owned().count() > 0 {
                let used_start = used_start_range.next_back().unwrap();
                pre = *used_start + 1;
                start = *used_start + 1;
            }
            // if used_end_range.to_owned().count() > 0 {
            //     let used_end = used_end_range.next().unwrap();
            //     end = *used_end;
            //     post = *used_end;
            // }

            // We now go char by char to find the start and end of the delimiters of the node
            match node {
                Node::ListItem(_) => {
                    // ListItem should capture 2 to 5 whitespace characters
                    // depending if in children there are sublists
                }
                Node::BlockQuote(_) => {}
                Node::Code(_) => {}
                Node::Link(_) => {}
                Node::Paragraph(_) => {}
                _ => {}
            }
            start += start_rel_offset;
            end -= end_rel_offset;
            // Insert consumed offsets
        }

        for i in pre..start {
            context.used_offsets_start.insert(i);
        }
        for i in end..post {
            context.used_offsets_end.insert(i);
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

fn capture_n_whitespace(text: &str, start: usize, end: usize, n: i32) -> usize {
    let mut count = 0;
    for (i, byte) in text[start..end].bytes().enumerate() {
        if byte == b' ' {
            count += 1;
        } else if byte == b'\t' {
            count += 4;
        } else {
            break;
        }
        if count >= n {
            return i + 1;
        }
    }
    return end - start;
}

/// Find start and end lines in a slice of text. Considers that start is the first character of a line and end is the last character of a line.
fn find_lines_start_end(text: &[u8], start: usize, end: usize) -> BTreeMap<usize, usize> {
    let mut end_lines: BTreeMap<usize, usize> = BTreeMap::new();

    let mut curr_index = start;
    let mut line_start = start;
    while curr_index < end {
        if text[curr_index] == b'\n' {
            end_lines.insert(line_start, curr_index + 1);
            line_start = curr_index + 1;
        }
        curr_index += 1;
    }

    if text[end - 1] != b'\n' {
        end_lines.insert(line_start, end + 1);
    }

    end_lines
}
