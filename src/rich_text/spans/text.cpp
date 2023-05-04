#include "text.h"
#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "profiling.h"

namespace RichText {
    TextString::TextString(): AbstractSpan() {
        m_type = T_TEXT;
    }

    bool TextString::add_chars(WrapString* wrap_chars) {
        //ZoneScoped;
        m_chars->clear();
        bool success = true;

        success = hk_add_pre_chars(wrap_chars);

        if (!Utf8StrToImCharStr(m_ui_state, wrap_chars, m_safe_string, m_text_boundaries.front().beg, m_text_boundaries.front().end, m_style))
            success = false;


        if (!hk_add_post_chars(wrap_chars))
            success = false;
        return success;
    }
    float TextString::hk_set_position(float& cursor_y_pos, float& x_offset) {
        // Dimension is also directly calculated here
        if (!m_chars->empty()) {
            // auto start_char = m_draw_chars.front();
            // auto end_char = m_draw_chars.back();
            // float ascent = start_char->ascent;
            // float descent = end_char->descent;
            // m_ext_dimensions = Rect{
            //     start_char->_calculated_position.x - start_char->offset.x,
            //     start_char->_calculated_position.y - start_char->offset.y,
            //     end_char->_calculated_position.x - end_char->offset.x + end_char->advance,
            //     end_char->_calculated_position.y - end_char->offset.y + ascent - descent
            // };
            // m_ext_dimensions.w -= m_ext_dimensions.x;
            // m_ext_dimensions.h -= m_ext_dimensions.y;
        }
        return cursor_y_pos;
    }
    void TextString::hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset) {

    }
    bool TextString::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        //ZoneScoped;
        bool ret = true;
        // We do not update cursor_y_pos in text span (taken care of parent block)
            // Draw all backgrounds
        if (m_style.font_bg_color != Colors::transparent) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            int i = 0;
            for (auto i = 0;i < m_chars->size();i++) {
                auto p = (*m_chars)[i];
                auto ptr = static_cast<DrawableCharPtr>(p);
                ImVec2 p_min = cursor_pos + ptr->calculated_position - ptr->info->offset;
                p_min.x += x_offset;
                p_min.y += cursor_y_pos;
                ImVec2 p_max = p_min + ImVec2(ptr->info->advance, ptr->info->ascent - ptr->info->descent);
                draw_list->AddRectFilled(p_min, p_max, m_style.font_bg_color, 0);
                i++;
            }
        }
        // Draw all chars
        // for (auto ptr : m_draw_chars) {
        //     if (!ptr->draw(draw_list, boundaries, ImVec2(x_offset, cursor_y_pos)))
        //         ret = false;
        // }
        return ret;
    }
    void TextString::hk_debug(const std::string&) {
        if (ImGui::TreeNode((std::string("T: ") + type_to_name(m_type) + "##" + std::to_string(m_id)).c_str())) {
            std::string str;
            AB::str_from_text_boundaries(*m_safe_string, str, m_text_boundaries);
            ImGui::TextWrapped("%s", str.c_str());
            ImGui::TreePop();
        }
    }
}