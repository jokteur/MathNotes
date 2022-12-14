/*
 * Part of this code is taken from MD4C: Markdown parser for C, http://github.com/mity/md4c
 * The licence for md4c is given below:
 *
 * Copyright (c) 2016-2020 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Terrible modifications by Jokteur, https://github.com/jokteur
 */
#include "ab_interpreter.h"
#include "internal_helpers.h"

#include <iostream>
#include <memory>

#include <unordered_set>
#include <unordered_map>

namespace AB {
    // TODO: correct utf8 implementation
    inline SIZE next_utf8_char(SIZE text_pos) {
        return text_pos + 1;
    }


    /* For the analysis of inline markers, this struct helps to manage
     * unresolved openers.
     */
    struct MarkChain {
        int start;
        int end;
    };

    enum LINETYPE {
        LINE_BLANK,
        LINE_HR,
        LINE_ATXHEADER,
        LINE_SETEXTHEADER,
        LINE_SETEXTUNDERLINE,
        LINE_INDENTEDCODE,
        LINE_FENCEDCODE,
        LINE_HTML,
        LINE_TEXT,
        LINE_TABLE,
        // LINE_TABLEUNDERLINE
    };

#define ENTER_BLOCK(type, arg) \
    ctx->parser.enter_block((type), (arg));
#define LEAVE_BLOCK(type, arg) \
    ctx->parser.leave_block((type), (arg));
#define ENTER_SPAN(type, arg) \
    ctx->parser.enter_span((type), (arg));
#define LEAVE_SPAN(type, arg) \
    ctx->parser.leave_span((type), (arg));
#define TEXT(type, begin, end) \
    ctx->parser.text((type), (arg));

#define CHECK_AND_RET(fct) \
    ret = (fct); \
    if (!ret) \
        goto abort;


    /**************************
     ***  Processing Block  ***
     **************************/
    struct Boundaries {
        OFFSET pre = 0;
        OFFSET beg = 0;
        OFFSET end = 0;
        OFFSET post = 0;
    };

    struct Container;
    typedef std::shared_ptr<Container> ContainerPtr;
    struct Container {
        bool closed = false;

        BLOCK_TYPE type;
        std::shared_ptr<BlockDetail> detail;
        ContainerPtr parent = nullptr;
        std::vector<ContainerPtr> children;
        std::vector<Boundaries> content_boundaries;
        unsigned indent = 0;
    };

    static const int LIST_OPENER = 0x1;
    static const int CODE_OPENER = 0x2;
    static const int HR_OPENER = 0x4;
    static const int H_OPENER = 0x8;
    static const int P_OPENER = 0x10;
    static const int QUOTE_OPENER = 0x20;
    static const int DIV_OPENER = 0x40;
    static const int DEFINITION_OPENER = 0x080;
    static const int LATEX_OPENER = 0x100;

#define IS_LIST(container) ((container)->type == BLOCK_OL || (container)->type == BLOCK_UL || (container)->type == BLOCK_LI)

    struct SegmentInfo {
        int flags = 0;
        BLOCK_TYPE type = BLOCK_DOC;
        Boundaries b_bounds;
        OFFSET start = 0;
        OFFSET end = 0;
        OFFSET first_non_blank = 0;
        OFFSET num_blank_before = 0;
        // unsigned indent = 0;
        bool blank_line = true;
        ContainerPtr current_container = nullptr;

        // List information
        char li_pre_marker = 0;
        char li_post_marker = 0;
        std::string li_number;
    };

    /**************
    *** Context ***
    ***************/
    /* The context is used throughout all the code and keeps stored
     * all the necessary informations for parsing
    */
    struct Context {
        /* Information given by the user */
        const CHAR* text;
        SIZE size;
        const Parser* parser;

        std::vector<ContainerPtr> containers;
        ContainerPtr current_container;
        ContainerPtr above_container = nullptr;

        std::vector<Boundaries> non_commited_blanks;

        std::vector<SegmentInfo*>* seg_above_history;
        std::vector<SegmentInfo*>* seg_history;

        std::vector<int> offset_to_line_number;
        std::vector<int> line_number_begs;
    };

    static std::string to_string(Context* ctx, OFFSET start, OFFSET end) {
        std::string out;
        for (OFFSET i = start;i < end;i++) {
            out += CH(i);
        }
        return out;
    }

    static OFFSET find_next_line_off(Context* ctx, OFFSET off) {
        OFFSET current_line_number = ctx->offset_to_line_number[off];
        if (current_line_number + 1 >= ctx->line_number_begs.size())
            return ctx->size;
        else
            return ctx->line_number_begs[current_line_number + 1] - 1;
    }

    ContainerPtr find_root_parent(Context* ctx, ContainerPtr& container) {
        ContainerPtr parent = container;
        while (parent->parent != nullptr) {
            if (parent->parent->type == BLOCK_DOC)
                break;
            parent = parent->parent;
        }
        return parent;
    }

