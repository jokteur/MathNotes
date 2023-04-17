#pragma once

namespace RichText {
    enum Type {
        T_NONAME,
        T_ROOT,
        // Blocks
        T_BLOCK_HIDDENSPACE,
        T_BLOCK_P,
        T_BLOCK_QUOTE,
        T_BLOCK_CODE,
        T_BLOCK_HTML,
        T_BLOCK_TABLE,
        T_BLOCK_THEAD,
        T_BLOCK_TBODY,
        T_BLOCK_TR,
        T_BLOCK_TH,
        T_BLOCK_TD,
        T_BLOCK_UL,
        T_BLOCK_LI,
        T_BLOCK_OL,
        T_BLOCK_HR,
        T_BLOCK_H,
        T_BLOCK_EMPTY,
        // Spans
        T_SPAN_EM,
        T_SPAN_STRONG,
        T_SPAN_A,
        T_SPAN_WIKILINK,
        T_SPAN_DEL,
        T_SPAN_CODE,
        T_SPAN_HIGHLIGHT,
        T_SPAN_U,
        T_SPAN_LATEX,
        T_SPAN_DISPLAY,
        // Text
        T_TEXT,
        T_LATEX,
        T_CODE,
        T_INTERTEXT
    };
    enum Category {
        C_NONAME,
        C_ROOT,
        C_BLOCK,
        C_SPAN,
        C_TEXT
    };

    const char* type_to_name(Type type);
}