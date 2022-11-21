#pragma once
#include <string>
#include <functional>

// Implementation is inspired from http://github.com/mity/md4c
namespace AB {
    /* A block represents a part of the herarchy like a paragraph
     * or a list
     */
    enum BLOCK_TYPE {
        BLOCK_DOC = 0,

        BLOCK_HIDDEN,

        BLOCK_QUOTE,

        BLOCK_UL,
        BLOCK_OL,
        BLOCK_LI,

        BLOCK_HR,
        BLOCK_H,

        BLOCK_SPECIAL,

        BLOCK_CODE,
        BLOCK_P,

        BLOCK_TABLE,
        BLOCK_THEAD,
        BLOCK_TBODY,
        BLOCK_TR,
        BLOCK_TH,
        BLOCK_TD
    };
    /*
     * A sequence of spans constitute a block
     * It is generally inline
     */
    enum SPAN_TYPE {
        SPAN_EM,
        SPAN_STRONG,
        SPAN_A,
        SPAN_IMG,
        SPAN_CODE,
        SPAN_LATEXMATH,
        SPAN_LATEXMATH_DISPLAY,
        SPAN_WIKILINK,
        SPAN_SUP,
        SPAN_SUB,
        SPAN_U,
        SPAN_DEL,
        SPAN_HIGHLIGHT
    };

    enum TEXT_TYPE {
        TEXT_NORMAL = 0,

        TEXT_LATEX,

        /* Everything that is not considered content by markdown and is used to give information
         * about a block, i.e. delimitation markers, whitespace, special chars TEXT_BLOCK_MARKER_HIDDEN
         */
         TEXT_BLOCK_MARKER_HIDDEN,
         /* Everything that is not considered content by markdown and is used to give information
          * about a span, i.e. delimitation markers is TEXT_SPAN_MARKER_HIDDEN
          */
          TEXT_SPAN_MARKER_HIDDEN,
          /* Ascii chars can be escaped with \ */
          TEXT_ESCAPE_CHAR_HIDDEN
    };

    const char* block_to_name(BLOCK_TYPE type);
    const char* span_to_name(SPAN_TYPE type);
    const char* text_to_name(TEXT_TYPE type);

    std::string decimal_to_roman(int number);
    std::string decimal_to_alpha(int number);

    int roman_to_decimal(const std::string& str);
    int alpha_to_decimal(const std::string& str);

    bool validate_roman_str(const std::string& str);

    typedef unsigned int SIZE;
    typedef int OFFSET;
    typedef char CHAR;

    struct BlockDetail {
        OFFSET start;
        OFFSET end;
    };

    struct BlockCodeDetail : public BlockDetail {
        std::string lang;
    };

    struct BlockOlDetail : public BlockDetail {
        enum OL_TYPE { OL_ALPHABETIC, OL_ROMAN };
        std::string pre_marker;
        std::string post_marker;
        OL_TYPE type;
    };

    struct BlockUlDetail : public BlockDetail {
        std::string marker;
    };

    struct BlockLiDetail : public BlockDetail {
        enum TASK_STATE { TASK_EMPTY, TASK_FAIL, TASK_SUCCESS };
        bool is_task;
        std::string marker;
        TASK_STATE task_state;
    };


    struct BlockHDetail : public BlockDetail {
        unsigned char level; /* Header level (1 to 6) */
    };
    // TODO, tables

    struct SpanDetail {
        /* A span can have pre and post information (before text begins)
         * E.g. [foo](my title) is an URL, where pre->start contains [
         * and end->post contains ](my title)
         */
        OFFSET pre;
        OFFSET start;
        OFFSET end;
        OFFSET post;
    };
    struct SpanADetail : public SpanDetail {
        std::string href;
        std::string title;
    };
    struct SpanImgDetail : public SpanDetail {
        std::string src;
        std::string title;
    };
    struct SpanWikiLink : public SpanDetail {
        std::string target;
    };

    typedef std::function<bool(BLOCK_TYPE type, BlockDetail* detail)> BlockFct;
    typedef std::function<bool(SPAN_TYPE type, SpanDetail* detail)> SpanFct;
    typedef std::function<bool(TEXT_TYPE type, const OFFSET begin, const OFFSET end)> TextFct;

    struct Parser {
        BlockFct enter_block;
        BlockFct leave_block;
        SpanFct enter_span;
        SpanFct leave_span;
        TextFct text;
    };

    bool parse(const CHAR* text, SIZE size, const Parser* parser);
};