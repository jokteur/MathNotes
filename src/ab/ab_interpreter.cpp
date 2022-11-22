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

namespace AB {
    // TODO: correct utf8 implementation
    inline SIZE next_utf8_char(SIZE text_pos) {
        return text_pos + 1;
    }

    const char* block_to_name(BLOCK_TYPE type) {
        switch (type) {
        case BLOCK_DOC:
            return "DOCUMENT";
        case BLOCK_QUOTE:
            return "B_QUOTE";
        case BLOCK_UL:
            return "B_UL";
        case BLOCK_OL:
            return "B_OL";
        case BLOCK_LI:
            return "B_LI";
        case BLOCK_HR:
            return "B_HR";
        case BLOCK_H:
            return "B_H";
        case BLOCK_CODE:
            return "B_CODE";
        case BLOCK_P:
            return "B_P";
        case BLOCK_TABLE:
            return "B_TABLE";
        case BLOCK_THEAD:
            return "B_THEAD";
        case BLOCK_TBODY:
            return "B_TBODY";
        case BLOCK_TR:
            return "B_TR";
        case BLOCK_TH:
            return "B_TH";
        case BLOCK_TD:
            return "B_TD;";
        };
        return "";
    }
    const char* span_to_name(SPAN_TYPE type) {
        switch (type) {
        case SPAN_EM:
            return "S_EM";
        case SPAN_STRONG:
            return "S_STRONG";
        case SPAN_A:
            return "S_A";
        case SPAN_IMG:
            return "S_IMG";
        case SPAN_CODE:
            return "S_CODE";
        case SPAN_DEL:
            return "S_DEL";
        case SPAN_LATEXMATH:
            return "S_LATEXMATH";
        case SPAN_LATEXMATH_DISPLAY:
            return "S_LATEXMATH_DISPLAY";
        case SPAN_WIKILINK:
            return "S_WIKILINK";
        case SPAN_U:
            return "S_U";
        case SPAN_HIGHLIGHT:
            return "S_HIGHLIGHT";
        };
        return "";
    }
    const char* text_to_name(TEXT_TYPE type) {
        switch (type) {
        case TEXT_NORMAL:
            return "T_NORMAL";
        case TEXT_LATEX:
            return "T_LATEX";
        case TEXT_BLOCK_MARKER_HIDDEN:
            return "T_BLOCK_MARKER_HIDDEN";
        case TEXT_SPAN_MARKER_HIDDEN:
            return "T_SPAN_MARKER_HIDDEN";
        case TEXT_ESCAPE_CHAR_HIDDEN:
            return "T_ESCAPE_CHAR_HIDDEN";
        };
        return "";
    }

    std::string decimal_to_roman(int number) {
        if (number > 3999 || number < 1)
            return "";

    }
    std::string decimal_to_str(int number) {
        if (number < 1)
            return "";

        int tmp, power, order;
        power = 1; order = 0; tmp = 26;
        while (tmp < number) {
            order++;
            power = tmp;
            tmp *= 26;
        }
        std::vector<char> res;
        for (int i = order; i >= 0;i--) {
            int div = number / power;
            number = number % power;
            res.push_back(div);
            power /= 26;
        }
        for (int i = res.size() - 1; i > 0;i--) {
            if (res[i] == 0) {
                res[i] = 26;
                for (int j = i - 1; j >= 0; j--) {
                    bool carry_done = res[j] != 0;
                    if (carry_done)
                        res[j] = (res[j] - 1) % 26;
                    else
                        res[j] = 25;
                    if (carry_done)
                        break;
                }
            }
        }
        std::string str;
        for (auto i : res) {
            if (i > 0) {
                str += 64 + i;
            }
        }
        return str;
    }

    int roman_value(const char c) {
        switch (c) {
        case 'I':
        case 'i':
            return 1;
        case 'V':
        case 'v':
            return 5;
        case 'X':
        case 'x':
            return 10;
        case 'L':
        case 'l':
            return 50;
        case 'C':
        case 'c':
            return 100;
        case 'D':
        case 'd':
            return 500;
        case 'M':
        case 'm':
            return 1000;
        };
        return 0;
    }

