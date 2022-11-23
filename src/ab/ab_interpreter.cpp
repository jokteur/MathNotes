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
 * Modifications by Jokteur, https://github.com/jokteur
 */
#include "ab_interpreter.h"
#include "internal_helpers.h"

#include <iostream>

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
    struct Container {
        OFFSET start = 0;
        OFFSET end = -1;
        BLOCK_TYPE type;
        Container* parent = nullptr;
        std::vector<Container*> children;
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

    struct Line {
        int flag = 0;
        OFFSET pre = 0;
        OFFSET beg = 0;
        OFFSET end = 0;
        OFFSET post = 0;
        unsigned indent = 0;
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

        std::vector<Container> containers;
        Container* current_container;

        std::vector<int> offset_to_line_number;
        std::vector<int> line_number_begs;
    };

    static OFFSET find_next_line_off(Context* ctx, OFFSET off) {
        OFFSET current_line_number = ctx->offset_to_line_number[off];
        if (current_line_number + 1 >= ctx->line_number_begs.size())
            return ctx->size;
        else
            return ctx->line_number_begs[current_line_number + 1];
    }

    static void add_container(Context* ctx, OFFSET start, BLOCK_TYPE type, unsigned indent) {
        Container container;
        container.start = start;
        Container* parent = ctx->current_container;
        container.parent = parent;
        container.indent = indent;
        ctx->containers.push_back(container);
        ctx->current_container = &(*(ctx->containers.end() - 1));
        parent->children.push_back(ctx->current_container);
    }
    /* To avoid `if (ctx->current_container == nullptr)`, we have to make
    * sure to never call this function when current_container is BLOCK_DOCUMENT */
    static void close_current_container(Context* ctx, OFFSET end) {
        ctx->current_container->end = end;
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
    bool check_for_whitespace(Context* ctx, OFFSET off) {
        while (CH(off) != '\n' && off < ctx->size) {
            if (!ISWHITESPACE(off))
                return false;
            off++;
        }
        return true;
    }

    bool analyze_line(Context* ctx, OFFSET off, OFFSET* end, Line* line) { //, Line* line_start_block, Line* current_line) {
        bool ret = true;

        // current_line->indent = find_line_indent(ctx, off, 0, end);
        // current_line->beg = off;
        OFFSET line_end = find_next_line_off(ctx, off);
        OFFSET start = off;
        OFFSET goto_end = line_end;

        OFFSET block_pre = off;
        OFFSET block_beg = off;
        OFFSET block_end, block_post;

        // off = *end;

        // int n_siblings = ctx->current_container->parent->children.size();
        // int current_indent = ctx->current_container->indent;
        /* We do not count the ROOT doc as a parent (which is always there),
        * hence -1 */
        // int n_parents = -1;
        Container* parent = ctx->current_container;
        // while (parent != nullptr) {
        //     n_parents++;
        //     parent = ctx->current_container->parent;
        // }

        int whitespace_counter = 0;
        std::string acc; // Acc is for accumulator

        enum SOLVED { NONE, PARTIAL, FULL };
        SOLVED block_solved = NONE;
        char list_mark = 0;
        int opener_flags = 0;
        int mark_counter = 0;
        int indent = 0;

#define MAKE_P() \
    block_pre = off; block_beg = off; opener_flags = P_OPENER; \
    block_solved = PARTIAL;
#define MAKE_UL() \
    opener_flags = LIST_OPENER; block_pre = off; block_beg = off + 2; \
    goto_end = off + 2; block_solved = FULL;
#define NEXT_LOOP() off++;
#define CHECK_WS_OR_END(off) ((off) >= ctx->size || ((off) < ctx->size && ISWHITESPACE((off))) || CH((off)) == '\n')
#define CHECK_INDENT(num) whitespace_counter - indent < (num)

        while (off < line_end) {
            acc += CH(off);

            if (CH(off) == '\\') {
                if (off == start) {
                    // Paragraph
                    MAKE_P();
                    break;
                }
                else {
                    NEXT_LOOP();
                }
            }

            if (CH(off) != ']' && opener_flags & DEFINITION_OPENER) {
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
                if (CHECK_INDENT(4) && count > 0 && count < 7
                    && CHECK_WS_OR_END(off + count)) {
                    // Valid header
                    mark_counter = count;
                    opener_flags = H_OPENER;
                    block_pre = off;
                    block_beg = off + count;
                    block_solved = FULL;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '>') {
                if (CHECK_INDENT(2)) {
                    // Valid quote
                    block_pre = off; block_beg = off;
                    opener_flags = QUOTE_OPENER;
                    goto_end = off + 1;
                    block_solved = FULL;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '`') {
                int backtick_counter = count_marks(ctx, off, '`');
                if (CHECK_INDENT(4) && backtick_counter > 2) {
                    // Valid code
                    mark_counter = backtick_counter;
                    block_pre = off;
                    block_beg = off + backtick_counter;
                    opener_flags = CODE_OPENER;
                    block_solved = FULL;
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
                if (CHECK_WS_OR_END(off + 1) && !(opener_flags & LIST_OPENER)) {
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
                if (count > 2 && check_for_whitespace(ctx, off + count)) {
                    opener_flags = HR_OPENER;
                    block_pre = off;
                    block_beg = off + count;
                    block_solved = FULL;
                    break;
                }
                else if (CHECK_WS_OR_END(off + 1) && !(opener_flags & LIST_OPENER)) {
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
                if (CHECK_WS_OR_END(off + 1) && !(opener_flags & LIST_OPENER)) {
                    // Make list
                    MAKE_UL();
                    break;
                }
            }
            // Potential ordered lists
            else if (CH(off) == '(') {
                if (opener_flags & LIST_OPENER) {
                    // Paragraph
                    MAKE_P();
                    break;
                }
                acc.clear();
                list_mark = '(';
                block_pre = off;
                block_beg = off + 1;
                opener_flags |= LIST_OPENER;
            }
            else if (ISANYOF2(off, ')', '.')) {
                std::string str = acc.substr(0, acc.size() - 1);
                if (str.length() > 0 && str.length() < 12 && CHECK_WS_OR_END(off + 1)) {
                    // Potential list
                    // The validity of enumeration should still be checked
                    block_end = off;
                    block_post = off + 2;
                    opener_flags = LIST_OPENER;
                    block_solved = PARTIAL;
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
                if (CHECK_INDENT(4) && count == 3) {
                    block_solved = FULL;
                    block_pre = off;
                    block_beg = off + count;
                    opener_flags = DIV_OPENER;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }

            }
            else if (CH(off) == '[') {
                if (CHECK_INDENT(4)) {
                    block_pre = off;
                    block_beg = off + 1;
                    opener_flags |= DEFINITION_OPENER;
                    acc.clear();
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == ']') {
                if (opener_flags & DEFINITION_OPENER && CH(off + 1) == ':') {
                    block_solved = FULL;
                    block_end = off;
                    block_post = off + 2;
                    opener_flags = DEFINITION_OPENER;
                    break;
                }
                else {
                    // Paragraph
                    MAKE_P();
                    break;
                }
            }
            else if (CH(off) == '$') {
                if (CHECK_INDENT(4) && CH(off + 1) == '$') {
                    // TODO: need to find closure
                    block_pre = off;
                    block_beg = off + 2;
                    block_solved = PARTIAL;
                    opener_flags = LATEX_OPENER;
                    break;
                }
                else {
                    MAKE_P();
                    break;
                }
            }

            NEXT_LOOP();
        }

        if (opener_flags & P_OPENER)
            std::cout << "P ";
        if (opener_flags & CODE_OPENER)
            std::cout << "CODE ";
        if (opener_flags & QUOTE_OPENER)
            std::cout << "QUOTE ";
        if (opener_flags & HR_OPENER)
            std::cout << "HR ";
        if (opener_flags & H_OPENER)
            std::cout << "H ";
        if (opener_flags & DIV_OPENER)
            std::cout << "DIV ";
        if (opener_flags & DEFINITION_OPENER)
            std::cout << "DEF ";
        if (opener_flags & LIST_OPENER)
            std::cout << "LIST ";
        if (opener_flags & LATEX_OPENER)
            std::cout << "LATEX ";

        std::cout << block_pre << " " << block_beg << " " << block_end << " " << block_end << std::endl;

        *end = goto_end;

        return ret;
    abort:
        return ret;
    }

    bool parse_blocks(Context* ctx) {
        bool ret = true;

        OFFSET off = 0;
        Line line_start_block;
        Line current_line;
        Line pivot_line;

        /* Add doc container */
        Container doc_container;
        doc_container.type = BLOCK_DOC;
        ctx->containers.push_back(doc_container);
        ctx->current_container = &(*(ctx->containers.end() - 1));

        Line line;
        while (off < ctx->size) {
            CHECK_AND_RET(analyze_line(ctx, off, &off, &line));
            //     // if (line_start_block == )    
        }

        return ret;
    abort:
        return ret;
    }

    void generate_line_number_data(Context* ctx) {
        ctx->offset_to_line_number.reserve(ctx->size);
        /* The first line always starts at 0 */
        ctx->line_number_begs.push_back(0);
        int line_counter = 0;
        for (auto i = 0;i < ctx->size;i++) {
            ctx->offset_to_line_number.push_back(line_counter);
            if (ctx->text[i] == '\n') {
                line_counter++;
                if (i + 1 < ctx->size)
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