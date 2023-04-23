#include "element.h"

#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "rich_text/chars/im_char.h"
#include "ab/ab_file.h"

#include "profiling.h"
#include "time_counter.h"

namespace RichText {
    // AbstractElement
    int AbstractElement::count = 0;
    bool AbstractElement::add_chars(std::vector<WrapCharPtr>&) {
        return true;
    }
    AbstractElement::~AbstractElement() {
        if (!m_is_root)
            for (auto ptr : m_childrens) {
                delete ptr;
            }
        count--;
    }
    bool AbstractElement::is_in_boundaries(const Rect& b) {
        return isInsideRectY(m_position.y, b) || isInsideRectY(m_position.y + m_dimensions.y, b)
            || b.y > m_position.y && b.y + b.h < m_position.y + m_dimensions.y;
    }
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
        float prev_height = m_dimensions.y;
        cursor_y_pos += m_style.v_paddings.y;
        m_dimensions.x = m_window_width - x_offset;
        m_dimensions.y = cursor_y_pos - last_y_pos;

        // if ((m_dimensions.y - prev_height) > 1e-1) {
        //     Tempo::SkipFrame();
        // }

        cursor_y_pos += m_style.v_margins.y;
        m_is_dimension_set = true;
    }
    bool AbstractElement::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        //ZoneScoped;
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
        if (m_show_boundaries && (isInsideRectY(m_position.y, boundaries) || isInsideRectY(m_position.y + m_dimensions.y, boundaries))) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + m_position;
            ImVec2 p_max = cursor_pos + m_position + m_dimensions;
            draw_list->AddRect(p_min, p_max, Colors::blue);
        }
    }

    void AbstractElement::hk_debug_attributes() {
        /* State */
        ImGui::Text("Dirty state %u", m_widget_dirty);
        ImGui::Checkbox("Is selected", &m_is_selected);

        /* Display */
        ImGui::Checkbox("Show boundaries", &m_show_boundaries);
        ImGui::Text("Display status: ");
        ImGui::SameLine();
        if (m_is_visible) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::green), "visible");
        }
        else {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), "hidden");
        }

        /* Position and dimensions */
        std::stringstream pos, dimension;
        pos << "Position: x=" << m_position.x << " y=" << m_position.y;
        dimension << "Dimension: x=" << m_dimensions.x << " y=" << m_dimensions.y;
        ImGui::TextUnformatted(pos.str().c_str());
        ImGui::TextUnformatted(dimension.str().c_str());
    }
    void AbstractElement::hk_debug(const std::string& prefix) {
        std::string cat = "B: ";
        if (m_category == C_SPAN)
            cat = "S: ";
        else if (m_category == C_TEXT)
            cat = "T: ";
        std::string str;
        AB::str_from_text_boundaries(*m_safe_string, str, m_text_boundaries);

        int length = str.length();
        std::string suffix = str;
        if (length > 10) {
            suffix = suffix.substr(0, 10) + "...";
        }
        suffix = "  /" + suffix;


        if (ImGui::TreeNode((prefix + cat + type_to_name(m_type) + suffix + "##" + std::to_string(m_id)).c_str())) {
            if (ImGui::TreeNode("Class attributes")) {
                hk_debug_attributes();
                ImGui::TreePop();
            }
            if (!m_childrens.empty())
                if (ImGui::TreeNode("Children")) {
                    for (auto ptr : m_childrens) {
                        ptr->hk_debug("");
                    }
                    ImGui::TreePop();
                }
            if (ImGui::TreeNode("Content")) {
                using namespace Fonts;
                FontRequestInfo font_request;
                font_request.font_styling.family = F_MONOSPACE;
                FontInfoOut font_out;
                m_ui_state->font_manager.requestFont(font_request, font_out);
                Tempo::PushFont(font_out.font_id);
                ImGui::TextWrapped("%s", str.c_str());
                Tempo::PopFont();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Text boundaries")) {
                for (auto bounds : m_text_boundaries) {
                    std::string bound_text;
                    bound_text += "Line: " + std::to_string(bounds.line_number);
                    bound_text += " Pre: " + std::to_string(bounds.pre);
                    bound_text += " beg: " + std::to_string(bounds.beg);
                    bound_text += " end: " + std::to_string(bounds.end);
                    bound_text += " post: " + std::to_string(bounds.post);
                    ImGui::Text("%s", bound_text.c_str());
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    bool AbstractElement::draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        //ZoneScoped;
        bool ret = true;
        float initial_y_pos = cursor_y_pos;
        float last_y_pos = hk_set_position(cursor_y_pos, x_offset);
        m_is_visible = is_in_boundaries(boundaries);
        if (m_is_visible || !m_is_dimension_set || m_widget_dirty & DIRTY_CHARS) {
            auto& timers = TimeCounter::getInstance();
            if (!hk_draw_main(draw_list, cursor_y_pos, x_offset, boundaries)) {
                m_widget_dirty |= DIRTY_CHARS;
                ret = false;
            }
            hk_set_dimensions(last_y_pos, cursor_y_pos, x_offset);
        }
        else {
            cursor_y_pos += m_dimensions.y;
        }
        // hk_draw_background(draw_list);
        hk_draw_show_boundaries(draw_list, cursor_y_pos, boundaries);
        if (m_no_y_update) {
            m_no_y_update = false;
            cursor_y_pos = initial_y_pos;
        }
        return ret;
    }
    void AbstractElement::setWindowWidth(float width) {
        //ZoneScoped;
        if (m_window_width == width)
            return;
        m_window_width = width;
        m_widget_dirty |= DIRTY_WIDTH;
        for (auto ptr : m_childrens) {
            ptr->setWindowWidth(width);
        }
    }
}