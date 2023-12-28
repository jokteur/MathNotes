use markdown::mdast::*;
use std::fmt;

pub enum NodeExt {
    Root(RootExt),
    Paragraph(ParagraphExt),
    Heading(HeadingExt),
    ThematicBreak(ThematicBreakExt),
    BlockQuote(BlockQuoteExt),
    List(ListExt),
    ListItem(ListItemExt),
    Html(HtmlExt),
    Code(CodeExt),
    Math(MathExt),
    Definition(DefinitionExt),
    Text(TextExt),
    Emphasis(EmphasisExt),
    Strong(StrongExt),
    InlineCode(InlineCodeExt),
    InlineMath(InlineMathExt),
    Break(BreakExt),
    Link(LinkExt),
    Image(ImageExt),
    LinkReference(LinkReferenceExt),
    ImageReference(ImageReferenceExt),
    FootnoteDefinition(FootnoteDefinitionExt),
    FootnoteReference(FootnoteReferenceExt),
    Table(TableExt),
    TableRow(TableRowExt),
    TableCell(TableCellExt),
    Delete(DeleteExt),
    Yaml(YamlExt),
    Toml(TomlExt),
    MdxjsEsm(MdxjsEsmExt),
    MdxFlowExpression(MdxFlowExpressionExt),
    MdxTextExpression(MdxTextExpressionExt),
    MdxJsxFlowElement(MdxJsxFlowElementExt),
    MdxJsxTextElement(MdxJsxTextElementExt),
    MdxJsxAttribute(MdxJsxAttributeExt),
}

