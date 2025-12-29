use crate::commons::*;
use crate::definitions::*;
use crate::internal::*;
use std::collections::{HashMap, HashSet};

const MAX_VERB_OPENINGS: usize = 32;

const S_EM_SIMPLE: u32 = 0x1;
const S_EM: u32 = 0x2;
const S_STRONG_SIMPLE: u32 = 0x4;
const S_STRONG: u32 = 0x8;
const S_HIGHLIGHT: u32 = 0x10;
const S_UNDERLINE: u32 = 0x20;
const S_DELETE: u32 = 0x40;
const S_VERBATIME: u32 = 0x80;
const S_LINK: u32 = 0x100;
const S_LINKDEF: u32 = 0x200;
const S_AUTOLINK: u32 = 0x400;
const S_REF: u32 = 0x800;
const S_INSERTED_REF: u32 = 0x1000;
const S_IMG: u32 = 0x2000;
const S_IMG_TITLE: u32 = 0x4000;
const S_IMG_DEF: u32 = 0x8000;
const S_MATH: u32 = 0x10000;
const S_ATTRIBUTE: u32 = 0x20000;

fn flag_to_type(flag: u32) -> SpanType {
    match flag {
        S_EM_SIMPLE | S_EM => SpanType::Em,
        S_STRONG_SIMPLE | S_STRONG => SpanType::Strong,
        S_VERBATIME => SpanType::Code,
        S_HIGHLIGHT => SpanType::Highlight,
        S_UNDERLINE => SpanType::Underline,
        S_DELETE => SpanType::Del,
        S_LINK | S_LINKDEF | S_AUTOLINK => SpanType::Url,
        S_REF | S_INSERTED_REF => SpanType::Ref,
        S_IMG | S_IMG_TITLE | S_IMG_DEF => SpanType::Img,
        S_MATH => SpanType::Math,
        _ => SpanType::Empty,
    }
}

const SELECT_ALL: u32 = !0;
const SELECT_REFS: u32 = S_REF | S_INSERTED_REF;
const SELECT_IMGS: u32 = S_IMG | S_IMG_DEF | S_IMG_TITLE;
const SELECT_LINKS: u32 = S_LINK | S_LINKDEF | S_AUTOLINK;
const SELECT_ALL_LINKTYPE: u32 = SELECT_REFS | SELECT_IMGS | SELECT_LINKS;

/**
 * Mark stores the rules for span detection
 *
 * s_type
 *     name (flag) of the span
 * open
 *     rule for opening the span
 * closing
 *     rule for closing the span
 * need_ws_or_punct
 *     when true, the opening must be preceeded by a punctuation or a
 *     whitespace and after the closing a punctuation or a whitespace
 *     is needed
 * dont_allow_inside
 *     flags of spans not allowed inside. e.g. a link cannot contain other links
 * repeat
 *     if true, then the opening and closing chars can be repeated as many times
 *     as needed
 * count
 *     stores the number of repeat chars if the previous attribute is true
 * second_close
 *     sometimes, spans are defined by a second closing rule
 *     e.g. [abc](def)  -->  open = "[", close = "](", second_close =")"
 *
 * The other attributes are used for solving the spans
 *
 * solved:
 *     when true, it means that the mark has been solved (i.e. opening and closing found)
 * is_closing:
 *     if true, then mark is used as a closing landmark in the mark_chain
 * pre, beg, line_number:
 *     starting bounds of the span
 * true_bounds:
 *     once the span is solved, we calculate the true boundaries and store them there
 */
#[derive(Clone, Debug)]
struct Mark {
    // Configuration / Rules
    pub s_type: u32,
    pub open: &'static str,
    pub close: &'static str,
    pub need_ws_or_punct: bool,
    pub dont_allow_inside: u32,
    pub repeat: bool,
    pub count: usize,
    pub second_close: &'static str,
    pub no_self_nested: bool,
    pub jump_after_match: bool,

    // Runtime state
    pub solved: bool,
    pub is_closing: bool,
    pub pre: Offset,
    pub beg: Offset,
    pub line_number: Offset,
    pub true_bounds: Vec<Boundaries>,
    pub start_idx: Option<usize>,
    pub attributes: Attributes,
}

