use serde::Serialize;

use crate::definitions::{Offset, BlockType, BlockDetail, Boundaries, Attributes};

pub type ContainerId = usize;

#[derive(Debug, Clone)]
pub struct RepeatedMarker {
    pub marker: char,
    pub count: usize,
    pub allow_greater_number: bool,
    pub allow_chars_before_closing: bool,
    pub allow_attribute: bool,
}
impl Default for RepeatedMarker {
    fn default() -> Self {
        Self {
            marker: '\0',
            count: 0,
            allow_greater_number: false,
            allow_chars_before_closing: false,
            allow_attribute: true,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Container {
    pub id: ContainerId,
    pub b_type: BlockType,
    pub children: Vec<ContainerId>,
    pub parent: Option<ContainerId>,
    pub content_boundaries: Vec<Boundaries>,
    pub detail: BlockDetail,
    pub attributes: Attributes,
    pub closed: bool,
    pub erase_block: bool,
    pub repeated_marker: Option<RepeatedMarker>,
    pub last_non_empty_child_line: Option<usize>,
    pub indent: usize,
    pub flag: u32,
}


pub struct Context<'a> {
    pub text: &'a str,
    pub nodes: Vec<Container>,
    pub current_container: ContainerId,
    pub above_container: Option<ContainerId>,
    pub line_number_begin: Vec<usize>, // Contains the starting offset of each line
    node_count: usize,
}

impl<'a> Context<'a> {
    pub fn new(text: &'a str) -> Self {
        let root = Container {
            id: 0,
            b_type: BlockType::Doc,
            children: Vec::new(),
            parent: None,
            content_boundaries: Vec::new(),
            detail: BlockDetail::None,
            attributes: Attributes::new(),
            closed: false,
            erase_block: false,
            repeated_marker: None,
            last_non_empty_child_line: None,
            indent: 0,
            flag: 0,
        };

        Self {
            text,
            nodes: vec![root],
            current_container: 0,
            above_container: None,
            line_number_begin: vec![],
            node_count: 1,
        }
    }

    pub fn close_current(&mut self) {
        if let Some(parent) = self.nodes[self.current_container].parent {
            self.nodes[self.current_container].closed = true;
            self.current_container = parent;
        }
    }

    pub fn request_id(&mut self) -> ContainerId {
        let new_id = self.node_count;
        self.node_count += 1;
        new_id
    }

    pub fn get_node(&self, id: ContainerId) -> &Container {
        &self.nodes[id]
    }
    pub fn get_node_mut(&mut self, id: ContainerId) -> &mut Container {
        &mut self.nodes[id]
    }

    pub fn find_line_number(&self, offset: Offset) -> usize {
        match self.line_number_begin.binary_search(&offset) {
            Ok(line) => line,
            Err(line) => line - 1,
        }
    }
    pub fn find_next_line_offset(&self, offset: Offset) -> usize {
        let current_line = self.find_line_number(offset);
        if current_line + 1 >= self.line_number_begin.len() {
            self.text.len()
        } else {
            self.line_number_begin[current_line + 1] - 1
        }
    }
    pub fn select_last_child_container(&mut self) {
        if self.above_container.is_some() {
            let above_id = self.above_container.unwrap();
            if let Some(&last_child_id) = self.nodes[above_id].children.last() {
                self.above_container = Some(last_child_id);
                let node = &self.nodes[last_child_id];
                if node.b_type == BlockType::Ul || node.b_type == BlockType::Ol {
                    if let Some(&child) = node.children.last() {
                        self.above_container = Some(child);
                    }
                }
                self.current_container = self.above_container.unwrap();
            }
            else {
                self.above_container = None;
            }
        }
    }
    pub fn char_at(&self, offset: Offset) -> char {
        self.text.chars().nth(offset).unwrap()
    }

}