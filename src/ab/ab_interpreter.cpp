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

    struct Line {
        LINETYPE type = LINE_BLANK;
        OFFSET beg = 0;
        OFFSET end = 0;
        unsigned indent = 0;
    };


    /******************************************
     ***  Processing Inlines (a.k.a Spans)  ***
     ******************************************/

     /* The mark structure.
      *
      * '\\': Maybe escape sequence.
      * '\0': NULL char.
      *  '*': Maybe (strong) emphasis start/end.
      *  '_': Maybe (strong) emphasis start/end.
      *  '~': Maybe strikethrough start/end (needs MD_FLAG_STRIKETHROUGH).
      *  '`': Maybe code span start/end.
      *  '&': Maybe start of entity.
      *  ';': Maybe end of entity.
      *  '<': Maybe start of raw HTML or autolink.
      *  '>': Maybe end of raw HTML or autolink.
      *  '[': Maybe start of link label or link text.
      *  '!': Equivalent of '[' for image.
      *  ']': Maybe end of link label or link text.
      *  '@': Maybe permissive e-mail auto-link (needs MD_FLAG_PERMISSIVEEMAILAUTOLINKS).
      *  ':': Maybe permissive URL auto-link (needs MD_FLAG_PERMISSIVEURLAUTOLINKS).
      *  '.': Maybe permissive WWW auto-link (needs MD_FLAG_PERMISSIVEWWWAUTOLINKS).
      *  'D': Dummy mark, it reserves a space for splitting a previous mark
      *       (e.g. emphasis) or to make more space for storing some special data
      *       related to the preceding mark (e.g. link).
      */
    struct Mark {
        OFFSET prev;
        OFFSET next;
        CHAR ch;
        unsigned char flags;
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
    /* To avoid `if (ctx->current_container == nullptr)`, we make
    * sure to never call this function when current_container is BLOCK_DOCUMENT */
    static void close_current_container(Context* ctx, OFFSET end) {
        ctx->current_container->end = end;
        ctx->current_container = ctx->current_container->parent;
    }

    static unsigned find_line_indent(Context* ctx, OFFSET off, unsigned current_indent, OFFSET* indent_pos) {
        unsigned indent = current_indent;

        while (off < ctx->size && ISBLANK(off)) {
            if (CH(off) == '\t')
                indent += 4;
            else
                indent++;
            off++;
        }
        *indent_pos = off;
        return indent - current_indent;
    }
    static OFFSET find_next_line(Context* ctx, OFFSET off) {
        OFFSET current_line_number = ctx->offset_to_line_number[off];
        if (current_line_number >= ctx->line_number_begs.size())
            return -1;
        else
            return ctx->line_number_begs[current_line_number + 1];
    }


    bool process_line(Context* ctx, OFFSET off, OFFSET* end, Line* line_start_block, Line* current_line) {
        bool ret = true;

        current_line->indent = find_line_indent(ctx, off, 0, end);
        current_line->beg = off;
        OFFSET end_of_line = ctx->line_number_begs[ctx->offset_to_line_number[off] + 1];

        off = *end;

        int n_siblings = ctx->current_container->parent->children.size();
        int current_indent = ctx->current_container->indent;
        /* We do not count the ROOT doc as a parent (which is always there),
        * hence -1 */
        int n_parents = -1;
        Container* parent = ctx->current_container;
        while (parent != nullptr) {
            n_parents++;
            parent = ctx->current_container->parent;
        }

        while (CH(off) != '\n') {
            off++;
        }

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

        // while (off < ctx->size) {
        //     CHECK_AND_RET(process_line(ctx, off, &off, &line_start_block, &current_line));
        //     // if (line_start_block == )    
        // }

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