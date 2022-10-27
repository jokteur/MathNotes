#include "widgets.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    // AbstractWidget
    bool AbstractWidget::add_chars(std::vector<WrapCharPtr>&) {
        return true;
    }
    float AbstractWidget::hk_set_position(float& cursor_y_pos, float& x_offset) {
        x_offset += m_style.h_margins.x;
        cursor_y_pos += m_style.v_margins.y;

        m_position.x = x_offset;
        m_position.y = cursor_y_pos;
        float current_y_pos = cursor_y_pos;

        x_offset += m_style.h_paddings.x;
        cursor_y_pos += m_style.v_paddings.x;
        return current_y_pos;
    }
    void AbstractWidget::hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset) {
        cursor_y_pos += m_style.v_paddings.y;
        m_dimensions.x = m_window_width - x_offset;
        m_dimensions.y = cursor_y_pos - last_y_pos;

        cursor_y_pos += m_style.v_margins.y;
    }
    void AbstractWidget::hk_build_widget(float x_offset) {

    }
    void AbstractWidget::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        ImVec2 padding_before(m_style.h_paddings.x, m_style.v_paddings.x);

        if (isInsideRectY(m_position, boundaries)) {
            for (auto ptr : m_draw_chars) {
                ptr->draw(draw_list, m_position + padding_before);
            }
        }
        for (auto& ptr : m_childrens) {
            ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries);
        }
    }
    void AbstractWidget::hk_draw_background(Draw::DrawList& draw_list) {

    }
    void AbstractWidget::hk_draw_show_boundaries(Draw::DrawList& draw_list) {
        if (m_show_boundaries) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRect(p_min, p_max, Colors::blue);
        }
    }

    void AbstractWidget::draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        float last_y_pos = hk_set_position(cursor_y_pos, x_offset);
        hk_build_widget(x_offset);
        hk_draw_main(draw_list, cursor_y_pos, x_offset, boundaries);
        hk_set_dimensions(last_y_pos, cursor_y_pos, x_offset);
        hk_draw_background(draw_list);
        hk_draw_show_boundaries(draw_list);
    }
    void AbstractWidget::setWidth(float width) {
        m_window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }

    // Blocks
    void AbstractBlock::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        ImVec2 padding_before(m_style.h_paddings.x, m_style.v_paddings.x);

        // Draw all the chars generated by the block
        if (isInsideRectY(m_position, boundaries)) {
            for (auto ptr : m_draw_chars) {
                ptr->draw(draw_list, m_position + padding_before);
            }
        }
        // Draw all childrens (spans)
        for (auto ptr : m_childrens) {
            if (ptr->m_category != C_SPAN) {
                break;
            }
            ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries);
        }
        // Update cursor from wrapper
        cursor_y_pos += m_wrapper.getHeight() + m_style.v_margins.x;
        // Draw all childrens (blocks)
        for (auto ptr : m_childrens) {
            if (ptr->m_category == C_BLOCK)
                ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries);
        }

        m_show_boundaries = true;
    }
    void AbstractBlock::hk_draw_background(Draw::DrawList& draw_list) {
        if (m_style.bg_color != Colors::transparent) {
            draw_list.SetCurrentChannel(0);
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRectFilled(p_min, p_max, m_style.bg_color, 5.f);
        }
    }
    void AbstractBlock::hk_build_widget(float x_offset) {
        if (m_widget_dirty) {
            m_wrap_chars.clear();
            m_draw_chars.clear();

            bool success = true;

            for (auto ptr : m_childrens) {
                if (ptr->m_category != C_SPAN) {
                    break;
                }
                auto res = ptr->add_chars(m_wrap_chars);
                if (!res) {
                    success = false;
                }
            }

            m_wrapper.clear();
            float internal_size = m_window_width - x_offset - m_style.h_paddings.x - m_style.h_paddings.y;
            m_wrapper.setWidth(internal_size);
            m_wrapper.setLineSpace(m_style.line_space);
            m_wrapper.setString(m_wrap_chars);

            if (success)
                m_widget_dirty = false;
        }
    }
    void AbstractBlock::setWidth(float width) {
        float internal_size = m_dimensions.x - m_style.h_paddings.x - m_style.h_paddings.y;
        m_wrapper.setWidth(internal_size);
        m_window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }


    // Spans
    bool InterText::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        m_draw_chars.clear();

        if (m_is_selected) {
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_draw_chars, m_safe_string, m_raw_text_info.begin, m_raw_text_info.end, m_special_chars_style);
            if (!res) {
                success = false;
            }
        }
        return success;
    }

    // Spans
    bool AbstractSpan::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        m_draw_chars.clear();

        if (m_is_selected) {
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_draw_chars, m_safe_string, m_raw_text_info.pre, m_raw_text_info.begin, m_special_chars_style);
            if (!res) {
                success = false;
            }
        }

        for (auto ptr : m_childrens) {
            auto res = ptr->add_chars(wrap_chars);
            if (!res) {
                success = false;
            }
        }
        if (m_is_selected) {
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_draw_chars, m_safe_string, m_raw_text_info.end, m_raw_text_info.post, m_special_chars_style);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
}