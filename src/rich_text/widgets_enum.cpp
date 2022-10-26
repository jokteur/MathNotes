#include "widgets_enum.h"

namespace RichText {


    const char* type_to_name(Type type) {
        switch (type) {
            case T_ROOT:
                return "T_ROOT";
            case T_BLOCK_P:
                return "T_BLOCK_P";
            case T_BLOCK_CODE:
                return "T_BLOCK_CODE";
            case T_BLOCK_QUOTE:
                return "T_BLOCK_QUOTE";
            case T_BLOCK_HTML:
                return "T_BLOCK_HTML";
            case T_BLOCK_TABLE:
                return "T_BLOCK_TABLE";
            case T_BLOCK_THEAD:
                return "T_BLOCK_THEAD";
            case T_BLOCK_TBODY:
                return "T_BLOCK_TBODY";
            case T_BLOCK_TR:
                return "T_BLOCK_TR";
            case T_BLOCK_TH:
                return "T_BLOCK_TH";
            case T_BLOCK_TD:
                return "T_BLOCK_TD";
            case T_BLOCK_UL:
                return "T_BLOCK_UL";
            case T_BLOCK_LI:
                return "T_BLOCK_LI";
            case T_BLOCK_OL:
                return "T_BLOCK_OL";
            case T_BLOCK_HR:
                return "T_BLOCK_HR";
            case T_BLOCK_H:
                return "T_BLOCK_H";
            case T_SPAN_EM:
                return "T_SPAN_EM";
            case T_SPAN_STRONG:
                return "T_SPAN_STRONG";
            case T_SPAN_A:
                return "T_SPAN_A";
            case T_SPAN_WIKILINK:
                return "T_SPAN_WIKILINK";
            case T_SPAN_DEL:
                return "T_SPAN_DEL";
            case T_SPAN_CODE:
                return "T_SPAN_CODE";
            case T_SPAN_U:
                return "T_SPAN_U";
            case T_SPAN_LATEX:
                return "T_SPAN_LATEX";
            case T_SPAN_DISPLAY:
                return "T_SPAN_DISPLAY";
            case T_TEXT:
                return "T_TEXT";
        }
        return "NOT VALID";
    }
}