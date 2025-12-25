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
    pub indent: usize,
    // flag?
}

pub struct Context<'a> {
    pub text: &'a str,
    pub offset: Offset,
    pub nodes: Vec<Container>,
    pub current_container: ContainerId,
    pub above_container: Option<ContainerId>,
    pub offset_to_line_number: Vec<usize>,
    pub line_number_begin: Vec<usize>, // Contains the starting offset of each line
}

impl<'a> Context<'a> {
    pub fn new(text: &'a str, offset: Offset) -> Self {
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
            indent: 0,
        };

        Self {
            text,
            offset,
            nodes: vec![root],
            current_container: 0,
            above_container: None,
            offset_to_line_number: vec![],
            line_number_begin: vec![],
        }
    }

    pub fn add_container(&mut self, b_type: BlockType, bounds: Boundaries, detail: BlockDetail) -> ContainerId {
        let new_id = self.nodes.len(); ///todo: correct?
        let parent_id = self.current_container;

        let new_node = Container {
            id: new_id,
            b_type,
            children: vec![],
            parent: Some(parent_id),
            content_boundaries: vec![bounds],
            detail,
            attributes: Attributes::new(),
            closed: false,
            erase_block: false,
            repeated_marker: None,
            indent: 0,
        };

        self.nodes.push(new_node);
        self.nodes[parent_id].children.push(new_id);
        self.current_container = new_id;
        return new_id;
    }

    pub fn close_current(&mut self) {
        if let Some(parent) = self.nodes[self.current_container].parent {
            self.nodes[self.current_container].closed = true;
            self.current_container = parent;
        }
    }

}