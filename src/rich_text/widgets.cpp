#include "widgets.h"
#include "imgui_internal.h"

namespace RichText {
    // AbstractWidget
    bool AbstractWidget::buildAndAddChars(std::vector<WrapCharPtr>&, std::vector<DrawableCharPtr>&) { 
       return true;
    }
    void AbstractWidget::draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset) {
        m_position.x = x_offset;
        m_position.y = cursor_y_pos;
        for (auto ptr : m_childrens) {
            ptr->draw(draw_list, cursor_y_pos, x_offset);
        }
    }
    void AbstractWidget::buildWidget() {}
    void AbstractWidget::setWidth(float width) {
        m_size_props.window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }

    // AbstractBlock
    void AbstractBlock::draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset) {
        x_offset += m_size_props.h_paddings.x;
        m_position.x = x_offset;
        m_position.y = cursor_y_pos;

        // buildWidget uses m_position, so we must call this function after setting m_position
        buildWidget();


        float current_y_pos = cursor_y_pos;
        for (auto ptr : m_draw_chars) {
            ptr->draw(draw_list, m_position);
        }
        cursor_y_pos += m_wrapper.getHeight() + m_size_props.v_paddings.x;

        for (auto ptr : m_childrens) {
            ptr->draw(draw_list, cursor_y_pos, x_offset);
        }

        current_y_pos += m_size_props.v_paddings.y;
        m_size_props.dimensions.x = m_size_props.window_width - m_size_props.h_paddings.x - m_size_props.h_paddings.y;
        m_size_props.dimensions.y = current_y_pos - current_y_pos;
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
            float internal_size = m_size_props.window_width - m_position.x;
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
        m_size_props.window_width = width;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }
}