impl fmt::Debug for NodeExt {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            NodeExt::Root(x) => write!(f, "Root {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Paragraph(x) => write!(f, "Paragraph {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Heading(x) => write!(f, "Heading {{boundaries: {:?} }}", x.boundaries),
            NodeExt::ThematicBreak(x) => write!(f, "ThematicBreak {{boundaries: {:?} }}", x.boundaries),
            NodeExt::BlockQuote(x) => write!(f, "BlockQuote {{boundaries: {:?} }}", x.boundaries),
            NodeExt::List(x) => write!(f, "List {{boundaries: {:?} }}", x.boundaries),
            NodeExt::ListItem(x) => write!(f, "ListItem {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Html(x) => write!(f, "Html {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Code(x) => write!(f, "Code {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Math(x) => write!(f, "Math {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Definition(x) => write!(f, "Definition {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Text(x) => write!(f, "Text {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Emphasis(x) => write!(f, "Emphasis {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Strong(x) => write!(f, "Strong {{boundaries: {:?} }}", x.boundaries),
            NodeExt::InlineCode(x) => write!(f, "InlineCode {{boundaries: {:?} }}", x.boundaries),
            NodeExt::InlineMath(x) => write!(f, "InlineMath {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Break(x) => write!(f, "Break {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Link(x) => write!(f, "Link {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Image(x) => write!(f, "Image {{boundaries: {:?} }}", x.boundaries),
            NodeExt::LinkReference(x) => write!(f, "LinkReference {{boundaries: {:?} }}", x.boundaries),
            NodeExt::ImageReference(x) => write!(f, "ImageReference {{boundaries: {:?} }}", x.boundaries),
            NodeExt::FootnoteDefinition(x) => write!(f, "FootnoteDefinition {{boundaries: {:?} }}", x.boundaries),
            NodeExt::FootnoteReference(x) => write!(f, "FootnoteReference {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Table(x) => write!(f, "Table {{boundaries: {:?} }}", x.boundaries),
            NodeExt::TableRow(x) => write!(f, "TableRow {{boundaries: {:?} }}", x.boundaries),
            NodeExt::TableCell(x) => write!(f, "TableCell {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Delete(x) => write!(f, "Delete {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Yaml(x) => write!(f, "Yaml {{boundaries: {:?} }}", x.boundaries),
            NodeExt::Toml(x) => write!(f, "Toml {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxjsEsm(x) => write!(f, "MdxjsEsm {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxFlowExpression(x) => write!(f, "MdxFlowExpression {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxTextExpression(x) => write!(f, "MdxTextExpression {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxJsxFlowElement(x) => write!(f, "MdxJsxFlowElement {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxJsxTextElement(x) => write!(f, "MdxJsxTextElement {{boundaries: {:?} }}", x.boundaries),
            NodeExt::MdxJsxAttribute(x) => write!(f, "MdxJsxAttribute {{boundaries: {:?} }}", x.boundaries),
        }
    }
}

impl NodeExt {
    #[must_use]
    pub fn children(&self) -> Option<&Vec<Node>> {
        match self {
            // Parent.
            NodeExt::Root(x) => Some(&x.data.children),
            NodeExt::Paragraph(x) => Some(&x.data.children),
            NodeExt::Heading(x) => Some(&x.data.children),
            NodeExt::BlockQuote(x) => Some(&x.data.children),
            NodeExt::List(x) => Some(&x.data.children),
            NodeExt::ListItem(x) => Some(&x.data.children),
            NodeExt::Emphasis(x) => Some(&x.data.children),
            NodeExt::Strong(x) => Some(&x.data.children),
            NodeExt::Link(x) => Some(&x.data.children),
            NodeExt::LinkReference(x) => Some(&x.data.children),
            NodeExt::FootnoteDefinition(x) => Some(&x.data.children),
            NodeExt::Table(x) => Some(&x.data.children),
            NodeExt::TableRow(x) => Some(&x.data.children),
            NodeExt::TableCell(x) => Some(&x.data.children),
            NodeExt::Delete(x) => Some(&x.data.children),
            NodeExt::MdxJsxFlowElement(x) => Some(&x.data.children),
            NodeExt::MdxJsxTextElement(x) => Some(&x.data.children),
            // Non-parent.
            _ => None,
        }
    }

    pub fn children_mut(&mut self) -> Option<&mut Vec<Node>> {
        match self {
            // Parent.
            NodeExt::Root(x) => Some(&mut x.data.children),
            NodeExt::Paragraph(x) => Some(&mut x.data.children),
            NodeExt::Heading(x) => Some(&mut x.data.children),
            NodeExt::BlockQuote(x) => Some(&mut x.data.children),
            NodeExt::List(x) => Some(&mut x.data.children),
            NodeExt::ListItem(x) => Some(&mut x.data.children),
            NodeExt::Emphasis(x) => Some(&mut x.data.children),
            NodeExt::Strong(x) => Some(&mut x.data.children),
            NodeExt::Link(x) => Some(&mut x.data.children),
            NodeExt::LinkReference(x) => Some(&mut x.data.children),
            NodeExt::FootnoteDefinition(x) => Some(&mut x.data.children),
            NodeExt::Table(x) => Some(&mut x.data.children),
            NodeExt::TableRow(x) => Some(&mut x.data.children),
            NodeExt::TableCell(x) => Some(&mut x.data.children),
            NodeExt::Delete(x) => Some(&mut x.data.children),
            NodeExt::MdxJsxFlowElement(x) => Some(&mut x.data.children),
            NodeExt::MdxJsxTextElement(x) => Some(&mut x.data.children),
            // Non-parent.
            _ => None,
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "root")
)]
pub struct Boundary {
    pub line_number: usize,
    pub pre: usize,
    pub start: usize,
    pub end: usize,
    pub post: usize,
}

pub type Boundaries = Vec<Boundary>;

/// Document.
///
/// ```markdown
/// > | a
///     ^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "root")
)]
pub struct RootExt<'a> {
    pub data: &'a Root,
    pub boundaries: Option<Boundaries>,
}

/// Paragraph.
///
/// ```markdown
/// > | a
///     ^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "paragraph")
)]
pub struct ParagraphExt {
    pub data: Paragraph,
    pub boundaries: Option<Boundaries>,
}

/// Heading.
///
/// ```markdown
/// > | # a
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "heading")
)]
pub struct HeadingExt {
    pub data: Heading,
    pub boundaries: Option<Boundaries>,
}

/// Thematic break.
///
/// ```markdown
/// > | ***
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "thematicBreak")
)]
pub struct ThematicBreakExt {
    pub data: ThematicBreak,
    pub boundaries: Option<Boundaries>,
}

/// Block quote.
///
/// ```markdown
/// > | > a
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "blockquote")
)]
pub struct BlockQuoteExt {
    pub data: BlockQuote,
    pub boundaries: Option<Boundaries>,
}

/// List.
///
/// ```markdown
/// > | * a
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "list")
)]
pub struct ListExt {
    pub data: List,
    pub boundaries: Option<Boundaries>,
}

/// List item.
///
/// ```markdown
/// > | * a
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "listItem")
)]
pub struct ListItemExt {
    pub data: ListItem,
    pub boundaries: Option<Boundaries>,
}

