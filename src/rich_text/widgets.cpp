#include "widgets.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    // AbstractWidget
    bool AbstractWidget::buildAndAddChars(std::vector<WrapCharPtr>&, std::vector<DrawableCharPtr>&) { 
       return true;
    }
    void AbstractWidget::draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        m_position.x = x_offset;
        m_position.y = cursor_y_pos;
        for (auto ptr : m_childrens) {
            ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries);
        }
    }
    void AbstractWidget::buildWidget() {}
    void AbstractWidget::setWidth(float width) {
        m_window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }

    // AbstractBlock
    void AbstractBlock::draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        x_offset += m_h_paddings.x;
        m_position.x = x_offset;
        m_position.y = cursor_y_pos;

        // buildWidget uses m_position, so we must call this function after setting m_position
        buildWidget();

        float current_y_pos = cursor_y_pos;
        if (isInsideRectY(m_position, boundaries)) {
            for (auto ptr : m_draw_chars) {
                ptr->draw(draw_list, m_position);
            }
        }
        cursor_y_pos += m_wrapper.getHeight() + m_v_paddings.x;

        for (auto ptr : m_childrens) {
            ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries);
        }

        current_y_pos += m_v_paddings.y;
        m_dimensions.x = m_window_width - m_h_paddings.x - m_h_paddings.y;
        m_dimensions.y = cursor_y_pos - current_y_pos;

        m_show_boundaries = true;
        if (m_show_boundaries) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRect(p_min, p_max, Colors::blue, 1.f);
        }

    }
    void AbstractBlock::buildWidget() {
        if (m_widget_dirty) {
            m_wrap_chars.clear();
            m_draw_chars.clear();

            bool success = true;
            
            bool ret = build_chars();
            if (!ret)
                success = false;

            for (auto ptr : m_childrens) {
                if (ptr->m_category != C_SPAN) {
                    break;
                }
                auto res = ptr->buildAndAddChars(m_wrap_chars, m_draw_chars);
                if (!res) {
                    success = false;
                }
            }
            m_wrapper.clear();
            float internal_size = m_window_width - m_position.x;
            m_wrapper.setWidth(internal_size);
            m_wrapper.setLineSpace(m_line_space);
            m_wrapper.setString(m_wrap_chars);
            if (success)
                m_widget_dirty = false;
        }
    }
    void AbstractBlock::setWidth(float width) {
        float internal_size = width - m_position.x;
        m_wrapper.setWidth(internal_size);
        m_window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }
}