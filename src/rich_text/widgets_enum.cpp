#include "widgets_enum.h"

namespace RichText {
    const char* type_to_name(Type type) {
        switch (type) {
        case T_ROOT:
            return "T_ROOT";
        case T_BLOCK_P:
            return "p";
        case T_BLOCK_CODE:
            return "code";
        case T_BLOCK_QUOTE:
            return "quote";
        case T_BLOCK_HIDDENSPACE:
            return "hidden";
        case T_BLOCK_TABLE:
            return "table";
        case T_BLOCK_THEAD:
            return "thead";
        case T_BLOCK_TBODY:
            return "tbody";
        case T_BLOCK_TR:
            return "tr";
        case T_BLOCK_TH:
            return "th";
        case T_BLOCK_TD:
            return "td";
        case T_BLOCK_UL:
            return "ul";
        case T_BLOCK_LI:
            return "li";
        case T_BLOCK_OL:
            return "ol";
        case T_BLOCK_HR:
            return "hr";
        case T_BLOCK_H:
            return "h";
        case T_SPAN_EM:
            return "em";
        case T_SPAN_STRONG:
            return "strong";
        case T_SPAN_A:
            return "a";
        case T_SPAN_WIKILINK:
            return "wikilink";
        case T_SPAN_DEL:
            return "del";
        case T_SPAN_CODE:
            return "code";
        case T_SPAN_U:
            return "underline";
        case T_SPAN_LATEX:
            return "latex";
        case T_SPAN_DISPLAY:
            return "T_SPAN_DISPLAY";
        case T_TEXT:
            return "text";
        }
        return "NOT VALID";
    }
}