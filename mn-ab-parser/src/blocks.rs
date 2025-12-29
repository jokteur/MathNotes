use crate::commons::*;
use crate::definitions::*;
use crate::helpers::*;
use crate::internal::*;
use crate::spans::parse_spans;

pub const LIST_OPENER: u32 = 0x1;
pub const CODE_OPENER: u32 = 0x2;
pub const HR_OPENER: u32 = 0x4;
pub const H_OPENER: u32 = 0x8;
pub const P_OPENER: u32 = 0x10;
pub const QUOTE_OPENER: u32 = 0x20;
pub const DIV_OPENER: u32 = 0x40;
pub const DEFINITION_OPENER: u32 = 0x080;
pub const MATH_OPENER: u32 = 0x100;

/// SegmentInfo is used to determine the blocks on each line
#[derive(Debug, Clone)]
pub struct SegmentInfo {
    pub flags: u32,
    pub b_type: BlockType,
    pub b_bounds: Boundaries,
    pub start: usize,
    pub end: usize,
    pub first_non_blank: usize,
    pub indent: usize,
    pub line_number: usize,
    pub above_list_depth: i32,
    pub blank_line: bool,
    pub skip_segment: bool,
    pub acc: String,
    pub close_block: bool,
    pub no_content_after: bool,
    pub count: usize, // Repeated marker count
    pub attributes: Attributes,

    // List info
    pub li_pre_marker: char,
    pub li_post_marker: char,
}
impl Default for SegmentInfo {
    fn default() -> Self {
        Self {
            flags: 0,
            b_type: BlockType::Doc,
            b_bounds: Boundaries::default(),
            start: 0,
            end: 0,
            first_non_blank: 0,
            indent: 0,
            line_number: 0,
            above_list_depth: 0,
            blank_line: true,
            skip_segment: false,
            acc: String::new(),
            close_block: false,
            no_content_after: false,
            count: 0,
            attributes: Attributes::new(),
            li_pre_marker: '\0',
            li_post_marker: '\0',
        }
    }
}

// A few helpers functions
#[inline]
fn check_ws_before(seg: &SegmentInfo, off: Offset) -> bool {
    seg.first_non_blank >= off
}
#[inline]
fn check_space_after(ctx: &Context, off: Offset) -> bool {
    if off >= ctx.text.len() {
        return true;
    }
    let ch = ctx.char_at(off + 1);
    ch == ' '
}

#[inline]
fn analyse_make_p(off: usize, end: &mut usize, seg: &mut SegmentInfo) {
    seg.attributes.clear();
    seg.b_bounds.pre = off;
    seg.b_bounds.beg = off;
    seg.flags = P_OPENER;
    seg.b_type = BlockType::P;
    *end = seg.end;
}
#[inline]
fn analyse_make_ul(ctx: &Context, off: usize, end: &mut usize, seg: &mut SegmentInfo, indent: usize) {
    seg.b_bounds.pre = seg.start;
    seg.b_bounds.beg = if off + 1 == seg.end { off + 1 } else { off + 2 };
    seg.indent = off + 2 - seg.start + indent;
    *end = if check_space_after(ctx, off) { off + 2 } else { off + 1 };
    seg.flags = LIST_OPENER;
    seg.b_type = BlockType::Ul;
    seg.li_pre_marker = ctx.char_at(off);
}
#[inline]
fn get_allowed_ws(flag: u32) -> isize {
    if (flag & (QUOTE_OPENER | DEFINITION_OPENER)) != 0 {
        1
    } else {
        3
    }
}
fn check_for_whitespace_after(ctx: &Context, off: Offset) -> bool {
    let mut off = off;
    while (off < ctx.text.len() && ctx.char_at(off) != '\n') {
        if !ctx.char_at(off).is_whitespace() {
            return false;
        }
        off += 1;
    }
    return false;
}

