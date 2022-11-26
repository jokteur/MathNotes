#pragma once

namespace AB {
    typedef unsigned int SIZE;
    typedef int OFFSET;
    typedef char CHAR;

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
        BLOCK_DIV,
        BLOCK_DEF,
        BLOCK_LATEX,

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
}