impl Default for Mark {
    fn default() -> Self {
        Self {
            s_type: 0,
            open: "",
            close: "",
            need_ws_or_punct: false,
            dont_allow_inside: 0,
            repeat: false,
            count: 0,
            second_close: "",
            no_self_nested: false,
            jump_after_match: false,
            solved: false,
            is_closing: false,
            pre: 0,
            beg: 0,
            line_number: 0,
            start_idx: None,
            true_bounds: Vec::new(),
            attributes: Attributes::default(),
        }
    }
}

fn get_opening_marks() -> HashSet<char> {
    ['!', '[', '*', '`', '_', '{', '$', 'h'].iter().cloned().collect()
}

fn get_closing_marks() -> HashSet<char> {
    [']', '=', '*', '+', '-', '_', '`', '$', '}'].iter().cloned().collect()
}

pub fn get_marks() -> Vec<Mark> {
    vec![
        Mark {
            s_type: S_EM,
            open: "{_",
            close: "_}",
            ..Default::default()
        },
        Mark {
            s_type: S_EM_SIMPLE,
            open: "_",
            close: "_",
            need_ws_or_punct: true,
            ..Default::default()
        },
        Mark {
            s_type: S_STRONG,
            open: "{*",
            close: "*}",
            ..Default::default()
        },
        Mark {
            s_type: S_STRONG_SIMPLE,
            open: "*",
            close: "*",
            need_ws_or_punct: true,
            ..Default::default()
        },
        Mark {
            s_type: S_VERBATIME,
            open: "`",
            close: "`",
            dont_allow_inside: SELECT_ALL,
            repeat: true,
            ..Default::default()
        },
        Mark {
            s_type: S_HIGHLIGHT,
            open: "{=",
            close: "=}",
            ..Default::default()
        },
        Mark {
            s_type: S_UNDERLINE,
            open: "{+",
            close: "+}",
            ..Default::default()
        },
        Mark {
            s_type: S_DELETE,
            open: "{-",
            close: "-}",
            ..Default::default()
        },
        Mark {
            s_type: S_INSERTED_REF,
            open: "![[",
            close: "]]",
            dont_allow_inside: SELECT_ALL,
            ..Default::default()
        },
        Mark {
            s_type: S_REF,
            open: "[[",
            close: "]]",
            dont_allow_inside: SELECT_ALL,
            ..Default::default()
        },
        Mark {
            s_type: S_IMG_TITLE,
            open: "![",
            close: "](",
            dont_allow_inside: SELECT_ALL_LINKTYPE,
            second_close: ")",
            ..Default::default()
        },
        Mark {
            s_type: S_IMG_DEF,
            open: "![",
            close: "][",
            dont_allow_inside: SELECT_ALL_LINKTYPE,
            second_close: "]",
            ..Default::default()
        },
        Mark {
            s_type: S_IMG,
            open: "![",
            close: "]",
            dont_allow_inside: SELECT_ALL,
            ..Default::default()
        },
        Mark {
            s_type: S_LINK,
            open: "[",
            close: "](",
            dont_allow_inside: SELECT_ALL_LINKTYPE,
            second_close: ")",
            no_self_nested: true,
            ..Default::default()
        },
        Mark {
            s_type: S_LINKDEF,
            open: "[",
            close: "][",
            dont_allow_inside: SELECT_ALL_LINKTYPE,
            second_close: "]",
            no_self_nested: true,
            ..Default::default()
        },
        Mark {
            s_type: S_MATH,
            open: "$$",
            close: "$$",
            dont_allow_inside: SELECT_ALL,
            jump_after_match: true,
            ..Default::default()
        },
        Mark {
            s_type: S_ATTRIBUTE,
            open: "{{",
            close: "}}",
            dont_allow_inside: SELECT_ALL,
            ..Default::default()
        },
        /* Autolinks handled by lookahead_autolink*/
    ]
}

// Indices for the marks vector
// Refer to get_marks() function
const M_EM: usize = 0;
const M_EM_SIMPLE: usize = 1;
const M_STRONG: usize = 2;
const M_STRONG_SIMPLE: usize = 3;
const M_VERBATIME: usize = 4;
const M_HIGHLIGHT: usize = 5;
const M_UNDERLINE: usize = 6;
const M_DELETE: usize = 7;
const M_INSERTED_REF: usize = 8;
const M_REF: usize = 9;
const M_IMG_TITLE: usize = 10;
const M_IMG_DEF: usize = 11;
const M_IMG: usize = 12;
const M_LINK: usize = 13;
const M_LINKDEF: usize = 14;
const M_MATH: usize = 15;
const M_ATTRIBUTE: usize = 16;