fn get_name_and_attributes(ctx: &Context, off: &mut usize) -> (Attributes, String) {
    let mut attributes = Attributes::new();
    let mut name = String::new();
    while *off < ctx.text.len() && ctx.char_at(*off) != '\n' {
        let ch = ctx.char_at(*off);
        if ch == '{' && *off + 1 < ctx.text.len() && ctx.char_at(*off + 1) == '{' {
            *off += 2;
            attributes = parse_attributes(ctx, off);
            break;
        } else if ch.is_whitespace() {
            *off += 1;
            continue;
        } else {
            name.push(ch);
            *off += 1;
        }
    }
    (attributes, name)
}
/// Checks at off if there are closing delimiters given the set of rules in the arguments
///
/// It optionally also parses the attributes
///
/// Returns the number of closing delimiters found
/// If after the closing delimiters there are non-authorised chars, then it returns None
#[inline]
fn check_for_closing_delimiters(
    ctx: &Context,
    off: &mut Offset,
    seg: &mut SegmentInfo,
    marker: char,
    num_markers: usize,
    allow_greater_number: bool,
    allow_chars_before_closing: bool,
    allow_attribute: bool,
) -> Option<i32> {
    let check_ws = allow_chars_before_closing || (!allow_chars_before_closing && check_ws_before(seg, *off));

    let mut count = 0;
    while *off < ctx.text.len() && ctx.char_at(*off) != '\n' {
        let ch = ctx.char_at(*off);
        if ch == '\\' {
            count = -1;
            *off += 1; // Consume backslash
        } else if ch == marker {
            count += 1;
        } else if count < num_markers as i32 {
            count = 0;
        } else {
            break;
        }
        *off += 1;
    }

    let is_count_right = if allow_greater_number {
        count >= num_markers as i32
    } else {
        count == num_markers as i32
    };
    let mut non_auth_text_after = false;

    let mut tmp_off = *off;
    skip_whitespace(ctx, &mut tmp_off);

    if tmp_off < ctx.text.len() && ctx.char_at(tmp_off) == '{' && allow_attribute {
        tmp_off += 1;
        if tmp_off < ctx.text.len() && ctx.char_at(tmp_off) == '{' {
            tmp_off += 1;
            seg.attributes = parse_attributes(ctx, &mut tmp_off);
            if seg.attributes.is_empty() {
                non_auth_text_after = true;
            }
        } else {
            non_auth_text_after = true;
        }
    } else if tmp_off < ctx.text.len() && ctx.char_at(tmp_off) != '\n' {
        non_auth_text_after = true;
    }

    if is_count_right && check_ws && !non_auth_text_after {
        Some(count)
    } else if non_auth_text_after {
        None
    } else {
        Some(0)
    }
}