    static void add_container(Context* ctx, SegmentInfo* seg, std::vector<SegmentInfo>* current_history, BLOCK_TYPE block_type, const Boundaries& bounds, std::shared_ptr<BlockDetail> detail = nullptr) {
        ContainerPtr container = std::make_shared<Container>();
        container->type = block_type;
        container->content_boundaries.push_back(bounds);
        container->detail = detail;
        ContainerPtr parent = ctx->current_container;
        container->parent = parent;
        if (seg != nullptr) {
            seg->current_container = container;
            seg->type = block_type;
            current_history->push_back(*seg);
        }

        ctx->containers.push_back(container);
        ctx->current_container = *(ctx->containers.end() - 1);
        parent->children.push_back(ctx->current_container);
    }
    static void add_container2(Context* ctx, BLOCK_TYPE block_type, const Boundaries& bounds, std::shared_ptr<BlockDetail> detail = nullptr) {
        ContainerPtr container = std::make_shared<Container>();
        container->type = block_type;
        container->content_boundaries.push_back(bounds);
        container->detail = detail;
        ContainerPtr parent = ctx->current_container;
        container->parent = parent;
        // if (seg != nullptr) {
        //     seg->current_container = container;
        //     seg->type = block_type;
        // }

        ctx->containers.push_back(container);
        ctx->current_container = *(ctx->containers.end() - 1);
        parent->children.push_back(ctx->current_container);
    }

    /* To avoid `if (ctx->current_container == nullptr)`, we have to make
    * sure to never call this function when current_container is BLOCK_DOCUMENT */
    static void close_current_container(Context* ctx) {
        ctx->current_container->closed = true;
        ctx->current_container = ctx->current_container->parent;
    }

    /**
     * Verifies if a str can be converted into a positiv number
    */
    bool verify_positiv_number(const std::string& str) {
        if (str.empty())
            return false;
        if (str.length() == 1 && ISDIGIT_(str[0]))
            return true;
        for (int i = 0;i < str.length();i++) {
            if (!ISDIGIT_(str[i]) || (i == 0 && str[i] == '0'))
                return false;
        }
        return true;
    }

    int count_marks(Context* ctx, OFFSET off, char mark) {
        int counter = 0;
        while (CH(off) != '\n' && off < ctx->size) {
            if (CH(off) == mark)
                counter++;
            else
                break;
            off++;
        }
        return counter;
    }
    bool check_for_whitespace_after(Context* ctx, OFFSET off) {
        while (CH(off) != '\n' && off < ctx->size) {
            if (!ISWHITESPACE(off))
                return false;
            off++;
        }
        return true;
    }

