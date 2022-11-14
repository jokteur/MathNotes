#pragma once
#include <string>
#include <functional>

// Implementation is inspired from http://github.com/mity/md4c
namespace Markdown {
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

        BLOCK_CODE,
        BLOCK_HTML,
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
        SPAN_DEL,
        SPAN_LATEXMATH,
        SPAN_LATEXMATH_DISPLAY,
        SPAN_WIKILINK,
        SPAN_U,
        SPAN_HIGHLIGHT
    };

    enum TEXT_TYPE {
        TEXT_NORMAL = 0,

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

    struct BLockLiDetail : public BlockDetail {
        bool is_task;
        bool is_task_selected;
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