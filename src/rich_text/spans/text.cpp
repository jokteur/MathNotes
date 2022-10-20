#include "text.h"
#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    TextString::TextString(UIState_ptr ui_state) : AbstractSpan(ui_state) {
        m_type = T_TEXT;
    }

    bool TextString::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        m_draw_chars.clear();

        using namespace Fonts;
        FontRequestInfo font_request;
        font_request.font_styling = m_style.font_styling;
        font_request.size_wish = m_style.font_size;

        FontInfoOut font_out;
        m_ui_state->font_manager.requestFont(font_request, font_out);
        auto font = Tempo::GetImFont(font_out.font_id);

        if (m_safe_string == nullptr || font->im_font == nullptr) {
            return false;
        }

        float font_size = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();
        for (int i = 0;i < m_processed_text.size();i++) {
            unsigned int c = (unsigned int)m_processed_text[i];
            if (c >= 0x80) {
                ImTextCharFromUtf8(&c, &(m_processed_text[i]), &(m_processed_text[m_processed_text.size() - 1]));
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            bool force_breakable = false;
            if (c == ',' || c == '|' || c == '-' || c == '.' || c == '!' || c == '?')
                force_breakable = true;
            auto char_ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)c, font_size, m_style.font_color, force_breakable);
            m_draw_chars.push_back(std::static_pointer_cast<DrawableChar>(char_ptr));
            wrap_chars.push_back(std::static_pointer_cast<WrapCharacter>(char_ptr));
        }
        return true;
    }
    float TextString::hk_set_position(float& cursor_y_pos, float& x_offset) {
        // Dimension is also directly calculated here
        if (!m_draw_chars.empty()) {
            auto start_char = m_draw_chars.begin()->get();
            auto end_char = (m_draw_chars.end() - 1)->get();
            float ascent = start_char->ascent;
            float descent = end_char->descent;
            m_position = ImVec2(
                start_char->_calculated_position.x - start_char->offset.x,
                start_char->_calculated_position.y - start_char->offset.y
            );
            m_dimensions = ImVec2(
                end_char->_calculated_position.x - end_char->offset.x + end_char->advance,
                end_char->_calculated_position.y - end_char->offset.y + ascent - descent
            );
            m_dimensions -= m_position;
        }
        return cursor_y_pos;
    }
    void TextString::hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset) {

    }
    void TextString::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        // We do not update cursor_y_pos in text span (taken care of parent block)
        if (isInsideRectY(m_position, boundaries)) {
            // Draw all backgrounds
            if (m_style.font_bg_color != Colors::transparent) {
                auto cursor_pos = ImGui::GetCursorScreenPos();
                int i = 0;
                for (auto ptr : m_draw_chars) {
                    ImVec2 p_min = cursor_pos + ptr->_calculated_position - ptr->offset;
                    p_min.x += x_offset;
                    p_min.y += cursor_y_pos;
                    ImVec2 p_max = p_min + ImVec2(ptr->advance, ptr->ascent - ptr->descent);
                    draw_list->AddRectFilled(p_min, p_max, m_style.font_bg_color, 0);
                    i++;
                }
            }
            // Draw all chars
            for (auto ptr : m_draw_chars) {
                ptr->draw(draw_list, ImVec2(x_offset, cursor_y_pos));
            }
        }
    }
}