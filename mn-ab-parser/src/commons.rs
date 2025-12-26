use crate::definitions::*;
use crate::internal::Context;

#[inline]
pub fn is_whitespace_not_eol(c: char) -> bool {
    c.is_whitespace() && c != '\n'
}

/// Parse attributes in the form of {{key1:value1,key2:value2,key3}}
pub fn parse_attributes(ctx: &Context, off: &mut Offset) -> Attributes {
    let mut attributes = Attributes::new();
    let mut start_collection = false;
    let mut is_key = true;
    let mut is_complete = false;
    let mut acc = String::new();
    let mut prev_key = String::new();


    while *off < ctx.text.len() && ctx.char_at(*off) != '\n' {
        let ch = ctx.char_at(*off);
        if ch == '\\' {
            *off += 1;
            continue;
        }
        if ch == '}' {
            if is_key {
                attributes.insert(acc.clone(), String::new());
            } else {
                attributes.insert(prev_key.clone(), acc.clone());
            }

            if start_collection {
                is_complete = true;
            }
            break;
        }

        if !start_collection && (ch == ':' || ch == '=') {
            start_collection = true;
        }

        if ch == ',' {
            if is_key {
                attributes.insert(acc.clone(), String::new());
            } else {
                attributes.insert(prev_key.clone(), acc.clone());
            }
            is_key = true;
            prev_key.clear();
            acc.clear();
            *off += 1;
            continue;
        } else if ch == ':' || ch == '=' {
            is_key = false;
            attributes.insert(acc.clone(), String::new());
            prev_key = acc.clone();
            acc.clear();
            *off += 1;
            continue;
        } else if is_whitespace_not_eol(ch) && is_key {
            *off += 1;
            continue;
        } else {
            acc.push(ch);
        }

        *off += 1;
    }

    if !is_complete {
        attributes.clear();
    }

    attributes
}

/// Counts the number of repeating markers from the given offset
pub fn count_marks(ctx: &Context, off: Offset, mark: char) -> usize {
    let mut counter = 0;
    let mut off = off;
    while ctx.char_at(off) != '\n' && off < ctx.text.len() {
        if ctx.char_at(off) == mark {
            counter += 1;
            off += 1;
        } else {
            break;
        }
    }
    counter
}

// Overloaded version taking offset by value
pub fn count_marks_val(ctx: &Context, mut off: Offset, mark: char) -> usize {
    let mut counter = 0;
    while ctx.char_at(off) != '\n' && off < ctx.text.len() {
        if ctx.char_at(off) == mark {
            counter += 1;
            off += 1;
        } else {
            break;
        }
    }
    counter
}

pub fn advance_until(ctx: &Context, off: &mut Offset, acc: &mut String, ch: char) -> bool {
    let mut found_end_char = false;
    while *off < ctx.text.len() && ctx.char_at(*off) != '\n' {
        let c = ctx.char_at(*off);
        if c == '\\' {
            *off += 1;
            continue;
        }
        acc.push(c);
        if c == ch {
            found_end_char = true;
            *off += 1;
            break;
        }
        *off += 1;
    }
    found_end_char
}

pub fn is_leaf_block(b_type: BlockType) -> bool {
    matches!(
        b_type,
            | BlockType::H
            | BlockType::Math
            | BlockType::Code
            | BlockType::P
    )
}

pub fn skip_whitespace(ctx: &Context, off: &mut Offset) {
    while *off < ctx.text.len() && ctx.char_at(*off) != '\n' {
        if !is_whitespace_not_eol(ctx.char_at(*off)) {
            break;
        }
        *off += 1;
    }
}