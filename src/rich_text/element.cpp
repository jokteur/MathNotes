#include "element.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    // AbstractElement
    bool AbstractElement::add_chars(std::vector<WrapCharPtr>&) {
        return true;
    }
    AbstractElement::~AbstractElement() {
        count--;
    }
    int AbstractElement::count = 0;
    float AbstractElement::hk_set_position(float& cursor_y_pos, float& x_offset) {
        x_offset += m_style.h_margins.x;
        cursor_y_pos += m_style.v_margins.y;

        m_position.x = x_offset;
        m_position.y = cursor_y_pos;
        float current_y_pos = cursor_y_pos;

        x_offset += m_style.h_paddings.x;
        cursor_y_pos += m_style.v_paddings.x;
        return current_y_pos;
    }
    void AbstractElement::hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset) {
        cursor_y_pos += m_style.v_paddings.y;
        m_dimensions.x = m_window_width - x_offset;
        m_dimensions.y = cursor_y_pos - last_y_pos;

        cursor_y_pos += m_style.v_margins.y;
        m_is_dimension_set = true;
    }
    bool AbstractElement::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        bool ret = true;
        ImVec2 padding_before(m_style.h_paddings.x, m_style.v_paddings.x);

        for (auto ptr : m_draw_chars) {
            if (!ptr->draw(draw_list, boundaries, m_position + padding_before))
                ret = false;
        }
        for (auto& ptr : m_childrens) {
            if (!ptr->draw(draw_list, cursor_y_pos, x_offset, boundaries))
                ret = false;
        }
        return ret;
    }
    void AbstractElement::hk_draw_background(Draw::DrawList& draw_list) {

    }
    void AbstractElement::hk_draw_show_boundaries(Draw::DrawList& draw_list, float cursor_y_pos, const Rect& boundaries) {
        if (m_show_boundaries && isInsideRectY(m_position.y, boundaries) && isInsideRectY(m_position.y + m_dimensions.y + m_dimensions.y, boundaries)) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRect(p_min, p_max, Colors::blue);
        }
    }

    bool AbstractElement::draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        bool ret = true;
        float last_y_pos = hk_set_position(cursor_y_pos, x_offset);
        if (!hk_draw_main(draw_list, cursor_y_pos, x_offset, boundaries))
            ret = false;
        hk_set_dimensions(last_y_pos, cursor_y_pos, x_offset);
        hk_draw_background(draw_list);
        hk_draw_show_boundaries(draw_list, cursor_y_pos, boundaries);
        return ret;
    }
    void AbstractElement::setWidth(float width) {
        m_window_width = width;
        m_is_dimension_set = false;
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
    }
}