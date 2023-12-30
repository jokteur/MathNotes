use crate::parser::ast::node_name;
use markdown::mdast::Node;
use std::{
    collections::{BTreeMap, BTreeSet, HashSet},
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
    indent: usize,
    level: usize,
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
        indent: 0,
        level: 0,
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
    let num_lines = node_lines.clone().count();

    let children_starting_lines_offsets = get_children_start_line_offsets(node, context);

    let mut rel_indent = 0;

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

        // TODO: capture \n if not captured

        let mut start_rel_offset = 0;
        let mut end_rel_offset = 0;
        // On the first line of the node, it is possible to deduce start from its first child
        if i == 0 {
            if node.children().is_some() {
                if children_starting_lines_offsets.contains(line_start) {
                    let first_child = node.children().unwrap().first().unwrap();
                    let first_child_position = first_child.position().unwrap();
                    let first_child_start = first_child_position.start.offset;
                    start = first_child_start;
                }
            } else {
                match node {
                    Node::ListItem(_) => {
                        start = *line_end;
                    }
                    _ => {}
                }
            }
            rel_indent = start - pre;
        } else {
            // Update pre and start for used offsets
            let mut used_start_range = context.used_offsets_start.range(*line_start..*line_end);
            let mut used_end_range = context.used_offsets_end.range(*line_start..*line_end);
            if used_start_range.to_owned().count() > 0 {
                let used_start = used_start_range.next_back().unwrap();
                pre = *used_start + 1;
                start = *used_start + 1;
            }
            if used_end_range.to_owned().count() > 0 {
                let used_end = used_end_range.next().unwrap();
                end = *used_end;
                post = *used_end;
            }

            match node {
                Node::ListItem(_) => {
                    start_rel_offset =
                        capture_n_whitespace(context.text, start, end, rel_indent as i32);
                }
                Node::BlockQuote(_) => {
                    start_rel_offset = capture_n_whitespace(context.text, start, end, 3 as i32);
                }
                Node::Paragraph(_) => {
                    start_rel_offset =
                        capture_n_whitespace(context.text, start, end, (end - start) as i32);
                }
                _ => {}
            }
            start += start_rel_offset;
        }

        if node.children().is_some() && i + 1 == num_lines {
            // take last child
            let last_child = node.children().unwrap().last().unwrap();
            let last_child_position = last_child.position().unwrap();
            let last_child_end = last_child_position.end.offset;
            end_rel_offset += end - last_child_end;
        }
        end -= end_rel_offset;

        for i in pre..start {
            context.used_offsets_start.insert(i);
        }
        for i in end..post {
            context.used_offsets_end.insert(i);
        }

        // markdown-rs does not consider the last \n of a node to be part of the node
        if i + 1 == num_lines && post == *line_end - 1 {
            end += 1;
            post += 1;
        }

        node_ext.boundaries.push(Boundary {
            pre: pre,
            start: start,
            end: end,
            post: post,
        })
    }
    print!("{:.<1$}", "", context.level * 2);
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
            let level = context.level;
            context.level += 1;
            calc_boundaries_inner(child, &mut child_ext, context);
            context.level = level;
            node_ext.children.push(child_ext);
        }
    }
}

fn get_children_start_line_offsets(node: &Node, context: &Context) -> HashSet<usize> {
    let mut line_offsets = HashSet::new();

    fn find_child_line_start_offset(node: &Node, context: &Context) -> Option<usize> {
        if node.position().is_none() {
            return None;
        }
        let node_position = node.position().unwrap();
        let min_key = context
            .lines
            .range(..=node_position.start.offset)
            .next_back()
            .unwrap()
            .0;
        Some(*min_key)
    }

    if node.children().is_some() {
        for child in node.children().unwrap() {
            if child.position().is_none() {
                continue;
            }
            match child {
                // If we have a List, skip right to ListItem
                Node::List(_) => {
                    for list_child in child.children().unwrap() {
                        find_child_line_start_offset(list_child, context).and_then(|offset| {
                            line_offsets.insert(offset);
                            Some(offset)
                        });
                    }
                }
                _ => {
                    find_child_line_start_offset(child, context).and_then(|offset| {
                        line_offsets.insert(offset);
                        Some(offset)
                    });
                }
            };
        }
    }

    line_offsets
}

pub fn print_boundaries(text: &String, ext_node: &ExtendedNode) {
    println!("");
    println!("{}", text);
    fn print_inner(ext_node: &ExtendedNode, text: &String, level: usize) {
        let mut new_text = String::new();

        println!("");
        print!("{: <1$}", "", level * 2);
        println!("Node: {:?}", node_name(ext_node.node));
        let mut bound_it = ext_node.boundaries.iter();
        for i in 0..text.len() {
            let curr_bound = bound_it.clone().next();
            if text.as_bytes()[i] == b'\n' {
                new_text.push_str("\\n\n");
            } else if curr_bound.is_some() {
                let bound = curr_bound.unwrap();
                if i < bound.pre {
                    new_text.push('.');
                } else if i >= bound.pre && i < bound.start || i >= bound.end && i < bound.post {
                    new_text.push('x');
                } else if i >= bound.start && i < bound.end {
                    new_text.push('_');
                } else {
                    bound_it.next();
                }
            } else {
                new_text.push('.');
            }
        }
        // print!("{: <1$}", "", level * 2);
        println!("{}", new_text);
        for child in &ext_node.children {
            print_inner(child, text, level + 1);
        }
    }

    print_inner(ext_node, text, 0);
}

fn find_indent(text: &str, start: usize, end: usize) -> usize {
    let mut indent = 0;
    for byte in text[start..end - 1].bytes() {
        if byte == b' ' {
            indent += 1;
        } else if byte == b'\t' {
            indent += 4;
        } else {
            break;
        }
    }
    indent
}

fn capture_n_whitespace(text: &str, start: usize, end: usize, n: i32) -> usize {
    let mut count = 0;
    for (i, byte) in text[start..end].bytes().enumerate() {
        if count >= n {
            return i;
        }
        if byte == b' ' {
            count += 1;
        } else if byte == b'\t' {
            count += 4;
        } else {
            break;
        }
    }
    return count as usize;
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