    int roman_to_decimal(const std::string& str) {
        int res = 0;
        for (int i = 0; i < str.length() - 1; ++i) {
            if (roman_value(str[i]) < roman_value(str[i + 1]))
                res -= roman_value(str[i]);
            else
                res += roman_value(str[i]);
        }
        res += roman_value(str[str.length() - 1]);
        return res;
    }
    int alpha_to_decimal(const std::string& str) {
        int res = 0;
        int multiplier = 1;
        for (int i = str.length() - 1;i >= 0;i--) {
            char letter = str[i];
            // Uppercase letter
            if (letter >= 65 && letter <= 90) {
                res += multiplier * (letter - 64);
            }
            // Lowercase letter
            else if (letter >= 97 && letter <= 122) {
                res += multiplier * (letter - 96);
            }
            else {
                return -1;
            }
            multiplier *= 26;
        }
        return res;
    }
    bool validate_roman_str(const std::string& str) {
        char prev = 0;
        int counter = 0;
        int prev_counter = 0;

        static char* units[] = { "sdf" };

        for (auto c : str) {
            if (roman_value(c) == 0) {
                return false;
            }
            if (prev == c) {
                // Not powers of 10 should never repeat
                if (c == 'V' || c == 'L' || c == 'D') {
                    return false;
                }
                counter++;
            }
            else {
                prev_counter = counter;
                counter = 0;
            }
            if (counter > 3) {
                return false;
            }
            if (c == 'X' && prev)
                prev = c;
        }
    }

    /*****************
     ***  Helpers  ***
     *****************/
     /* Character accessors. */
#define CH(off)                 (ctx->text[(off)])
#define STR(off)                (ctx->text + (off))

     /* Character classification.
      * Note we assume ASCII compatibility of code points < 128 here. */
#define _T(t) t
#define ISIN_(ch, ch_min, ch_max)       ((ch_min) <= (unsigned)(ch) && (unsigned)(ch) <= (ch_max))
#define ISANYOF_(ch, palette)           ((ch) != _T('\0')  &&  md_strchr((palette), (ch)) != NULL)
#define ISANYOF2_(ch, ch1, ch2)         ((ch) == (ch1) || (ch) == (ch2))
#define ISANYOF3_(ch, ch1, ch2, ch3)    ((ch) == (ch1) || (ch) == (ch2) || (ch) == (ch3))
#define ISASCII_(ch)                    ((unsigned)(ch) <= 127)
#define ISBLANK_(ch)                    (ISANYOF2_((ch), _T(' '), _T('\t')))
#define ISNEWLINE_(ch)                  (ISANYOF2_((ch), _T('\r'), _T('\n')))
#define ISWHITESPACE_(ch)               (ISBLANK_(ch) || ISANYOF2_((ch), _T('\v'), _T('\f')))
#define ISCNTRL_(ch)                    ((unsigned)(ch) <= 31 || (unsigned)(ch) == 127)
#define ISPUNCT_(ch)                    (ISIN_(ch, 33, 47) || ISIN_(ch, 58, 64) || ISIN_(ch, 91, 96) || ISIN_(ch, 123, 126))
#define ISUPPER_(ch)                    (ISIN_(ch, _T('A'), _T('Z')))
#define ISLOWER_(ch)                    (ISIN_(ch, _T('a'), _T('z')))
#define ISALPHA_(ch)                    (ISUPPER_(ch) || ISLOWER_(ch))
#define ISDIGIT_(ch)                    (ISIN_(ch, _T('0'), _T('9')))
#define ISXDIGIT_(ch)                   (ISDIGIT_(ch) || ISIN_(ch, _T('A'), _T('F')) || ISIN_(ch, _T('a'), _T('f')))
#define ISALNUM_(ch)                    (ISALPHA_(ch) || ISDIGIT_(ch))

#define ISANYOF(off, palette)           ISANYOF_(CH(off), (palette))
#define ISANYOF2(off, ch1, ch2)         ISANYOF2_(CH(off), (ch1), (ch2))
#define ISANYOF3(off, ch1, ch2, ch3)    ISANYOF3_(CH(off), (ch1), (ch2), (ch3))
#define ISASCII(off)                    ISASCII_(CH(off))
#define ISBLANK(off)                    ISBLANK_(CH(off))
#define ISNEWLINE(off)                  ISNEWLINE_(CH(off))
#define ISWHITESPACE(off)               ISWHITESPACE_(CH(off))
#define ISCNTRL(off)                    ISCNTRL_(CH(off))
#define ISPUNCT(off)                    ISPUNCT_(CH(off))
#define ISUPPER(off)                    ISUPPER_(CH(off))
#define ISLOWER(off)                    ISLOWER_(CH(off))
#define ISALPHA(off)                    ISALPHA_(CH(off))
#define ISDIGIT(off)                    ISDIGIT_(CH(off))
#define ISXDIGIT(off)                   ISXDIGIT_(CH(off))
#define ISALNUM(off)                    ISALNUM_(CH(off))

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