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
impl Default for Boundaries {
    fn default() -> Self {
        Self {
            line_number: 0,
            pre: 0,
            beg: 0,
            end: 0,
            post: 0,
        }
    }
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

    H,
    Hr,

    Special,
    Div,
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
    Code {
        lang: String,
        num_ticks: i32,
    },
    Ol {
        pre_marker: char,
        post_marker: char,
        lower_case: bool,
        ol_type: OlType,
    },
    Ul {
        marker: char,
    },
    Li {
        task_state: Option<TaskState>,
        is_task: bool,
        number: String,
        level: u8,
    },
    Def {
        def_type: DefType,
        name: String,
    },
    Div {
        name: String,
    },

    H {
        level: u8,
    },
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

pub fn block_to_name(b: BlockType) -> &'static str {
    match b {
        BlockType::Doc => "DOCUMENT",
        BlockType::Hidden => "B_HIDDEN",
        BlockType::Quote => "B_QUOTE",
        BlockType::Ul => "B_UL",
        BlockType::Ol => "B_OL",
        BlockType::Li => "B_LI",
        BlockType::Hr => "B_HR",
        BlockType::H => "B_H",
        BlockType::Special => "B_SPECIAL",
        BlockType::Div => "B_DIV",
        BlockType::Def => "B_DEF",
        BlockType::Math => "B_MATH",
        BlockType::Code => "B_CODE",
        BlockType::P => "B_P",
        BlockType::Table => "B_TABLE",
        BlockType::Thead => "B_THEAD",
        BlockType::Tbody => "B_TBODY",
        BlockType::Tr => "B_TR",
        BlockType::Th => "B_TH",
        BlockType::Td => "B_TD",
        BlockType::Empty => "B_EMPTY",
    }
}

pub fn block_to_html(b: BlockType) -> &'static str {
    match b {
        BlockType::Doc => "doc",
        BlockType::Hidden => "hidden",
        BlockType::Quote => "blockquote",
        BlockType::Ul => "ul",
        BlockType::Ol => "ol",
        BlockType::Li => "li",
        BlockType::Hr => "hr",
        BlockType::H => "h",
        BlockType::Special => "special",
        BlockType::Div => "div",
        BlockType::Def => "def",
        BlockType::Math => "math",
        BlockType::Code => "code",
        BlockType::P => "p",
        BlockType::Table => "table",
        BlockType::Thead => "thead",
        BlockType::Tbody => "tbody",
        BlockType::Tr => "tr",
        BlockType::Th => "th",
        BlockType::Td => "td",
        BlockType::Empty => "empty",
    }
}

pub fn span_to_name(s: SpanType) -> &'static str {
    match s {
        SpanType::Em => "S_EM",
        SpanType::Strong => "S_STRONG",
        SpanType::Underline => "S_UNDERLINE",
        SpanType::Del => "S_DELETE",
        SpanType::Highlight => "S_HIGHLIGHT",
        SpanType::Url => "S_URL",
        SpanType::Img => "S_IMG",
        SpanType::Code => "S_CODE",
        SpanType::Math => "S_MATH",
        SpanType::Ref => "S_REF",
    }
}

pub fn span_to_html(s: SpanType) -> &'static str {
    match s {
        SpanType::Em => "em",
        SpanType::Strong => "strong",
        SpanType::Underline => "u",
        SpanType::Del => "del",
        SpanType::Highlight => "highlight",
        SpanType::Url => "a",
        SpanType::Img => "img",
        SpanType::Code => "code",
        SpanType::Math => "math",
        SpanType::Ref => "ref",
    }
}

pub fn text_to_name(t: TextType) -> &'static str {
    match t {
        TextType::Normal => "TEXT",
        TextType::Code => "TEXT_CODE",
        TextType::Math => "TEXT_MATH",
    }
}
