#include "definitions.h"

namespace AB {
    const char* block_to_name(BLOCK_TYPE type) {
        switch (type) {
        case BLOCK_DOC:
            return "DOCUMENT";
        case BLOCK_QUOTE:
            return "B_QUOTE";
        case BLOCK_HIDDEN:
            return "B_HIDDEN";
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
}