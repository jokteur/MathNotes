#include "span.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"
#include <sstream>

#include "profiling.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */
    bool AbstractSpan::hk_add_pre_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        if (m_is_selected) {
            auto& bounds = m_text_boundaries.front();
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_safe_string, bounds.line_number, bounds.pre, bounds.beg, m_special_chars_style, true);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
    bool AbstractSpan::hk_add_post_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        if (m_is_selected) {
            auto& bounds = m_text_boundaries.back();
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_safe_string, bounds.line_number, bounds.end, bounds.post, m_special_chars_style, true);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
    bool AbstractSpan::add_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        // m_chars.clear();

        hk_add_pre_chars(wrap_chars);
        for (auto ptr : m_childrens) {
            auto res = ptr->add_chars(wrap_chars);
            if (!res) {
                success = false;
            }
        }
        hk_add_post_chars(wrap_chars);
        return success;
    }
    void AbstractSpan::hk_debug_attributes() {
        AbstractElement::hk_debug_attributes();
    }
}