type MarkChain = Vec<Mark>;

#[inline]
fn check_match(ctx: &Context, txt: &str, i: &mut usize, off: Offset, end: Offset) -> bool {
    let mut count = 0;
    while *i < txt.len() && off + (*i) < end {
        if txt.chars().nth(*i).unwrap() != ctx.char_at(off + *i) {
            return false;
        }
        *i += 1;
        count += 1;
    }
    if count < txt.len() {
        return false;
    }
    true
}

#[inline]
fn add_to_flag_count(flag_count: &mut HashMap<u32, u32>, flag: u32) {
    *flag_count.entry(flag).or_insert(0) += 1;
}

#[inline]
fn remove_from_flag_count(flag_count: &mut HashMap<u32, u32>, flag: u32) {
    if let Some(count) = flag_count.get_mut(&flag) {
        if *count > 0 {
            *count -= 1;
        }
    }
}

#[inline]
fn is_count_positive(flag_count: &HashMap<u32, u32>, flag: u32) -> bool {
    flag_count.get(&flag).map_or(false, |&count| count > 0)
}

fn close_mark(
    ctx: &Context,
    mark_chain: &mut MarkChain,
    mark: &Mark,
    off: &mut Offset,
    end: Offset,
    content_bounds: &[Boundaries],
    flag_count: &mut HashMap<u32, u32>,
) -> bool {
    let mut found_match = true;
    let mut i = 0;
    let mut jump_to = *off;
    let mut mark_count = 0;

    if mark.repeat {
        let ch = mark.close.chars().next().unwrap_or('\0');
        while *off + i < end && i < MAX_VERB_OPENINGS {
            if ch != ctx.char_at(*off + i) {
                break;
            }
            mark_count += 1;
            i += 1;
        }
        if mark_count > 0 {
            jump_to = jump_to + i;
        } else {
            found_match = false;
        }
    } else {
        found_match = check_match(ctx, mark.close, &mut i, *off, end);
        jump_to = jump_to + i;

        if !mark.second_close.is_empty() && found_match {
            found_match = false;
            let mut tmp_off = *off + 1;

            // Look ahead
            while tmp_off < end {
                if ctx.char_at(tmp_off) == mark.second_close.chars().next().unwrap_or('\0') {
                    let mut j = 0;
                    found_match = check_match(ctx, mark.second_close, &mut j, tmp_off, end);
                    if found_match {
                        jump_to = tmp_off + j;
                    }
                    break;
                }
                tmp_off += 1;
            }
        }
    }

    if found_match
        && jump_to < end
        && mark.need_ws_or_punct
        && !ctx.char_at(jump_to).is_whitespace()
        && !ctx.char_at(jump_to).is_ascii_punctuation()
    {
        found_match = false;
    }

    if found_match {
        // Find matching mark and collect marks to remove
        let mut matching_mark_idx = None;
        let mut marks_to_remove = Vec::new();

        // Convert LinkedList to Vec for easier reverse iteration
        let mut marks: Vec<_> = mark_chain.iter().enumerate().collect();
        marks.reverse();

        for (original_idx, current_mark) in marks {
            if current_mark.solved {
                if (current_mark.s_type & mark.dont_allow_inside) == 0 {
                    continue;
                }
            } else if current_mark.s_type == mark.s_type && current_mark.count == mark_count {
                matching_mark_idx = Some(original_idx);
                break;
            }
            marks_to_remove.push(original_idx);
        }

        if let Some(match_idx) = matching_mark_idx {
            // Get the matching mark and modify it
            let mut mark_vec: Vec<_> = mark_chain.drain(..).collect();

            if match_idx < mark_vec.len() {
                let mut matching_mark = &mut mark_vec[match_idx];
                matching_mark.solved = true;

                /* Need to calculate the true boundaries of the span
                 * A span can be on multiple lines, this is why we need
                 * to use content_boundaries */
                let tmp_mark = matching_mark.clone();

                let b_end = *off;
                let b_post = jump_to;
                let line_number = ctx.find_line_number(b_end);

                if line_number > tmp_mark.line_number {
                    let mut bound_iter = content_bounds.iter();
                    while let Some(bound) = bound_iter.next() {
                        if bound.line_number == matching_mark.line_number as Offset {
                            break;
                        }
                    }

                    if let Some(start_bound) = bound_iter.as_slice().first() {
                        // Starting boundary
                        matching_mark.true_bounds.push(Boundaries {
                            line_number: matching_mark.line_number as Offset,
                            pre: tmp_mark.pre,
                            beg: tmp_mark.beg,
                            end: start_bound.end,
                            post: start_bound.end,
                        });

                        // In-between boundaries
                        for bound in bound_iter {
                            if bound.line_number == line_number as Offset {
                                break;
                            }
                            matching_mark.true_bounds.push(Boundaries {
                                line_number: bound.line_number,
                                pre: bound.beg,
                                beg: bound.beg,
                                end: bound.end,
                                post: bound.end,
                            });
                        }

                        // Last boundary
                        if let Some(last_bound) = content_bounds.iter().find(|b| b.line_number == line_number as Offset)
                        {
                            matching_mark.true_bounds.push(Boundaries {
                                line_number: line_number as Offset,
                                pre: last_bound.beg,
                                beg: last_bound.beg,
                                end: b_end,
                                post: b_post,
                            });
                        }
                    }
                } else {
                    matching_mark.true_bounds.push(Boundaries {
                        line_number: line_number as Offset,
                        pre: tmp_mark.pre,
                        beg: tmp_mark.beg,
                        end: b_end,
                        post: b_post,
                    });
                }

                // Remove marks that should be erased
                marks_to_remove.sort_by(|a, b| b.cmp(a)); // Sort in descending order
                for &idx in &marks_to_remove {
                    if idx < mark_vec.len() {
                        remove_from_flag_count(flag_count, mark_vec[idx].s_type);
                        mark_vec.remove(idx);
                    }
                }

                // Create closing mark
                let mut closing_mark = tmp_mark.clone();
                closing_mark.solved = true;
                closing_mark.is_closing = true;
                // Link closing mark to its opener index (still valid because we only
                // remove marks after the opener)
                closing_mark.start_idx = Some(match_idx);

                // Rebuild the Vec
                *mark_chain = mark_vec;
                mark_chain.push(closing_mark);

                *off = jump_to;
                return true;
            }
        }
    }

    false
}