/// Html (flow or phrasing).
///
/// ```markdown
/// > | <a>
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "html")
)]
pub struct HtmlExt {
    pub data: Html,
    pub boundaries: Option<Boundaries>,
}

/// Code (flow).
///
/// ```markdown
/// > | ~~~
///     ^^^
/// > | a
///     ^
/// > | ~~~
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "code")
)]
pub struct CodeExt {
    pub data: Code,
    pub boundaries: Option<Boundaries>,
}

/// Math (flow).
///
/// ```markdown
/// > | $$
///     ^^
/// > | a
///     ^
/// > | $$
///     ^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "math")
)]
pub struct MathExt {
    pub data: Math,
    pub boundaries: Option<Boundaries>,
}

/// Definition.
///
/// ```markdown
/// > | [a]: b
///     ^^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "definition")
)]
pub struct DefinitionExt {
    pub data: Definition,
    pub boundaries: Option<Boundaries>,
}

/// Text.
///
/// ```markdown
/// > | a
///     ^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "text")
)]
pub struct TextExt {
    pub data: Text,
    pub boundaries: Option<Boundaries>,
}

/// Emphasis.
///
/// ```markdown
/// > | *a*
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "emphasis")
)]
pub struct EmphasisExt {
    pub data: Emphasis,
    pub boundaries: Option<Boundaries>,
}

/// Strong.
///
/// ```markdown
/// > | **a**
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "strong")
)]
pub struct StrongExt {
    pub data: Strong,
    pub boundaries: Option<Boundaries>,
}

/// Code (phrasing).
///
/// ```markdown
/// > | `a`
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "inlineCode")
)]
pub struct InlineCodeExt {
    pub data: InlineCode,
    pub boundaries: Option<Boundaries>,
}

/// Math (phrasing).
///
/// ```markdown
/// > | $a$
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "inlineMath")
)]
pub struct InlineMathExt {
    pub data: InlineMath,
    pub boundaries: Option<Boundaries>,
}

/// Break.
///
/// ```markdown
/// > | a\
///      ^
///   | b
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "break")
)]
pub struct BreakExt {
    pub data: Break,
    pub boundaries: Option<Boundaries>,
}

/// Link.
///
/// ```markdown
/// > | [a](b)
///     ^^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "link")
)]
pub struct LinkExt {
    pub data: Link,
    pub boundaries: Option<Boundaries>,
}

/// Image.
///
/// ```markdown
/// > | ![a](b)
///     ^^^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "image")
)]
pub struct ImageExt {
    pub data: Image,
    pub boundaries: Option<Boundaries>,
}

/// Link reference.
///
/// ```markdown
/// > | [a]
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "linkReference")
)]
pub struct LinkReferenceExt {
    pub data: LinkReference,
    pub boundaries: Option<Boundaries>,
}

