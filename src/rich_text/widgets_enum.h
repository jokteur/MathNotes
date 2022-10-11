#pragma once

namespace RichText {
    enum Type {
        T_NONAME,
        T_ROOT,
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
        T_TEXT
    };
}