fn open_mark(
    ctx: &Context,
    mark_chain: &mut MarkChain,
    mark: &Mark,
    off: Offset,
    end: Offset,
    ws_or_punct_before: bool,
    flag_count: &mut HashMap<u32, u32>,
) -> usize {
    let mut found_match = true;
    let mut i = 0;
    let mut mark_count: usize = 0;

    if mark.repeat {
        let ch = mark.open.chars().next().unwrap_or_default();
        while off + i < end && i < MAX_VERB_OPENINGS {
            // Assuming CH() is a function to get character at position
            if ch != ctx.char_at(off + i) {
                break;
            }
            mark_count += 1;
            i += 1;
        }
        if mark_count == 0 {
            found_match = false;
        }
    } else {
        found_match = check_match(ctx, &mark.open, &mut i, off, end);
    }

    if mark.need_ws_or_punct && !ws_or_punct_before {
        found_match = false;
    }

    if found_match {
        let mut tmp_mark = mark.clone();
        tmp_mark.line_number = ctx.find_line_number(off);
        tmp_mark.pre = off;

        if mark.repeat {
            tmp_mark.count = mark_count;
            tmp_mark.beg = off + mark_count;
        } else {
            tmp_mark.beg = off + mark.open.len();
        }

        mark_chain.push(tmp_mark);
        add_to_flag_count(flag_count, mark.s_type);

        if mark.jump_after_match {
            mark.open.len()
        } else {
            mark_count
        }
    } else {
        0
    }
}

