use std::collections::HashMap;

pub type Offset = usize;
pub type Attributes = HashMap<String, String>;

#[derive(Debug, Clone, PartialEq)]
pub struct Boundaries {
    pub line_number: Offset,
    pub pre: Offset,
    pub beg: Offset,
    pub end: Offset,
    pub post: Offset,
}

// A block represents a structural element in the document (e.g., paragraph, list, code block).
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BlockType {
    Doc,

    Hidden,

    Quote,

    Ul,
    Ol,
    Li,

    Special,
    Def,
    Math,

    Code,
    P, 
    
    Table,
    Thead,
    Tbody,
    Tr,
    Th,
    Td,

    Empty,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum OlType {
    Numeric,
    Alphabetic,
    Roman,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TaskState {
    Empty,
    Fail,
    Success,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum DefType {
    Footnote,
    Citation,
    Link,
}


#[derive(Debug, Clone)]
pub enum BlockDetail {
    Code { lang: String, num_ticks: i32 },
    Ol { 
        pre_marker: char, 
        post_marker: char, 
        is_numeric: bool, 
        ol_type: OlType,
    },
    Ul { marker: char },
    Li { 
        task_state: Option<TaskState>,
        is_task: bool,
        number: String,
        level: u8,
    },
    Def { def_type: DefType, name: String },
    Div { name: String },


    H { level: u8 },
    // Use Unit variant for blocks with no details
    None,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum SpanType {
    Em,
    Strong,
    Underline,
    Del,
    Highlight,
    Url,
    Img,
    Code,
    Math,
    Ref,
    None,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TextType {
    Normal,
    Math,
    Code,
}

#[derive(Debug, Clone)]
pub enum SpanDetail {
    Url { href: String },
    Img { src: String, alt: String },
    Ref { name: String },
    None,
}