/// Returns SegmentInfo and the offset for the next segment
fn analyse_segment(ctx: &mut Context, off: Offset) -> (SegmentInfo, Offset) {
    let mut off = off;
    let mut seg = SegmentInfo::default();
    seg.end = ctx.find_next_line_offset(off);
    seg.line_number = ctx.find_line_number(off);
    let mut this_segment_end = seg.end;
    let mut above_container = ctx.above_container;

    seg.start = off;
    seg.first_non_blank = seg.end; // Unless otherwise, the first blank is defined at the end of the line
    seg.b_bounds.pre = off;
    seg.b_bounds.beg = off;
    seg.blank_line = true; // Unless otherwise, the default line is a blank line

    let mut repeated_marker = RepeatedMarker::default();

    let mut local_indent = 0;
    let mut total_indent = 0;

    if let Some(above_id) = above_container {
        let above = ctx.get_node(above_id);
        local_indent = above.indent;
        total_indent = local_indent;
        if above.repeated_marker.is_some() && !above.closed {
            repeated_marker = above.repeated_marker.clone().unwrap();
        }

        // If above is a block which has open/close delimiters (repeating markers),
        // then all standard detection rules should be ignored and one should only
        // look to closing delimiters.
        // We already know that the segment will be of the flag of above
        if repeated_marker.marker != '\0' && !above.closed {
            seg.flags = above.flag;
            seg.blank_line = false;
            seg.b_bounds.end = seg.end;
            seg.b_bounds.post = seg.end;
        }
    }

    let mut whitespace_counter = 0;
    let mut acc = String::new(); // Accumulator

    #[derive(PartialEq)]
    enum Solved {
        NONE,
        PARTIAL,
        FULL,
    }
    let mut b_solved = Solved::NONE;

    while off < seg.end {
        let ch = ctx.char_at(off);
        acc.push(ch);

        /* Indent is useful for knowing when to move above_container (see explanations
            * in process_segment) in the case of lists or definitions. Here is an example:
            *
            * - > abc
            *   > def
            * ^
        * cursor pos
                    seg->b_bounds.end = off + num_markers;
                    seg->b_bounds.post = off + num_markers;
            *
            * We are on the second line. above_container points to the LI, which has an
            * indent of 2 (you need two whitespace characters before the block to be part
            * of the LI). So if we detect enough whitespace characters before the block `>`,
            * then we move the above_container to the child of the LI, i.e. QUOTE. This way,
            * in process_segment(), the QUOTE can be continued as part of the LI.
            * */
        /* It means that there is enough indent to be part of the LI or DEF */
        if let Some(above_id) = above_container {
            let above = ctx.get_node_mut(above_id);
            if local_indent > 0 && seg.blank_line && whitespace_counter >= local_indent && !above.closed {
                above.content_boundaries.push(Boundaries {
                    line_number: seg.line_number,
                    pre: seg.start,
                    beg: off,
                    end: seg.end,
                    post: seg.end,
                });
                if above.b_type == BlockType::Li {
                    if let Some(parent_id) = above.parent {
                        let parent = ctx.get_node_mut(parent_id);
                        parent.content_boundaries.push(Boundaries {
                            line_number: seg.line_number,
                            pre: seg.start,
                            beg: seg.start,
                            end: seg.end,
                            post: seg.end,
                        });
                    }
                }
                seg.start = off;
                ctx.select_last_child_container();
                seg.above_list_depth += 1;
                above_container = ctx.above_container;
                if let Some(new_above_id) = above_container {
                    let new_above = ctx.get_node(new_above_id);
                    local_indent = new_above.indent;
                    total_indent += local_indent;
                    repeated_marker = new_above.repeated_marker.clone().unwrap_or_default();
                }
            }
        }

        if !ch.is_whitespace() && seg.blank_line {
            seg.blank_line = false;
            seg.first_non_blank = off;
            acc.clear();
            acc.push(ch);
        }

        let ws_diff = whitespace_counter as isize - total_indent as isize;

        if ch == ' ' {
            whitespace_counter += 1;
        } else if ch == '\t' {
            whitespace_counter += 4;
        } else if ch == '\\' {
            if off == seg.start && repeated_marker.marker == '\0' {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
            off += 1; // skip next char
        } else if repeated_marker.marker != '\0' {
            if ch == repeated_marker.marker {
                /* Try to see if the block is closed with pre-defined rules */
                let off_before = off;
                let num = check_for_closing_delimiters(
                    ctx,
                    &mut off,
                    &mut seg,
                    repeated_marker.marker,
                    repeated_marker.count,
                    repeated_marker.allow_greater_number,
                    repeated_marker.allow_chars_before_closing,
                    repeated_marker.allow_attribute,
                );
                if num.is_some() && num.unwrap() > 0 {
                    //TODO
                    seg.close_block = true;
                    seg.flags = above_container.and_then(|id| Some(ctx.get_node(id).flag)).unwrap_or(0);
                    seg.b_bounds.end = off_before;
                    seg.b_bounds.post = off;
                    break;
                }
            }
        } else if ch == '#' {
            let count = count_marks(ctx, off, '#');
            if check_ws_before(&seg, off)
                && count > 0
                && count < 7
                && check_ws_or_end(ctx, off + count)
                && (ws_diff < 3)
            {
                seg.flags = H_OPENER;
                seg.b_bounds.pre = seg.start;
                seg.b_bounds.beg = off + count;
                seg.b_type = BlockType::H;
                b_solved = Solved::FULL;
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '>' {
            if check_ws_before(&seg, off) && (ws_diff < get_allowed_ws(QUOTE_OPENER)) {
                seg.b_bounds.pre = seg.start;
                seg.b_bounds.beg = off + 1;
                seg.flags = QUOTE_OPENER;
                this_segment_end = off + 1;
                seg.b_type = BlockType::Quote;
                b_solved = Solved::FULL;
                if off + 1 < ctx.text.len() && ctx.char_at(off + 1) == ' ' {
                    seg.b_bounds.beg = off + 2;
                    this_segment_end = off + 2;
                }
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '*' {
            if check_ws_before(&seg, off) && check_ws_or_end(ctx, off + 1) && (seg.flags & LIST_OPENER) == 0 {
                analyse_make_ul(ctx, off, &mut this_segment_end, &mut seg, whitespace_counter);
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '-' {
            let count = count_marks(ctx, off, '-');
            if check_ws_before(&seg, off) && count > 2 && check_for_whitespace_after(ctx, off + count) {
                seg.flags = HR_OPENER;
                seg.b_bounds.pre = off;
                seg.b_bounds.beg = off + count;
                this_segment_end = seg.end;
                seg.b_type = BlockType::Hr;
                b_solved = Solved::FULL;
                break;
            } else if check_ws_before(&seg, off)
                && (off + 1 >= ctx.text.len() || ctx.char_at(off + 1).is_whitespace())
                && ((seg.flags & LIST_OPENER) == 0)
                && (ws_diff < get_allowed_ws(LIST_OPENER))
            {
                analyse_make_ul(ctx, off, &mut this_segment_end, &mut seg, whitespace_counter);
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '+' {
            if check_ws_before(&seg, off)
                && (off + 1 >= ctx.text.len() || ctx.char_at(off + 1).is_whitespace())
                && (seg.flags & LIST_OPENER) == 0
                && (ws_diff < get_allowed_ws(LIST_OPENER))
            {
                analyse_make_ul(ctx, off, &mut this_segment_end, &mut seg, whitespace_counter);
                break;
            }
        } else if ch == '(' {
            if (seg.flags & LIST_OPENER) != 0 || !check_ws_before(&seg, off) {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
            b_solved = Solved::PARTIAL;
            seg.acc.clear();
            seg.flags |= LIST_OPENER;
            seg.li_pre_marker = '(';
        } else if ch == ')' || ch == '.' {
            // Check if we are forming an ordered list "1." "a)" etc.
            // Acc has the number/letter.
            let str_marker = if seg.acc.len() > 0 {
                &seg.acc[..seg.acc.len() - 1]
            } else {
                ""
            };

            if !str_marker.is_empty()
                && str_marker.len() < 12
                && check_ws_or_end(ctx, off + 1)
                && !(seg.li_pre_marker == '(' && ch == '.')
            {
                seg.b_bounds.pre = seg.start;
                seg.b_bounds.beg = off + 1;
                seg.indent = off + 2 - seg.start + whitespace_counter;
                this_segment_end = off + 2;
                if off + 1 < ctx.text.len() && ctx.char_at(off + 1) == ' ' {
                    seg.b_bounds.beg += 1;
                }
                seg.flags = LIST_OPENER;
                b_solved = Solved::PARTIAL;
                seg.li_post_marker = ch;
                acc = str_marker.to_string();
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '[' {
            let mut is_parent_root = false;
            if let Some(above_id) = above_container {
                let above = ctx.get_node(above_id);
                if let Some(parent_id) = above.parent {
                    let parent = ctx.get_node(parent_id);
                    if parent.b_type == BlockType::Doc {
                        is_parent_root = true;
                    }
                }
            }
            if ws_diff >= get_allowed_ws(DEFINITION_OPENER) || (above_container.is_some() && !is_parent_root) {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
            // Definition check
            let start_off = off;
            seg.acc.clear();
            let found_end = advance_until(ctx, &mut off, &mut seg.acc, ']');
            if !found_end || off + 1 >= ctx.text.len() || ctx.char_at(off + 1) != ':' || off - start_off < 2 {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            } else {
                seg.flags = DEFINITION_OPENER;
                b_solved = Solved::FULL;
                seg.indent = 4 + whitespace_counter;
                seg.b_bounds.pre = seg.start;
                seg.b_bounds.beg = off + 2;
                seg.b_bounds.end = off + 2;
                seg.b_bounds.post = off + 2;
                this_segment_end = off + 2;
                // acc already filled by advance_until
                break;
            }
        } else if ch == ':' {
            let before_off = off;
            let count = count_marks(ctx, off, ':');
            off += count;
            skip_whitespace(ctx, &mut off);

            if check_ws_before(&seg, before_off) && count == 3 && off < seg.end {
                seg.flags = DIV_OPENER;
                seg.b_bounds.pre = seg.start;
                seg.b_bounds.beg = seg.end;
                seg.b_bounds.end = seg.end;
                seg.b_bounds.post = seg.end;
                seg.indent = 4 + whitespace_counter;
                this_segment_end = seg.end;
                seg.acc.clear();
                // Move off to start of name
                off += count;
                let (attributes, name) = get_name_and_attributes(ctx, &mut off);
                seg.acc = name;
                seg.attributes = attributes;
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '$' {
            let tmp_off = off;
            let count = count_marks(ctx, off, '$');
            off += count;
            // Try to advance until the end of the line to see if the block is already closed
            advance_until(ctx, &mut off, &mut acc, '$');
            let closing = check_for_closing_delimiters(ctx, &mut off, &mut seg, '$', 2, false, true, true);

            if check_ws_before(&seg, tmp_off) && count == 2 && closing.is_some() {
                seg.flags = MATH_OPENER;
                seg.b_bounds.beg = tmp_off + count;
                seg.b_bounds.end = seg.end;
                seg.b_bounds.post = seg.end;
                if closing.unwrap() > 0 {
                    seg.close_block = true;
                    seg.b_bounds.end = off - closing.unwrap() as usize;
                }
                off += 1;
                break;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        } else if ch == '`' {
            let count = count_marks(ctx, off, '`');
            if check_ws_before(&seg, off) && count > 2 && (ws_diff < get_allowed_ws(CODE_OPENER)) {
                seg.flags = CODE_OPENER;
                seg.b_bounds.beg = seg.end;
                seg.b_bounds.end = seg.end;
                seg.b_bounds.post = seg.end;
                seg.count = count;
                off += count;
                seg.acc.clear();
                let (attributes, name) = get_name_and_attributes(ctx, &mut off);
                seg.acc = name;
                seg.attributes = attributes;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
                break;
            }
        }

        off += 1;
    }

    if !seg.blank_line && seg.flags == 0 {
        analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
    }
    if whitespace_counter < local_indent
        && seg.first_non_blank as isize - seg.start as isize > get_allowed_ws(seg.flags)
    {
        analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
    }
    // Some blank lines can be transformed into boundaries of indented blocks (LI, DEF)
    if seg.blank_line && whitespace_counter <= local_indent {
        if let Some(above_id) = above_container {
            let above = ctx.get_node_mut(above_id);
            let b_type = above.b_type;
            if b_type == BlockType::Li || b_type == BlockType::Def || b_type == BlockType::Div {
                above.content_boundaries.push(Boundaries {
                    line_number: seg.line_number,
                    pre: seg.start,
                    beg: seg.end,
                    end: seg.end,
                    post: seg.end,
                });
                if b_type == BlockType::Li {
                    if let Some(parent_id) = above.parent {
                        let parent = ctx.get_node_mut(parent_id);
                        parent.content_boundaries.push(Boundaries {
                            line_number: seg.line_number,
                            pre: seg.start,
                            beg: seg.start,
                            end: seg.end,
                            post: seg.end,
                        });
                    }
                }
                seg.skip_segment = true;
            }
        }
    }

    // If potential list has been detected, verify if the enumeration makes sense
    if seg.flags & LIST_OPENER != 0 && b_solved == Solved::PARTIAL {
        if seg.li_pre_marker == '(' && seg.li_post_marker != ')' {
            analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
        } else {
            /* Still need to verify if ordered list has valid enumeration
             * Example: 'IM' is not a valid roman value */
            let is_numeric = verify_positive_number(&seg.acc) && seg.acc.len() < 10;
            let is_roman = validate_roman_enumeration(&seg.acc);
            let is_alpha = alpha_to_decimal(&seg.acc) > 0 && seg.acc.len() < 4;

            if is_numeric || is_roman || is_alpha {
                b_solved = Solved::FULL;
                seg.b_type = BlockType::Ol;
            } else {
                analyse_make_p(seg.start, &mut this_segment_end, &mut seg);
            }
        }
    }

    if seg.flags & LIST_OPENER != 0 && seg.b_bounds.beg == seg.end {
        seg.no_content_after = true;
    }

    //println!("Analysed segment: {:?} ", seg);

    (seg, this_segment_end)
}

// Processing

fn enter_block<P: crate::Parser>(parser: &mut P, ctx: &Context, container: &Container) -> Result<(), String> {
    parser.enter_block(
        container.b_type,
        &container.content_boundaries,
        &container.attributes,
        &container.detail,
    )?;

    for &child_id in &container.children {
        let child = ctx.get_node(child_id);
        if child.b_type == BlockType::Empty {
            continue;
        }
        enter_block(parser, ctx, child)?;
    }
    if is_leaf_block(container.b_type) {
        parse_spans(parser, ctx, container)?;
    }
    parser.leave_block(container.b_type)?;
    Ok(())
}

pub fn add_container(
    ctx: &mut Context,
    b_type: BlockType,
    bounds: Vec<Boundaries>,
    seg: &SegmentInfo,
    detail: BlockDetail,
) -> ContainerId {
    let new_id = ctx.request_id();

    let parent_id = ctx.current_container;

    let mut new_node = Container {
        id: new_id,
        b_type,
        children: vec![],
        parent: Some(parent_id),
        content_boundaries: bounds,
        detail,
        attributes: seg.attributes.clone(),
        closed: false,
        erase_block: false,
        repeated_marker: None,
        last_non_empty_child_line: None,
        indent: seg.indent,
        flag: seg.flags,
    };
    if b_type == BlockType::Empty {
        new_node.closed = true;
    }
    if b_type != BlockType::Hidden {
        let parent = ctx.get_node_mut(parent_id);
        parent.last_non_empty_child_line = Some(seg.line_number);
    }

    //println!("Added container: {:?} with id {}", new_node, new_id);
    //println!(
    //     "Current container: {:?}, Above: {:?}",
    //     ctx.current_container, ctx.above_container
    // );

    ctx.nodes.push(new_node);
    ctx.get_node_mut(parent_id).children.push(new_id);
    ctx.current_container = new_id;
    return new_id;
}

fn close_current_container(ctx: &mut Context) {
    let node = &mut ctx.get_node_mut(ctx.current_container);
    node.closed = true;
    if node.parent.is_some() {
        ctx.current_container = node.parent.unwrap();
    }
}

fn send_previous_blocks<P: crate::Parser>(ctx: &mut Context, parser: &mut P) -> Result<(), String> {
    let root_children = &ctx.get_node(0).children;
    for child in root_children {
        enter_block(parser, ctx, ctx.get_node(*child))?;
    }

    // Clear children of root
    ctx.get_node_mut(0).children.clear();
    ctx.above_container = Some(0);

    // Todo, reset memory?
    Ok(())
}

fn select_parent(ctx: &Context, id: Offset) -> Option<ContainerId> {
    let node = ctx.get_node(id);
    let parent_id = node.parent;
    if let Some(parent_id) = parent_id {
        let parent = ctx.get_node(parent_id);
        if parent.b_type == BlockType::Ul || parent.b_type == BlockType::Ol {
            let grandparent_id = parent.parent;
            if let Some(grandparent_id) = grandparent_id {
                Some(grandparent_id)
            } else {
                None
            }
        } else {
            Some(parent_id)
        }
    } else {
        None
    }
}

fn make_list_item(ctx: &mut Context, seg: &SegmentInfo, off: Offset) -> bool {
    let above_container = ctx.above_container;

    let is_ul = seg.acc.is_empty();
    let pre_marker = seg.li_pre_marker;
    let post_marker = seg.li_post_marker;
    let above_parent = if let Some(above_id) = above_container {
        ctx.get_node(above_id).parent
    } else {
        None
    };
    let is_above_ol = if let Some(above_id) = above_container {
        ctx.get_node(above_id).b_type == BlockType::Ol
    } else {
        false
    };
    let is_above_ul = if let Some(above_id) = above_container {
        ctx.get_node(above_id).b_type == BlockType::Ul
    } else {
        false
    };

    let mut ol_type = OlType::Numeric;
    let mut alpha = -1;
    let mut roman = -1;
    if (!is_ul) {
        alpha = alpha_to_decimal(&seg.acc) as i32;
        roman = roman_to_decimal(&seg.acc) as i32;
        if verify_positive_number(&seg.acc) {
            ol_type = OlType::Numeric;
        }
        /* With this simple rule, we can decide between cases that are valid in both roman
         * and alpha case, e.g. 'i)' */
        else if alpha > 0 && roman > 0 {
            ol_type = if alpha < roman {
                OlType::Alphabetic
            } else {
                OlType::Roman
            };
        } else if roman > 0 {
            ol_type = OlType::Roman;
        } else if alpha > 0 {
            ol_type = OlType::Alphabetic;
        }
    }

    let mut make_new_list = false;
    /* If above at the same level there is not list, then we must
     * create a new list */
    if !is_above_ul && !is_above_ol {
        make_new_list = true;
    } else if is_above_ul {
        if let Some(above_parent_id) = above_parent {
            let parent = ctx.get_node(above_parent_id);
            let detail_marker = if let BlockDetail::Ul { marker } = &parent.detail {
                *marker
            } else {
                '\0'
            };
            /* Even there is an above list, if the marker don't match then a new list
             * is still created */
            if detail_marker != pre_marker {
                make_new_list = true;
            }
        }
    } else if is_above_ol && !is_ul {
        if let Some(above_parent_id) = above_parent {
            let parent = ctx.get_node(above_parent_id);
            if let BlockDetail::Ol {
                pre_marker,
                post_marker,
                lower_case: _,
                ol_type: parent_type,
                ..
            } = &parent.detail
            {
                if *pre_marker != seg.li_pre_marker || *post_marker != seg.li_post_marker {
                    make_new_list = true;
                }
                /* By default, we choose the enumeration type of the one that is lowest in decimal
                 * However, if we are already in a list that is either alpha or roman, then the
                 * current list item must inherit the alpha or roman property */
                if ol_type != OlType::Numeric {
                    if *parent_type == OlType::Alphabetic && roman > 0 && alpha > 0 {
                        ol_type = OlType::Alphabetic;
                    } else if *parent_type == OlType::Roman && roman > 0 && alpha > 0 {
                        ol_type = OlType::Roman;
                    }
                }
                if ol_type != *parent_type {
                    make_new_list = true;
                }
            }
        }
    } else if is_above_ol && is_ul {
        make_new_list = true;
    }

    /* If above we have a list (at the same level at the current),
     * we need to close the last inserted list element */
    if is_above_ul || is_above_ol {
        ctx.get_node_mut(ctx.current_container).closed = true;
        if let Some(above_id) = above_container {
            let children = ctx.get_node(above_id).children.clone();
            for child in children {
                ctx.get_node_mut(child).closed = true;
            }

            let last_line = ctx.get_node(above_id).last_non_empty_child_line.unwrap();
            if seg.line_number as i32 - last_line as i32 > 2 {
                make_new_list = true;
            }

            let parent_id = ctx.get_node(above_id).parent.unwrap();
            if make_new_list {
                ctx.current_container = ctx.get_node(parent_id).parent.unwrap();
            } else {
                ctx.current_container = parent_id;
            }
        }
    }

    let list_bounds = Boundaries {
        line_number: seg.line_number,
        pre: seg.b_bounds.pre,
        beg: seg.b_bounds.pre,
        end: seg.end,
        post: seg.end,
    };

    if make_new_list {
        if is_ul {
            let detail = BlockDetail::Ul {
                marker: seg.li_pre_marker,
            };
            add_container(ctx, BlockType::Ul, vec![list_bounds], seg, detail);
        } else {
            let detail = BlockDetail::Ol {
                pre_marker: seg.li_pre_marker,
                post_marker: seg.li_post_marker,
                ol_type: ol_type.clone(),
                lower_case: seg.b_bounds.beg < ctx.text.len() && ctx.char_at(seg.b_bounds.beg).is_lowercase(),
            };
            add_container(ctx, BlockType::Ol, vec![list_bounds], seg, detail);
        }
    } else {
        // Append boundaries to existing list
        let curr = ctx.current_container;
        ctx.get_node_mut(curr).content_boundaries.push(list_bounds);
    }

    // Add LI
    let li_detail = BlockDetail::Li {
        number: if !is_ul { seg.acc.clone() } else { String::new() },
        is_task: false,
        task_state: None, // To be implemented
        level: 0,
    };
    let li_bounds = Boundaries {
        line_number: seg.line_number,
        pre: seg.b_bounds.pre,
        beg: seg.b_bounds.beg,
        end: seg.end,
        post: seg.end,
    };
    add_container(ctx, BlockType::Li, vec![li_bounds], seg, li_detail);

    if seg.no_content_after {
        add_container(
            ctx,
            BlockType::Empty,
            vec![Boundaries::default()],
            seg,
            BlockDetail::None,
        );
    }

    ctx.above_container = None;
    true
}

/// Once a segment of a line has been analysed by the analyse_segment
/// function, we need to decide were to place the block in the AST.
/// This process is context dependent.
fn process_segment<P: crate::Parser>(
    ctx: &mut Context,
    parser: &mut P,
    off: &Offset,
    seg: &mut SegmentInfo,
) -> Result<(), String> {
    if seg.skip_segment {
        return Ok(());
    }

    /* Above container is the critical part to take decisions on how the place
     * the newly analysed block. This pointer points to where we would be if we
     * are looking at the same level on the above line.
     * Here is an example:
     *
     *     > - item1
     *     > abc
     *     ^
     *  cursor pos
     *
     * We are on the second line. We just detected a quote character. above_container
     * points to the QUOTE of the previous line. This way we know we have to prolong
     * the QUOTE and not create a new one. For the next loop, we use select_last_child_container()
     * so the above pointer will point to the next block in the above line, i.e. list item (LI).
     *
     *     > - item1
     *     > abc
     *       ^
     *   cursor pos
     *
     * On the second pass, we detected a paragraph. However, as above we point to the LI,
     * we know that it should be closed and that the paragraph should be placed on the same
     * level as the UL containing the LI.
     *
     * There is a second pointer called ctx->current_container. Whenever add_container() is
     * called, the new block is insert as a child to ctx->current_container. This pointer
     * closely follows above_container, until above_container is set to nullptr, in which
     * case current_container always points to the last inserted block.
     */
    let mut above_container_id = ctx.above_container;
    let mut set_above_to_null = false;
    let mut line_number_diff = 0;

    if let Some(above_id) = above_container_id {
        let node = ctx.get_node(above_id);
        if node.b_type != BlockType::Doc {
            let last_bound = node.content_boundaries.last().unwrap();
            line_number_diff = seg.line_number as i32 - last_bound.line_number as i32;

            let flag = node.flag;
            let type_check = (flag & (DEFINITION_OPENER | DIV_OPENER)) != 0;
            let node_b_type = node.b_type;
            let node_parent = node.parent;

            if line_number_diff > 1 || flag != seg.flags || type_check {
                close_current_container(ctx);
                if node_b_type == BlockType::Li {
                    close_current_container(ctx);
                }
                set_above_to_null = true;

                let parent = node_parent.unwrap();
                if ctx.get_node(parent).b_type == BlockType::Doc && !seg.blank_line {
                    send_previous_blocks(ctx, parser)?;
                }
            }
        }
    }

    if seg.blank_line {
        let mut parent_id = 0; // By default, blank lines belong to ROOT
        if let Some(id) = above_container_id {
            /* Blank lines should always be commited to parent above container */
            parent_id = select_parent(&ctx, id).unwrap();
        }
        ctx.current_container = parent_id;

        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.start,
            beg: seg.start,
            end: seg.end,
            post: seg.end,
        };
        add_container(ctx, BlockType::Hidden, vec![b], seg, BlockDetail::None);
        close_current_container(ctx);
    }

    if set_above_to_null {
        ctx.above_container = None;
        above_container_id = None;
    }

    let is_block_continued = |t: BlockType| -> bool {
        if let Some(id) = above_container_id {
            ctx.get_node(id).b_type == t && line_number_diff > 0
        } else {
            false
        }
    };

    // Dispatch based on flags
    if (seg.flags & P_OPENER) != 0 {
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.start,
            beg: seg.first_non_blank,
            end: seg.end,
            post: seg.end,
        };
        if is_block_continued(BlockType::P) {
            ctx.get_node_mut(above_container_id.unwrap()).content_boundaries.push(b);
        } else {
            add_container(ctx, BlockType::P, vec![b], seg, BlockDetail::None);
        }
    } else if (seg.flags & HR_OPENER) != 0 {
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.start,
            beg: seg.start,
            end: seg.end,
            post: seg.end,
        };
        add_container(ctx, BlockType::Hr, vec![b], seg, BlockDetail::None);
    } else if (seg.flags & H_OPENER) != 0 {
        let mut new_header = true;
        let level = (seg.b_bounds.beg - seg.b_bounds.pre) as u8;
        /* Headers can be empty, e.g. `##`
         * In this case, the mandatory space after is not taken into account
         * When there is the mandatory space, b_beg should begin one char after */
        let mut beg = seg.b_bounds.beg;
        if beg < seg.end {
            beg += 1;
        }

        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.end,
            post: seg.end,
        };

        if is_block_continued(BlockType::H) {
            if let BlockDetail::H { level: old_level } = ctx.get_node(above_container_id.unwrap()).detail {
                if old_level == level {
                    new_header = false;
                    ctx.get_node_mut(above_container_id.unwrap())
                        .content_boundaries
                        .push(b.clone());
                } else {
                    close_current_container(ctx);
                }
            }
        }

        if new_header {
            add_container(ctx, BlockType::H, vec![b], seg, BlockDetail::H { level });
        }
    } else if (seg.flags & QUOTE_OPENER) != 0 {
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.end,
            post: seg.end,
        };
        if is_block_continued(BlockType::Quote) {
            ctx.get_node_mut(above_container_id.unwrap()).content_boundaries.push(b);
        } else {
            add_container(ctx, BlockType::Quote, vec![b], seg, BlockDetail::None);
        }
    } else if (seg.flags & DEFINITION_OPENER) != 0 {
        let def_type = if seg.acc.starts_with('^') {
            DefType::Footnote
        } else if seg.acc.starts_with("c:") {
            DefType::Citation
        } else {
            DefType::Link
        };
        let detail = BlockDetail::Def {
            name: seg.acc.clone(),
            def_type,
        };
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.b_bounds.end,
            post: seg.b_bounds.post,
        };
        add_container(ctx, BlockType::Def, vec![b], seg, detail);
    } else if (seg.flags & LIST_OPENER) != 0 {
        make_list_item(ctx, seg, *off);
    } else if (seg.flags & DIV_OPENER) != 0 {
        let detail = BlockDetail::Div { name: seg.acc.clone() };
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.b_bounds.end,
            post: seg.b_bounds.post,
        };
        add_container(ctx, BlockType::Div, vec![b], seg, detail);
        seg.flags = 0;
        let b = Boundaries {
            line_number: seg.line_number,
            ..Default::default()
        };
        add_container(ctx, BlockType::Empty, vec![b], seg, BlockDetail::None);
    } else if (seg.flags & MATH_OPENER) != 0 {
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.b_bounds.end,
            post: seg.b_bounds.post,
        };
        if is_block_continued(BlockType::Math) {
            let node = ctx.get_node_mut(ctx.current_container);
            node.content_boundaries.push(b);
            node.attributes = seg.attributes.clone();
        } else {
            add_container(ctx, BlockType::Math, vec![b], seg, BlockDetail::None);
            let node = ctx.get_node_mut(ctx.current_container);
            node.repeated_marker = Some(RepeatedMarker {
                marker: '$',
                count: 2,
                allow_greater_number: true,
                allow_chars_before_closing: true,
                allow_attribute: true,
            });
        }
        if seg.close_block {
            ctx.get_node_mut(ctx.current_container).closed = true;
        }
    } else if (seg.flags & CODE_OPENER) != 0 {
        let b = Boundaries {
            line_number: seg.line_number,
            pre: seg.b_bounds.pre,
            beg: seg.b_bounds.beg,
            end: seg.b_bounds.end,
            post: seg.b_bounds.post,
        };
        if is_block_continued(BlockType::Code) {
            ctx.get_node_mut(ctx.current_container).content_boundaries.push(b);
        } else {
            let detail = BlockDetail::Code {
                lang: seg.acc.clone(),
                num_ticks: seg.count as i32,
            };
            add_container(ctx, BlockType::Code, vec![b], seg, detail);
            let node = ctx.get_node_mut(ctx.current_container);
            node.repeated_marker = Some(RepeatedMarker {
                marker: '`',
                count: seg.count,
                allow_greater_number: false,
                allow_chars_before_closing: false,
                allow_attribute: false,
            });
            // other repeated_marker fields default to false
        }
        if seg.close_block {
            ctx.get_node_mut(ctx.current_container).closed = true;
        }
    }

    Ok(())
}

pub fn parse_blocks<P: crate::Parser>(ctx: &mut Context, parser: &mut P) -> Result<(), String> {
    let root_b_type = ctx.get_node(0).b_type;
    let root_content_boundaries = ctx.get_node(0).content_boundaries.clone();
    let root_attributes = ctx.get_node(0).attributes.clone();
    let root_detail = ctx.get_node(0).detail.clone();

    let mut off = 0;

    parser.enter_block(root_b_type, &root_content_boundaries, &root_attributes, &root_detail)?;

    while off < ctx.text.len() {
        ctx.select_last_child_container();
        let (mut seg, end) = analyse_segment(ctx, off);
        off = end;
        process_segment(ctx, parser, &mut off, &mut seg)?;

        // We arrived at the end of a line
        if off >= seg.end {
            ctx.above_container = Some(0);
            ctx.current_container = ctx.above_container.unwrap();
            off += 1;
        }
        //println!(
        //     "Endofloop: off={}, above={:?}, current={:?}",
        //     off, ctx.above_container, ctx.current_container
        // );
    }
    send_previous_blocks(ctx, parser)?;
    parser.leave_block(root_b_type)?;

    Ok(())
}