fn lookahead_autolink(ctx: &Context, mark_chain: &mut Vec<Mark>, off: &mut Offset, end: Offset) -> bool {
    // Basic efficient sanity check, look for http:// or https://
    if *off + 7 >= end {
        return false;
    }
    if ctx.char_at(*off + 1) != 't' || ctx.char_at(*off + 2) != 't' || ctx.char_at(*off + 3) != 'p' {
        return false;
    }
    let mut is_https = false;
    if ctx.char_at(*off + 4) == 's' {
        is_https = true;
    }
    if is_https && *off + 8 >= end {
        return false;
    }

    if ctx.char_at(*off + 4 + is_https as Offset) != ':'
        || ctx.char_at(*off + 5 + is_https as Offset) != '/'
        || ctx.char_at(*off + 6 + is_https as Offset) != '/'
    {
        return false;
    }

    let start = *off;
    while *off < end {
        let ch = ctx.char_at(*off);
        if ch.is_ascii_whitespace() || ch == '[' || ch == ']' {
            break;
        } else if ch.is_ascii_punctuation() && check_ws_or_end(ctx, *off + 1) {
            break;
        }
        *off += 1;
    }

    let autolink = Mark {
        s_type: S_AUTOLINK,
        open: "http://",
        close: " ",
        repeat: false,
        dont_allow_inside: SELECT_ALL,
        start_idx: None,
        solved: true,
        is_closing: false,
        true_bounds: vec![Boundaries {
            line_number: ctx.find_line_number(start),
            beg: start,
            pre: start,
            end: *off,
            post: *off,
        }],
        ..Default::default()
    };

    mark_chain.push(autolink.clone());

    let mark_index = mark_chain.len() - 1;

    // Create closing mark
    let mut closing_autolink = autolink;
    closing_autolink.start_idx = Some(mark_index);
    closing_autolink.is_closing = true;
    mark_chain.push(closing_autolink);

    true
}

fn create_text<P: crate::Parser>(
    parser: &mut P,
    ctx: &Context,
    boundaries: &[Boundaries],
    text_type: TextType,
    start: Offset,
    end: Offset,
) -> Result<(), String> {
    if start == end || end > ctx.text.len() {
        return Ok(());
    }

    let mut bounds = Vec::<Boundaries>::new();
    let mut start = start;
    if boundaries.is_empty() {
        return Ok(());
    }

    // Choose the boundary corresponding to the start offset's line
    let start_line = ctx.find_line_number(start);
    let mut start_idx = 0usize;
    for (i, b) in boundaries.iter().enumerate() {
        if b.line_number as usize == start_line {
            start_idx = i;
            break;
        }
        if b.line_number as usize > start_line {
            break;
        }
    }
    let mut b_it = boundaries.iter().skip(start_idx).peekable();
    let mut current_b = b_it.next().unwrap_or(&boundaries[boundaries.len() - 1]);
    // Edge case if the cursor just stopped on a new line
    if start == current_b.post && b_it.peek().is_some() {
        current_b = b_it.next().unwrap();
        start = current_b.beg;
    }

    let last_line = ctx.find_line_number(end);
    let mut diff = last_line as isize - current_b.line_number as isize;
    if diff > 0 {
        bounds.push(Boundaries {
            line_number: current_b.line_number,
            pre: start,
            beg: start,
            end: current_b.end,
            post: current_b.end,
        });
        while let Some(b) = b_it.next() {
            if diff <= 1 {
                break;
            }
            bounds.push(Boundaries {
                line_number: b.line_number,
                pre: start,
                beg: start,
                end: b.end,
                post: b.end,
            });
            start = b.beg;
            diff = last_line as isize - b.line_number as isize;
        }
        if let Some(b) = b_it.next()
            && b.beg < end
        {
            bounds.push(Boundaries {
                line_number: b.line_number,
                pre: b.beg,
                beg: b.beg,
                end: end,
                post: end,
            });
        }
    } else {
        bounds.push(Boundaries {
            line_number: current_b.line_number,
            pre: start,
            beg: start,
            end: end,
            post: end,
        });
    }

    // Equivalent of CHECK_AND_RET macro
    parser.text(text_type, &bounds)?;
    Ok(())
}

