#include "span.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */
    bool AbstractSpan::hk_add_pre_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        if (m_is_selected) {
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_draw_chars, m_safe_string, m_text_boundaries.front().pre, m_text_boundaries.front().beg, m_special_chars_style, true);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
    bool AbstractSpan::hk_add_post_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        if (m_is_selected) {
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_draw_chars, m_safe_string, m_text_boundaries.back().end, m_text_boundaries.back().post, m_special_chars_style, true);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
    bool AbstractSpan::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        m_draw_chars.clear();

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
    void AbstractSpan::hk_draw_background(Draw::DrawList& draw_list) {
        if (m_style.bg_color != Colors::transparent) {
            draw_list.SetCurrentChannel(0);
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRectFilled(p_min, p_max, m_style.bg_color, 5.f);
        }
    }
}