/// Image reference.
///
/// ```markdown
/// > | ![a]
///     ^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "imageReference")
)]
pub struct ImageReferenceExt {
    pub data: ImageReference,
    pub boundaries: Option<Boundaries>,
}

/// GFM: footnote definition.
///
/// ```markdown
/// > | [^a]: b
///     ^^^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "footnoteDefinition")
)]
pub struct FootnoteDefinitionExt {
    pub data: FootnoteDefinition,
    pub boundaries: Option<Boundaries>,
}

/// GFM: footnote reference.
///
/// ```markdown
/// > | [^a]
///     ^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "footnoteReference")
)]
pub struct FootnoteReferenceExt {
    pub data: FootnoteReference,
    pub boundaries: Option<Boundaries>,
}

/// GFM: table.
///
/// ```markdown
/// > | | a |
///     ^^^^^
/// > | | - |
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "table")
)]
pub struct TableExt {
    pub data: Table,
    pub boundaries: Option<Boundaries>,
}

/// GFM: table row.
///
/// ```markdown
/// > | | a |
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "tableRow")
)]
pub struct TableRowExt {
    pub data: TableRow,
    pub boundaries: Option<Boundaries>,
}

/// GFM: table cell.
///
/// ```markdown
/// > | | a |
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "tableCell")
)]
pub struct TableCellExt {
    pub data: TableCell,
    pub boundaries: Option<Boundaries>,
}

/// GFM: delete.
///
/// ```markdown
/// > | ~~a~~
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "delete")
)]
pub struct DeleteExt {
    pub data: Delete,
    pub boundaries: Option<Boundaries>,
}

/// Frontmatter: yaml.
///
/// ```markdown
/// > | ---
///     ^^^
/// > | a: b
///     ^^^^
/// > | ---
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "yaml")
)]
pub struct YamlExt {
    pub data: Yaml,
    pub boundaries: Option<Boundaries>,
}

/// Frontmatter: toml.
///
/// ```markdown
/// > | +++
///     ^^^
/// > | a: b
///     ^^^^
/// > | +++
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "toml")
)]
pub struct TomlExt {
    pub data: Toml,
    pub boundaries: Option<Boundaries>,
}

/// MDX: ESM.
///
/// ```markdown
/// > | import a from 'b'
///     ^^^^^^^^^^^^^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxjsEsm")
)]
pub struct MdxjsEsmExt {
    pub data: MdxjsEsm,
    pub boundaries: Option<Boundaries>,
}

/// MDX: expression (flow).
///
/// ```markdown
/// > | {a}
///     ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxFlowExpression")
)]
pub struct MdxFlowExpressionExt {
    pub data: MdxFlowExpression,
    pub boundaries: Option<Boundaries>,
}

/// MDX: expression (text).
///
/// ```markdown
/// > | a {b}
///       ^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxTextExpression")
)]
pub struct MdxTextExpressionExt {
    pub data: MdxTextExpression,
    pub boundaries: Option<Boundaries>,
}

/// MDX: JSX element (container).
///
/// ```markdown
/// > | <a />
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxJsxFlowElement")
)]
pub struct MdxJsxFlowElementExt {
    pub data: MdxJsxFlowElement,
    pub boundaries: Option<Boundaries>,
}

/// MDX: JSX element (text).
///
/// ```markdown
/// > | <a />.
///     ^^^^^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxJsxTextElement")
)]
pub struct MdxJsxTextElementExt {
    pub data: MdxJsxTextElement,
    pub boundaries: Option<Boundaries>,
}

/// MDX: JSX attribute.
///
/// ```markdown
/// > | <a b />
///        ^
/// ```
#[derive(Clone, Debug, Eq, PartialEq)]
#[cfg_attr(
    feature = "serde",
    derive(serde::Serialize, serde::Deserialize),
    serde(tag = "type", rename = "mdxJsxAttribute")
)]
pub struct MdxJsxAttributeExt {
    pub data: MdxJsxAttribute,
    pub boundaries: Option<Boundaries>,
}