fn main_loop(ctx: &Context, node: &Container, mark_chain: &mut MarkChain) {
    let mut flag_count: HashMap<u32, u32> = HashMap::new();
    let marks = get_marks();

    macro_rules! open_mark_wrapper {
        ($mark:expr, $off:expr, $ws_or_punct_before:expr, $advance:expr) => {{
            let mark_count = open_mark(
                ctx,
                mark_chain,
                $mark,
                $off,
                ctx.text.len(),
                $ws_or_punct_before,
                &mut flag_count,
            );
            if mark_count > *$advance {
                *$advance = mark_count;
            }
        }};
    }

    macro_rules! close_mark_wrapper {
        ($mark:expr, $off:expr, $advance:expr, $success:expr) => {{
            if !*$success && is_count_positive(&flag_count, $mark.s_type) {
                *$success = close_mark(
                    ctx,
                    mark_chain,
                    $mark,
                    $off,
                    ctx.text.len(),
                    &node.content_boundaries,
                    &mut flag_count,
                );
                if *$success {
                    remove_from_flag_count(&mut flag_count, $mark.s_type);
                    *$advance = 0;
                }
            }
        }};
    }

    for bound in &node.content_boundaries {
        let mut prev_is_ws = true;
        let mut prev_is_punct = true;
        /* Kind of a hack to avoid making ![[]] become !<ref />*/
        let prev_is_exclamation_or_bracket = false;
        let mut off = bound.beg;
        while off < bound.end {
            let mut success = false;
            let mut advance = 1;
            let prev_is_ws_or_punct = prev_is_ws || prev_is_punct;

            let ch = ctx.char_at(off);
            if ch == '\\' {
                let is_verbatim = mark_chain.last().map(|m| m.s_type & S_VERBATIME != 0).unwrap_or(false);
                let is_next_tick = if off + 1 < ctx.text.len() && ctx.char_at(off) == '`' {
                    true
                } else {
                    false
                };
                /* Edge case for `\` */
                if !mark_chain.is_empty() && is_verbatim && is_next_tick {
                    off += 1;
                } else {
                    off += 2;
                }
                continue;
            } else if ch.is_whitespace() {
                prev_is_ws = true;
            }
            /* Opening marks, num based on table marks */
            else if ch == '{' {
                open_mark_wrapper!(&marks[M_EM], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_STRONG], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_HIGHLIGHT], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_UNDERLINE], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_DELETE], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_ATTRIBUTE], off, prev_is_ws_or_punct, &mut advance);
            } else if ch == '*' {
                close_mark_wrapper!(&marks[M_STRONG], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_STRONG_SIMPLE], &mut off, &mut advance, &mut success);
                if !success {
                    open_mark_wrapper!(&marks[M_STRONG_SIMPLE], off, prev_is_ws_or_punct, &mut advance);
                }
            } else if ch == '_' {
                close_mark_wrapper!(&marks[M_EM], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_EM_SIMPLE], &mut off, &mut advance, &mut success);
                if !success {
                    open_mark_wrapper!(&marks[M_EM_SIMPLE], off, prev_is_ws_or_punct, &mut advance);
                }
            } else if ch == '`' {
                close_mark_wrapper!(&marks[M_VERBATIME], &mut off, &mut advance, &mut success);
                if !success {
                    open_mark_wrapper!(&marks[M_VERBATIME], off, prev_is_ws_or_punct, &mut advance);
                }
            } else if ch == '!' {
                open_mark_wrapper!(&marks[M_INSERTED_REF], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_IMG_TITLE], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_IMG_DEF], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_IMG], off, prev_is_ws_or_punct, &mut advance);
            } else if ch == '[' {
                open_mark_wrapper!(&marks[M_REF], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_LINK], off, prev_is_ws_or_punct, &mut advance);
                open_mark_wrapper!(&marks[M_LINKDEF], off, prev_is_ws_or_punct, &mut advance);
            } else if ch == '$' {
                close_mark_wrapper!(&marks[M_MATH], &mut off, &mut advance, &mut success);
                if !success {
                    open_mark_wrapper!(&marks[M_MATH], off, prev_is_ws_or_punct, &mut advance);
                }
            } else if ch == 'h' {
                if lookahead_autolink(ctx, mark_chain, &mut off, bound.end) {
                    continue;
                }
            } else if ch == '=' {
                close_mark_wrapper!(&marks[M_HIGHLIGHT], &mut off, &mut advance, &mut success);
            } else if ch == '+' {
                close_mark_wrapper!(&marks[M_UNDERLINE], &mut off, &mut advance, &mut success);
            } else if ch == '-' {
                close_mark_wrapper!(&marks[M_DELETE], &mut off, &mut advance, &mut success);
            } else if ch == ']' {
                close_mark_wrapper!(&marks[M_REF], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_INSERTED_REF], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_IMG_TITLE], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_IMG_DEF], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_IMG], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_LINK], &mut off, &mut advance, &mut success);
                close_mark_wrapper!(&marks[M_LINKDEF], &mut off, &mut advance, &mut success);
            } else if ch == '}' {
                close_mark_wrapper!(&marks[M_ATTRIBUTE], &mut off, &mut advance, &mut success);
            }

            if ch.is_ascii_punctuation() {
                prev_is_punct = true;
            }

            if !success {
                off += advance;
            }
        }
    }
}