    bool analyze_segment(Context* ctx, OFFSET off, OFFSET* end, SegmentInfo* seg, int indent) {
        bool ret = true;

        seg->end = find_next_line_off(ctx, off);
        OFFSET goto_end = seg->end;
        seg->start = off;
        seg->first_non_blank = seg->end;

        seg->b_bounds.pre = off;
        seg->b_bounds.beg = off;
        seg->b_bounds.end = -1;
        seg->b_bounds.post = -1;

        seg->blank_line = true;
        int whitespace_counter = 0;

        std::string acc; // Acc is for accumulator

        seg->flags = 0;

        enum SOLVED { NONE, PARTIAL, FULL };
        SOLVED b_solved = NONE;
#define MAKE_P() \
    seg->b_bounds.pre = off; seg->b_bounds.beg = off; seg->flags = P_OPENER; \
    b_solved = PARTIAL; seg->type = BLOCK_P;
#define MAKE_UL() \
    seg->flags = LIST_OPENER; seg->b_bounds.pre = seg->start; \
    seg->b_bounds.beg = (off + 1 == seg->end) ? off + 1 : off + 2; \
    goto_end = off + 2; b_solved = FULL; seg->type = BLOCK_UL; \
    seg->li_pre_marker = CH(off);
#define NEXT_LOOP() off++;
#define CHECK_WS_OR_END(off) ((off) >= ctx->size || ((off) < ctx->size && ISWHITESPACE((off))) || CH((off)) == '\n')
#define CHECK_WS_BEFORE(off) (seg->first_non_blank >= (off))
#define CHECK_INDENT(allowed_ws) (off - seg->start < indent + (allowed_ws))

        while (off < seg->end) {
            acc += CH(off);

            if (!(ISWHITESPACE(off) || CH(off) == '\n') && seg->blank_line) {
                seg->blank_line = false;
                seg->first_non_blank = off;
                if (!(seg->flags & DEFINITION_OPENER))
                    acc = CH(off);
            }

            if (CH(off) == '\\') {
                if (off == seg->start) {
                    // Paragraph
                    MAKE_P();
                    break;
                }
                else {
                    NEXT_LOOP();
                }
            }

            if (CH(off) != ']' && seg->flags & DEFINITION_OPENER) {
                NEXT_LOOP();
                continue;
            }

            // Block detection
            if (CH(off) == ' ') {
                whitespace_counter++;
            }
            else if (CH(off) == '\t') {
                whitespace_counter += 4;
            }
            else if (CH(off) == '#') {
                int count = count_marks(ctx, off, '#');
                if (CHECK_WS_BEFORE(off) && count > 0 && count < 7
                    && CHECK_WS_OR_END(off + count) && CHECK_INDENT(3)) {
                    // Valid header
                    seg->flags = H_OPENER;
                    seg->b_bounds.pre = seg->start;
                    seg->b_bounds.beg = off + count;
                    b_solved = FULL;
                    seg->type = BLOCK_H;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '>') {
                if (CHECK_WS_BEFORE(off) && CHECK_INDENT(1)) {
                    // Valid quote
                    seg->b_bounds.pre = seg->start; seg->b_bounds.beg = off + 1;
                    seg->flags = QUOTE_OPENER;
                    goto_end = off + 1;
                    b_solved = FULL;
                    seg->type = BLOCK_QUOTE;
                    if (off < ctx->size && CH(off + 1) == ' ') {
                        seg->b_bounds.beg = off + 2;
                        goto_end = off + 2;
                    }
                    break;
                }
                else {
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '`') {
                int backtick_counter = count_marks(ctx, off, '`');
                if (backtick_counter > 2 && CHECK_WS_BEFORE(off) && CHECK_INDENT(3)) {
                    // Valid code
                    seg->b_bounds.pre = seg->start;
                    seg->b_bounds.beg = seg->end;
                    seg->flags = CODE_OPENER;
                    b_solved = FULL;
                    seg->type = BLOCK_CODE;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            // Potential bullet lists
            else if (CH(off) == '*') {
                if (CHECK_WS_BEFORE(off) && CHECK_WS_OR_END(off + 1) && !(seg->flags & LIST_OPENER)) {
                    // Make list
                    MAKE_UL();
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '-') {
                int count = count_marks(ctx, off, '-');
                if (CHECK_WS_BEFORE(off) && count > 2 && check_for_whitespace_after(ctx, off + count)) {
                    seg->flags = HR_OPENER;
                    seg->b_bounds.pre = off;
                    seg->b_bounds.beg = off + count;
                    b_solved = FULL;
                    seg->type = BLOCK_HR;
                    break;
                }
                else if (CHECK_WS_BEFORE(off) && CHECK_WS_OR_END(off + 1) && !(seg->flags & LIST_OPENER)) {
                    // Unordered list
                    MAKE_UL();
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }

            }
            else if (CH(off) == '+') {
                if (CHECK_WS_BEFORE(off) && CHECK_WS_OR_END(off + 1) && !(seg->flags & LIST_OPENER)) {
                    // Make list
                    MAKE_UL();
                    break;
                }
            }
            // Potential ordered lists
            else if (CH(off) == '(') {
                if (seg->flags & LIST_OPENER || !CHECK_WS_BEFORE(off)) {
                    // Paragraph
                    MAKE_P();
                    break;
                }
                b_solved = PARTIAL;
                acc.clear();
                seg->flags |= LIST_OPENER;
                seg->li_pre_marker = '(';
            }
            else if (ISANYOF2(off, ')', '.')) {
                std::string str = acc.substr(0, acc.size() - 1);
                if (str.length() > 0 && str.length() < 12 && CHECK_WS_OR_END(off + 1)) {
                    // Potential list
                    // The validity of enumeration should still be checked
                    seg->b_bounds.pre = seg->start;
                    seg->b_bounds.beg = off + 1;
                    goto_end = off + 1;
                    if (off + 1 < ctx->size && CH(off + 1) == ' ') {
                        seg->b_bounds.beg = off + 2;
                        goto_end = off + 2;
                    }
                    seg->flags = LIST_OPENER;
                    b_solved = PARTIAL;
                    seg->li_post_marker = CH(off);
                    acc = str;
                    break;
                }
                else {
                    MAKE_P();
                    break;
                }
            }
            // Potential definitions or divs
            else if (CH(off) == ':') {
                int count = count_marks(ctx, off, ':');
                if (count == 3 && CHECK_WS_BEFORE(off) && CHECK_INDENT(3)) {
                    b_solved = FULL;
                    seg->b_bounds.pre = off;
                    seg->b_bounds.beg = off + count;
                    seg->type = BLOCK_DIV;
                    seg->flags = DIV_OPENER;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }

            }
            else if (CH(off) == '[') {
                if (CHECK_INDENT(3)) {
                    seg->b_bounds.pre = off;
                    seg->b_bounds.beg = off + 1;
                    seg->flags |= DEFINITION_OPENER;
                    acc.clear();
                }
                else {
                    MAKE_P();
                }
            }
            else if (CH(off) == ']') {
                if (seg->flags & DEFINITION_OPENER && CH(off + 1) == ':') {
                    b_solved = FULL;
                    seg->b_bounds.end = off;
                    seg->b_bounds.post = off + 2;
                    seg->flags = DEFINITION_OPENER;
                    seg->type = BLOCK_DEF;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '$') {
                if (CH(off + 1) == '$' && CHECK_WS_BEFORE(off) && CHECK_INDENT(3)) {
                    // TODO: need to find closure
                    seg->b_bounds.pre = off;
                    seg->b_bounds.beg = off + 2;
                    b_solved = PARTIAL;
                    seg->flags = LATEX_OPENER;
                    seg->type = BLOCK_LATEX;
                    break;
                }
                else {
                    MAKE_P();
                    break;
                }
            }
            NEXT_LOOP();
        }

#define MAKE_P_FROM_LIST() \
    off = seg->first_non_blank; goto_end = seg->end; \
    seg->b_bounds.pre = off; seg->b_bounds.beg = off; \
    seg->flags = P_OPENER; b_solved = PARTIAL; seg->type = BLOCK_P;

        if (seg->flags & LIST_OPENER && b_solved == PARTIAL) {
            if (seg->li_pre_marker == '(' && seg->li_post_marker != ')') {
                MAKE_P_FROM_LIST();
            }
            else {
                // Still need to verify if ordered list has valid enumeration
                if ((verify_positiv_number(acc) && acc.length() < 10)
                    || validate_roman_enumeration(acc)
                    || alpha_to_decimal(acc) > 0 && acc.length() < 4) {
                    b_solved = FULL;
                    seg->li_number = acc;
                    seg->type = BLOCK_OL;
                }
                else {
                    MAKE_P_FROM_LIST();
                }
            }
        }
        if (seg->flags == 0) {
            seg->flags = P_OPENER;
            seg->type = BLOCK_P;
            seg->b_bounds.pre = seg->start;
            seg->b_bounds.beg = seg->start;
            seg->b_bounds.end = seg->end;
            seg->b_bounds.post = seg->end;
        }

        if (seg->first_non_blank != seg->end)
            seg->num_blank_before = seg->first_non_blank - seg->start;

        *end = goto_end;

        return ret;
    abort:
        return ret;
    }

    bool make_list_item(Context* ctx, SegmentInfo* seg, SegmentInfo* prev_seg, OFFSET off, std::vector<SegmentInfo>* current_history) {
        ContainerPtr above_container = nullptr;
        SegmentInfo* above_seg = nullptr;
        if (above_seg != nullptr)
            above_container = above_seg->current_container;

        bool is_ul = seg->li_number.empty();
        char pre_marker = seg->li_pre_marker;
        char post_marker = seg->li_post_marker;
        ContainerPtr above_list = (above_container != nullptr) ? above_container->parent : nullptr;
        bool is_above_ol = above_container != nullptr && above_list->type == BLOCK_OL;
        bool is_above_ul = above_container != nullptr && above_list->type == BLOCK_UL;

        BlockOlDetail::OL_TYPE type;
        int alpha = -1; int roman = -1;
        if (!is_ul) {
            alpha = alpha_to_decimal(seg->li_number);
            roman = roman_to_decimal(seg->li_number);
            if (verify_positiv_number(seg->li_number)) {
                type = BlockOlDetail::OL_NUMERIC;
            }
            // With this simple rule, we can decide between cases that are valid in both roman
            // and alpha case, e.g. 'i)'
            else if (alpha > 0 && roman > 0) {
                if (alpha < roman)
                    type = BlockOlDetail::OL_ALPHABETIC;
                else
                    type = BlockOlDetail::OL_ROMAN;
            }
            else if (roman > 0) {
                type = BlockOlDetail::OL_ROMAN;
            }
            else {
                type = BlockOlDetail::OL_ALPHABETIC;
            }
        }

        bool make_new_list = false;
        // No current list going on
        if (!is_above_ul && !is_above_ol) {
            make_new_list = true;
        }
        else if (is_above_ul) {
            auto detail = std::static_pointer_cast<BlockUlDetail>(above_list->detail);
            //Try to find if current list, but different markers or enumeration
            if (pre_marker != detail->marker)
                make_new_list = true;
        }
        else if (is_above_ol && !is_ul) {
            auto detail = std::static_pointer_cast<BlockOlDetail>(above_list->detail);
            if (detail->pre_marker != pre_marker || detail->post_marker != post_marker)
                make_new_list = true;

            // By default, we choose the enumeration type of the one that is lowest in decimal
            // However, if we are already in a list that is either alpha or roman, then the 
            // current list item must inherit the alpha or roman property 
            if (type != BlockOlDetail::OL_NUMERIC) {
                if (detail->type == BlockOlDetail::OL_ALPHABETIC && roman > 0 && alpha > 0)
                    type = BlockOlDetail::OL_ALPHABETIC;
                else if (detail->type == BlockOlDetail::OL_ROMAN && roman > 0 && alpha > 0)
                    type = BlockOlDetail::OL_ROMAN;
            }
            if (type != detail->type)
                make_new_list = true;
        }
        else if (is_above_ol && is_ul) {
            make_new_list = true;
        }

        // Close previous list item
        if (is_above_ul || is_above_ol) {
            for (auto ptr : above_container->children) {
                ptr->closed = true;
            }
            ctx->current_container->closed = true;
            ctx->current_container = above_container->parent->parent;
        }
        if (make_new_list) {
            if (seg->li_number.empty()) {
                auto detail = std::make_shared<BlockUlDetail>();
                detail->marker = pre_marker;
                add_container(ctx, nullptr, nullptr, BLOCK_UL, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end }, detail);
            }
            else {
                auto detail = std::make_shared<BlockOlDetail>();
                detail->pre_marker = pre_marker;
                detail->post_marker = post_marker;
                detail->type = type;
                detail->lower_case = ISLOWER(seg->b_bounds.beg);
                add_container(ctx, nullptr, nullptr, BLOCK_OL, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end }, detail);
            }
        }
        else
            ctx->current_container = above_list;

        // We can now add our list item
        auto detail = std::make_shared<BlockLiDetail>();
        if (!is_ul)
            detail->number = seg->li_number;
        add_container(ctx, seg, current_history, BLOCK_LI, { seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end }, detail);
        ctx->current_container->indent = seg->b_bounds.beg - seg->start;
        // We add a hidden block if the list item is empty
        if (seg->end <= off) {
            add_container(ctx, seg, current_history, BLOCK_HIDDEN, { seg->end, seg->end, seg->end, seg->end });
        }
        return true;
    }

    bool make_list_item2(Context* ctx, SegmentInfo* seg, OFFSET off) {
        ContainerPtr above_container = nullptr;
        SegmentInfo* above_seg = nullptr;
        if (above_seg != nullptr)
            above_container = above_seg->current_container;

        bool is_ul = seg->li_number.empty();
        char pre_marker = seg->li_pre_marker;
        char post_marker = seg->li_post_marker;
        ContainerPtr above_list = (above_container != nullptr) ? above_container->parent : nullptr;
        bool is_above_ol = above_container != nullptr && above_list->type == BLOCK_OL;
        bool is_above_ul = above_container != nullptr && above_list->type == BLOCK_UL;

        BlockOlDetail::OL_TYPE type;
        int alpha = -1; int roman = -1;
        if (!is_ul) {
            alpha = alpha_to_decimal(seg->li_number);
            roman = roman_to_decimal(seg->li_number);
            if (verify_positiv_number(seg->li_number)) {
                type = BlockOlDetail::OL_NUMERIC;
            }
            // With this simple rule, we can decide between cases that are valid in both roman
            // and alpha case, e.g. 'i)'
            else if (alpha > 0 && roman > 0) {
                if (alpha < roman)
                    type = BlockOlDetail::OL_ALPHABETIC;
                else
                    type = BlockOlDetail::OL_ROMAN;
            }
            else if (roman > 0) {
                type = BlockOlDetail::OL_ROMAN;
            }
            else {
                type = BlockOlDetail::OL_ALPHABETIC;
            }
        }

        bool make_new_list = false;
        // No current list going on
        if (!is_above_ul && !is_above_ol) {
            make_new_list = true;
        }
        else if (is_above_ul) {
            auto detail = std::static_pointer_cast<BlockUlDetail>(above_list->detail);
            //Try to find if current list, but different markers or enumeration
            if (pre_marker != detail->marker)
                make_new_list = true;
        }
        else if (is_above_ol && !is_ul) {
            auto detail = std::static_pointer_cast<BlockOlDetail>(above_list->detail);
            if (detail->pre_marker != pre_marker || detail->post_marker != post_marker)
                make_new_list = true;

            // By default, we choose the enumeration type of the one that is lowest in decimal
            // However, if we are already in a list that is either alpha or roman, then the 
            // current list item must inherit the alpha or roman property 
            if (type != BlockOlDetail::OL_NUMERIC) {
                if (detail->type == BlockOlDetail::OL_ALPHABETIC && roman > 0 && alpha > 0)
                    type = BlockOlDetail::OL_ALPHABETIC;
                else if (detail->type == BlockOlDetail::OL_ROMAN && roman > 0 && alpha > 0)
                    type = BlockOlDetail::OL_ROMAN;
            }
            if (type != detail->type)
                make_new_list = true;
        }
        else if (is_above_ol && is_ul) {
            make_new_list = true;
        }

        // Close previous list item
        if (is_above_ul || is_above_ol) {
            for (auto ptr : above_container->children) {
                ptr->closed = true;
            }
            ctx->current_container->closed = true;
            ctx->current_container = above_container->parent->parent;
        }
        if (make_new_list) {
            if (seg->li_number.empty()) {
                auto detail = std::make_shared<BlockUlDetail>();
                detail->marker = pre_marker;
                add_container(ctx, nullptr, nullptr, BLOCK_UL, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end }, detail);
            }
            else {
                auto detail = std::make_shared<BlockOlDetail>();
                detail->pre_marker = pre_marker;
                detail->post_marker = post_marker;
                detail->type = type;
                detail->lower_case = ISLOWER(seg->b_bounds.beg);
                add_container(ctx, nullptr, nullptr, BLOCK_OL, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end }, detail);
            }
        }
        else
            ctx->current_container = above_list;

        // We can now add our list item
        auto detail = std::make_shared<BlockLiDetail>();
        if (!is_ul)
            detail->number = seg->li_number;
        add_container2(ctx, BLOCK_LI, { seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end }, detail);
        ctx->current_container->indent = seg->b_bounds.beg - seg->start;
        // We add a hidden block if the list item is empty
        if (seg->end <= off) {
            add_container2(ctx, BLOCK_HIDDEN, { seg->end, seg->end, seg->end, seg->end });
        }
        return true;
    }

    bool commit_blanks(Context* ctx, ContainerPtr container) {
        ctx->current_container = container;
        for (auto& bounds : ctx->non_commited_blanks) {
            add_container2(ctx, BLOCK_HIDDEN, bounds);
        }
        ctx->non_commited_blanks.clear();
        return true;
    }


    bool process_segment2(Context* ctx, OFFSET* off, SegmentInfo* seg, int& depth) {
        bool ret = true;

        ContainerPtr above_container = ctx->above_container;

        if (above_container != nullptr && above_container->type != seg->type && above_container->parent != nullptr) {
            if (IS_LIST(above_container)) {

            }
            // while (!added_to_li && IS_LIST(ctx->current_container))
            //     ctx->current_container = ctx->current_container->parent;
        }

        // If the current container is a non-closed BLOCK_CODE, everything should be ignored, except
        // if we are closing this block
        if (above_container != nullptr && above_container->type == BLOCK_CODE && !above_container->closed) {
            // TODO: match number of ticks
            // seg->flags = CODE_OPENER
            if (seg->flags & CODE_OPENER && check_for_whitespace_after(ctx, seg->b_bounds.beg))
                close_current_container(ctx);
            else
                above_container->content_boundaries.push_back({ seg->start, seg->start, seg->end, seg->end });
            goto skip;
        }

        if (seg->blank_line) {
            //     auto type = ctx->current_container->type;
            //     if (type != BLOCK_LI && type != BLOCK_DOC && type != BLOCK_QUOTE)
            //         close_current_container(ctx);
            //     add_container2(ctx, seg, BLOCK_HIDDEN, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end });
            ctx->non_commited_blanks.push_back({ seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end });
        }
        else if (seg->flags & P_OPENER) {
            if (above_container != nullptr && above_container->type == BLOCK_P) {
                above_container->content_boundaries.push_back({ seg->start, seg->first_non_blank, seg->end, seg->end });
            }
            else {
                add_container2(ctx, BLOCK_P, { seg->start, seg->first_non_blank, seg->end, seg->end });
            }
        }
        else if (seg->flags & CODE_OPENER) {
            auto detail = std::make_shared<BlockCodeDetail>();
            detail->lang = to_string(ctx, seg->b_bounds.beg, seg->end);
            detail->num_ticks = seg->b_bounds.beg - seg->b_bounds.pre;
            add_container2(ctx, BLOCK_CODE, { seg->start, seg->end, seg->end, seg->end }, detail);
            seg->current_container = ctx->current_container;
        }
        else if (seg->flags & QUOTE_OPENER) {
            bool new_block = true;
            if (above_container != nullptr && above_container->type == BLOCK_QUOTE) {
                new_block = false;
                above_container->content_boundaries.push_back({ seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end });
            }
            else {
                add_container2(ctx, BLOCK_QUOTE, { seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end });
            }

            // In the case an empty quote has been added (i.e. '>\n'), we add an empty block inside the quote
            if (seg->end == seg->b_bounds.beg) {
                if (!new_block)
                    ctx->current_container = above_container;
                add_container2(ctx, BLOCK_HIDDEN, { seg->end, seg->end, seg->end, seg->end });
            }
        }
        else if (seg->flags & LIST_OPENER) {
            // Lists are not trivial to handle, there are a lot of edge cases
            make_list_item2(ctx, seg, *off);
        }
        return ret;
    skip:
        return ret;
    }

    bool process_segment(Context* ctx, OFFSET* off, SegmentInfo* seg, std::vector<SegmentInfo>* above_history, std::vector<SegmentInfo>* current_history, int& depth, std::unordered_set<SegmentInfo*>& flagged_li) {
        bool ret = true;

        ContainerPtr above_container = nullptr;
        SegmentInfo* above_seg = nullptr;
        if (depth < above_history->size())
            above_seg = &(*(above_history->begin() + depth));

        if (above_seg != nullptr) {
            above_container = above_seg->current_container;
            // Unless a new container is created, current segment inherits from above segments container
            seg->current_container = above_container;
        }
        bool clear_history = false;
        bool make_new_block = (above_container != nullptr && above_container->closed) ? true : false;
        ContainerPtr potential_list = above_container;
        bool has_list_above = false;

        // To contextualize the current segment, we need to use the above segment
        if (above_seg != nullptr) {
            // We check if somewhere in the above segment tree, if there is a list item laying around
            while (potential_list != nullptr) {
                if (IS_LIST(potential_list)) {
                    has_list_above = true;
                    break;
                }
                potential_list = potential_list->parent;
            }
            // If there is a list item, we check if we can match the indentation of current segment
            // to the list item
            bool added_to_li = false;
            int indent = seg->num_blank_before;
            if (has_list_above && !(seg->flags & LIST_OPENER)) {
                int list_indent = potential_list->indent;
                indent = seg->num_blank_before - list_indent;

                // If indent is positive and the current line has not been added
                // to the li, we are part of list item 
                if (indent >= 0 && flagged_li.find(above_seg) == flagged_li.end()) {
                    depth++;
                    // Rewrite history to match list structure
                    current_history->push_back(*above_seg);
                    flagged_li.insert(above_seg);
                    if (depth < above_history->size())
                        above_seg = &(*(above_history->begin() + depth));
                    above_container = above_seg->current_container;
                    seg->current_container = above_container;
                    added_to_li = true;

                    // Need to update text pos info on li/ul/ol parents
                    if (IS_LIST(potential_list)) {
                        potential_list->content_boundaries.push_back({ seg->start, seg->start + list_indent, seg->end, seg->end });
                        potential_list->parent->content_boundaries.push_back({ seg->start, seg->start, seg->end, seg->end });;
                    }
                    // LI will have pre = seg->start, beg = seg->first_non_blank
                    // This line ensures that the following block does not count the LI spaces
                    seg->b_bounds.pre += list_indent;
                    seg->start += list_indent;
                }
            }
            if ((indent > 3) || indent > 1 && (seg->flags & QUOTE_OPENER)) {
                seg->flags = P_OPENER;
                *off = seg->end;
            }

            // If flags from current and above do not match, then we have to close the current
            // container and clear the previous segment history
            if (above_seg->flags != seg->flags && above_container->parent != nullptr) {
                clear_history = true;
                ctx->current_container = above_container->parent;
                while (!added_to_li && IS_LIST(ctx->current_container))
                    ctx->current_container = ctx->current_container->parent;
            }
        }

        // If the current container is a non-closed BLOCK_CODE, everything should be ignored, except
        // if we are closing this block
        if (above_container != nullptr && above_container->type == BLOCK_CODE && !above_container->closed) {
            // TODO: match number of ticks
            // seg->flags = CODE_OPENER
            if (seg->flags & CODE_OPENER && check_for_whitespace_after(ctx, seg->b_bounds.beg))
                close_current_container(ctx);
            else
                above_container->content_boundaries.push_back({ seg->start, seg->start, seg->end, seg->end });
            goto skip;
        }

        // if (above_seg == nullptr && (seg->num_blank_before > 3 && !(seg->flags & LIST_OPENER)) ||
        //     seg->num_blank_before > 1 && (seg->flags & QUOTE_OPENER)) {
        //     seg->flags = P_OPENER;
        //     *off = seg->end;
        // }

        if (seg->blank_line) {
            auto type = ctx->current_container->type;
            if (type != BLOCK_LI && type != BLOCK_DOC && type != BLOCK_QUOTE)
                close_current_container(ctx);
            add_container(ctx, seg, current_history, BLOCK_HIDDEN, { seg->b_bounds.pre, seg->b_bounds.pre, seg->end, seg->end });
        }
        else if (seg->flags & P_OPENER) {
            if (above_container != nullptr && above_container->type == BLOCK_P && !make_new_block) {
                above_container->content_boundaries.push_back({ seg->start, seg->first_non_blank, seg->end, seg->end });
            }
            else {
                add_container(ctx, seg, current_history, BLOCK_P, { seg->start, seg->first_non_blank, seg->end, seg->end });
            }
        }
        else if (seg->flags & CODE_OPENER) {
            auto detail = std::make_shared<BlockCodeDetail>();
            detail->lang = to_string(ctx, seg->b_bounds.beg, seg->end);
            detail->num_ticks = seg->b_bounds.beg - seg->b_bounds.pre;
            add_container(ctx, seg, current_history, BLOCK_CODE, { seg->start, seg->end, seg->end, seg->end }, detail);
            seg->current_container = ctx->current_container;
        }
        else if (seg->flags & HR_OPENER) {
            add_container(ctx, seg, current_history, BLOCK_HR, { seg->start, seg->end, seg->end, seg->end });
            close_current_container(ctx);
        }
        else if (seg->flags & QUOTE_OPENER) {
            bool new_block = true;
            if (above_container != nullptr && above_container->type == BLOCK_QUOTE && !make_new_block) {
                new_block = false;
                above_container->content_boundaries.push_back({ seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end });
            }
            else {
                add_container(ctx, seg, current_history, BLOCK_QUOTE, { seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end });
            }
            // In the case an empty quote has been added (i.e. '>\n'), we add an empty block inside the quote
            if (seg->end == seg->b_bounds.beg) {
                if (!new_block)
                    ctx->current_container = above_container;
                add_container(ctx, seg, current_history, BLOCK_HIDDEN, { seg->end, seg->end, seg->end, seg->end });
            }
        }
        else if (seg->flags & H_OPENER) {
            bool new_header = true;
            int level = seg->b_bounds.beg - seg->b_bounds.pre;
            // Headers can be empty, e.g. `##`
            // In this case, the mandatory space after is not taken into account
            // When there is the mandatory space, b_beg should begin one char after
            if (seg->b_bounds.beg < seg->end) {
                seg->b_bounds.beg++;
            }

            if (above_container != nullptr && above_container->type == BLOCK_H) {
                auto detail = std::static_pointer_cast<BlockHDetail>(above_container->detail);
                if (detail->level == level && !make_new_block) {
                    new_header = false;
                    above_container->content_boundaries.push_back({ seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end });
                }
                else {
                    close_current_container(ctx);
                }
            }
            if (new_header) {
                auto detail = std::make_shared<BlockHDetail>();
                detail->level = level;
                add_container(ctx, seg, current_history, BLOCK_H, { seg->b_bounds.pre, seg->b_bounds.beg, seg->end, seg->end }, detail);
            }
        }
        else if (seg->flags & LIST_OPENER) {
            // Lists are not trivial to handle, there are a lot of edge cases
            make_list_item(ctx, seg, above_seg, *off, current_history);
        }

        if (clear_history)
            above_history->clear();
        return ret;
    skip:
        if (clear_history)
            above_history->clear();
        return ret;
    }

    bool parse_blocks(Context* ctx) {
        bool ret = true;

        OFFSET off = 0;
        SegmentInfo current_seg;
        SegmentInfo dummy;

        /* Add doc container */
        ContainerPtr doc_container = std::make_shared<Container>();
        doc_container->type = BLOCK_DOC;
        ctx->containers.push_back(doc_container);
        ctx->current_container = *(ctx->containers.end() - 1);

        std::vector<SegmentInfo> hist1, hist2;
        std::vector<SegmentInfo>* history = &hist1;
        std::vector<SegmentInfo>* prev_history = &hist2;
        std::unordered_set<SegmentInfo*> flagged_li;
        bool history_pivot = true;

        int depth = 0;
        int indent = 0;
        while (off < ctx->size) {
            if (ctx->above_container != nullptr) {
                if (!ctx->above_container->children.empty())
                    ctx->above_container = *(ctx->above_container->children.end() - 1);
                else
                    ctx->above_container = nullptr;
            }

            indent = 0;
            if (ctx->above_container != nullptr && IS_LIST(ctx->above_container)) {
                // Check indent
                ContainerPtr child = (*(ctx->above_container->children.end() - 1));
                while (!child->children.empty()) {
                    // if ((*(child->children.end() - 1))->)
                }
            }
            CHECK_AND_RET(analyze_segment(ctx, off, &off, &current_seg, indent));
            CHECK_AND_RET(process_segment2(ctx, &off, &current_seg, depth));

            // We arrived at a the end of a line
            if (off >= current_seg.end) {
                current_seg = SegmentInfo();
                flagged_li.clear();

                if (history_pivot) {
                    prev_history = &hist1;
                    history = &hist2;
                }
                else {
                    prev_history = &hist2;
                    history = &hist1;
                }
                history->clear();
                history_pivot = !history_pivot;
                ctx->above_container = *ctx->containers.begin();
                depth = -1;
                off++;
            }
            depth++;
        }

        return ret;
    abort:
        return ret;
    }

    void generate_line_number_data(Context* ctx) {
        ctx->offset_to_line_number.reserve(ctx->size);
        /* The first seg always starts at 0 */
        ctx->line_number_begs.push_back(0);
        int line_counter = 0;
        for (auto i = 0;i < ctx->size;i++) {
            ctx->offset_to_line_number.push_back(line_counter);
            if (ctx->text[i] == '\n') {
                line_counter++;
                if (i + 1 <= ctx->size)
                    ctx->line_number_begs.push_back(i + 1);
            }
        }
    }

    bool process_doc(Context* ctx) {
        bool ret = true;

        generate_line_number_data(ctx);

        /* First, process all the blocks that we
        * can find */
        CHECK_AND_RET(parse_blocks(ctx));

        for (auto ptr : ctx->containers) {
            // Find out level of widget
            // Not efficient but do not care
            int level = 0;
            auto tmp_ptr = ptr;
            while (tmp_ptr->parent != nullptr) {
                tmp_ptr = tmp_ptr->parent;
                std::cout << "  ";
                level++;
            }
            std::cout << block_to_name(ptr->type);
            for (auto bound : ptr->content_boundaries) {
                std::cout << " {" << bound.pre;
                std::cout << ", " << bound.beg;
                std::cout << ", " << bound.end;
                std::cout << ", " << bound.post << "} ";
            }
            std::cout << std::endl;
        }

    abort:
        return ret;
    }

    bool parse(const CHAR* text, SIZE size, const Parser* parser) {
        Context ctx;
        ctx.text = text;
        ctx.size = size;
        ctx.parser = parser;

        process_doc(&ctx);

        return 0;
    }
};