fn mark_cleanup(ctx: &Context, mark_chain: &mut MarkChain) {
    // Cleanup of unmatched spans and attribute creation
    // Strategy: first collect removals and attribute updates immutably,
    // then apply updates mutably, and finally remove by descending indices.

    #[derive(Clone, Copy)]
    struct AttrUpdate {
        start_idx: usize,
        attr_beg: Offset,
        new_post: Offset,
    }

    let mut to_remove: Vec<usize> = Vec::new();
    let mut updates: Vec<AttrUpdate> = Vec::new();

    let mut i = 0usize;
    while i < mark_chain.len() {
        let mark = &mark_chain[i];

        // Remove unmatched (unsolved) spans
        if !mark.solved {
            to_remove.push(i);
            i += 1;
            continue;
        }

        // Process solved attribute marks
        if mark.s_type & S_ATTRIBUTE != 0 {
            // Try to attach attributes to the previous solved span (its opener via start_idx)
            if i > 0 {
                let prev = &mark_chain[i - 1];
                if prev.solved {
                    if let Some(start_idx) = prev.start_idx {
                        if let Some(prev_open) = mark_chain.get(start_idx) {
                            if let Some(prev_bound) = prev_open.true_bounds.last() {
                                // Same line and only whitespace between prev bound.post and current attribute.pre
                                let mut valid_attribute = prev_bound.line_number == mark.line_number;
                                if valid_attribute {
                                    let start = prev_bound.post;
                                    let end = mark.pre.min(ctx.text.len());
                                    let mut off = start;
                                    while off < end {
                                        if !ctx.char_at(off).is_whitespace() {
                                            valid_attribute = false;
                                            break;
                                        }
                                        off += 1;
                                    }
                                }

                                if valid_attribute {
                                    let new_post = mark.true_bounds.last().map(|b| b.post).unwrap_or(prev_bound.post);
                                    updates.push(AttrUpdate {
                                        start_idx,
                                        attr_beg: mark.beg,
                                        new_post,
                                    });
                                }
                            }
                        }
                    }
                }
            }

            // Always remove the attribute pair (assumed to be consecutive solved marks)
            to_remove.push(i);
            if i + 1 < mark_chain.len() {
                to_remove.push(i + 1);
            }
            i += 2;
            continue;
        }

        i += 1;
    }

    // Apply attribute updates
    for upd in updates {
        if let Some(start) = mark_chain.get_mut(upd.start_idx) {
            let mut off = upd.attr_beg;
            start.attributes = parse_attributes(ctx, &mut off);
            if let Some(last) = start.true_bounds.last_mut() {
                last.post = upd.new_post;
            }
        }
    }

    // Remove collected indices in descending order to keep indices valid
    to_remove.sort_unstable();
    to_remove.dedup();
    for idx in to_remove.into_iter().rev() {
        if idx < mark_chain.len() {
            mark_chain.remove(idx);
        }
    }
}

fn parse_text<P: crate::Parser>(
    parser: &mut P,
    ctx: &Context,
    node: &Container,
    mark_chain: &MarkChain,
) -> Result<(), String> {
    // Pass the spans to the caller of the library
    let boundaries = &node.content_boundaries;
    if boundaries.is_empty() {
        return Ok(());
    }
    let mut text_off = boundaries.first().unwrap().beg;

    let t_type = TextType::Normal;
    for mark in mark_chain {
        if !mark.is_closing {
            // Create span detail when entering
            let mut detail = SpanDetail::None;
            if (mark.s_type & (S_LINK | S_LINKDEF)) != 0 {
                if let Some(last) = mark.true_bounds.last() {
                    let start = last.end + 2;
                    let end = last.post.saturating_sub(1);
                    if start <= end && end <= ctx.text.len() {
                        let mut href = String::new();
                        let mut off = start;
                        while off < end {
                            href.push(ctx.char_at(off));
                            off += 1;
                        }
                        detail = SpanDetail::Url { href };
                    }
                }
            } else if mark.s_type == S_AUTOLINK {
                if let Some(last) = mark.true_bounds.last() {
                    let start = last.pre;
                    let end = last.end.min(ctx.text.len());
                    if start < end {
                        let mut href = String::new();
                        let mut off = start;
                        while off < end {
                            href.push(ctx.char_at(off));
                            off += 1;
                        }
                        detail = SpanDetail::Url { href };
                    }
                }
            } else if (mark.s_type & SELECT_IMGS) != 0 {
                if let Some(last) = mark.true_bounds.last() {
                    let start = last.beg;
                    let end = last.end.min(ctx.text.len());
                    let post = last.post.min(ctx.text.len());
                    let mut src = String::new();
                    let mut alt = String::new();
                    if (mark.s_type & S_IMG) != 0 {
                        let mut off = start;
                        while off < end {
                            src.push(ctx.char_at(off));
                            off += 1;
                        }
                    } else {
                        let mut off = start;
                        while off < end {
                            alt.push(ctx.char_at(off));
                            off += 1;
                        }
                        let s = end + 2;
                        let e = post.saturating_sub(1);
                        let mut off2 = s;
                        while off2 < e {
                            src.push(ctx.char_at(off2));
                            off2 += 1;
                        }
                    }
                    detail = SpanDetail::Img { src, alt };
                }
            } else if (mark.s_type & SELECT_REFS) != 0 {
                if let Some(last) = mark.true_bounds.last() {
                    let start = last.beg;
                    let end = last.end.min(ctx.text.len());
                    let mut name = String::new();
                    let mut off = start;
                    while off < end {
                        name.push(ctx.char_at(off));
                        off += 1;
                    }
                    detail = SpanDetail::Ref { name };
                }
            }

            // Insert text left to span
            if let Some(first) = mark.true_bounds.first() {
                create_text(parser, ctx, boundaries, TextType::Normal, text_off, first.pre)?;
                text_off = first.beg;
            }

            // Enter span
            let bounds_vec = mark.true_bounds.clone();
            parser.enter_span(flag_to_type(mark.s_type), &bounds_vec, &mark.attributes, &detail)?;
        } else {
            // Leave span: insert inner text then leave
            if let Some(start_idx) = mark.start_idx {
                if let Some(open) = mark_chain.get(start_idx) {
                    if let Some(bound) = open.true_bounds.last() {
                        let mut has_text = true;
                        let mut ty = TextType::Normal;
                        if mark.s_type == S_MATH {
                            ty = TextType::Math;
                        } else if mark.s_type == S_VERBATIME {
                            ty = TextType::Code;
                        } else if (mark.s_type & (SELECT_REFS | SELECT_IMGS)) != 0 {
                            has_text = false;
                        }

                        if has_text {
                            create_text(parser, ctx, boundaries, ty, text_off, bound.end)?;
                        }
                        text_off = bound.post;
                    }
                }
            }
            parser.leave_span(flag_to_type(mark.s_type))?;
        }
    }

    // Remaining text after last span
    if let Some(last_bound) = boundaries.last() {
        create_text(parser, ctx, boundaries, t_type, text_off, last_bound.end)?;
    }
    Ok(())
}

pub fn parse_spans<P: crate::Parser>(parser: &mut P, ctx: &Context, node: &Container) -> Result<(), String> {
    let mut mark_chain: MarkChain = Vec::new();

    if node.b_type != BlockType::Code && node.b_type != BlockType::Math {
        main_loop(ctx, node, &mut mark_chain);
        mark_cleanup(ctx, &mut mark_chain);
        parse_text(parser, ctx, node, &mark_chain)?;
    } else {
        let mut t_type = TextType::Normal;
        if node.b_type == BlockType::Code {
            t_type = TextType::Code;
        } else if node.b_type == BlockType::Math {
            t_type = TextType::Math;
        }
        let boundaries = &node.content_boundaries;
        let start = boundaries.first().unwrap().beg;
        let end = boundaries.last().unwrap().end;
        create_text(parser, ctx, boundaries, t_type, start, end)?;
    }

    Ok(())
}
