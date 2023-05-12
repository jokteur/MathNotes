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
    int AbstractElement::visible_count = 0;

    MultiOffset& MultiOffset::operator+=(float offset) {
        for (auto& pair : m_offsets) {
            pair.second += offset;
        }
        m_min += offset;
        m_max += offset;
        return *this;
    }
    MultiOffset& MultiOffset::operator-=(float offset) {
        for (auto& pair : m_offsets) {
            pair.second -= offset;
        }
        m_min -= offset;
        m_max -= offset;
        return *this;
    }
    float MultiOffset::getOffset(int line_number) const {
        if (m_offsets.find(line_number) == m_offsets.end()) {
            return 0.f;
        }
        return m_offsets.at(line_number);
    }
    void MultiOffset::addOffset(int line_number, float offset) {
        if (m_offsets.find(line_number) == m_offsets.end()) {
            m_offsets[line_number] = 0.f;
        }
        m_offsets[line_number] += offset;

        m_min = 1e9;
        m_max = -1e9;
        for (auto& pair : m_offsets) {
            if (pair.second < m_min)
                m_min = pair.second;
            if (pair.second > m_max)
                m_max = pair.second;
        }
    }
    void MultiOffset::clear() {
        m_offsets.clear();
        m_min = 1e9;
        m_max = -1e9;
    }
    void MultiOffset::clear(const std::vector<int>& line_numbers) {
        m_offsets.clear();
        for (auto line_number : line_numbers) {
            m_offsets[line_number] = 0.f;
        }
        m_min = 0.f;
        m_max = 0.f;
    }
    void MultiOffset::clear(int from, int to) {
        m_offsets.clear();
        for (int i = from;i <= to;i++) {
            m_offsets[i] = 0.f;
        }
        m_min = 0.f;
        m_max = 0.f;
    }

    float MultiOffset::getMin() const {
        if (m_offsets.empty())
            return 0.f;
        return m_min;
    }
    float MultiOffset::getMax() const {
        if (m_offsets.empty())
            return 0.f;
        return m_max;
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
        ImGui::SameLine();
        if (m_is_dimension_set) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::green), " / dim set");
        }
        else {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), " / dim not set");
        }
        ImGui::SameLine();
        if (m_widget_dirty) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), " / dirty");
        }
        else {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::green), " / clean");
        }

        /* Position and dimensions */
        std::stringstream pos, dimension;
        pos << "Position: x=" << m_ext_dimensions.x << " y=" << m_ext_dimensions.y;
        dimension << "Dimension: x=" << m_ext_dimensions.w << " y=" << m_ext_dimensions.h;
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
                m_ui_state.font_manager.requestFont(font_request, font_out);
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
    bool AbstractElement::add_chars(WrapParagraph*) {
        return true;
    }

    AbstractElement::AbstractElement() {
        count++;
    }
    AbstractElement::~AbstractElement() {
        for (auto ptr : m_childrens) {
            delete ptr;
        }
        count--;
    }
    bool AbstractElement::is_in_boundaries(const Rect& b) {
        const auto& dims = m_ext_dimensions;
        return isInsideRectY(dims.y, b) || isInsideRectY(dims.y + dims.h, b)
            || b.y > dims.y && b.y + b.h < dims.y + dims.h;
    }
    float AbstractElement::hk_set_position(float& cursor_y_pos, MultiOffset& x_offset) {
        m_ext_dimensions.x = x_offset.getMin();
        m_ext_dimensions.y = cursor_y_pos;

        // for (auto offset)
        if (!m_is_selected)
            x_offset += m_style.h_margins.x.getFloat();
        cursor_y_pos += m_style.v_margins.x.getFloat();

        if (!m_is_selected)
            m_int_dimensions.x = x_offset.getMin();
        m_int_dimensions.y = cursor_y_pos;

        float current_y_pos = cursor_y_pos;

        if (!m_is_selected)
            x_offset += m_style.h_paddings.x.getFloat();
        cursor_y_pos += m_style.v_paddings.x.getFloat();
        return current_y_pos;
    }
    void AbstractElement::hk_set_dimensions(DrawContext* ctx, float last_y_pos) {
        if (m_category == C_BLOCK && ctx->cursor_y_pos - last_y_pos == 0.f) {
            for (const auto& bounds : m_text_boundaries) {
                ctx->cursor_y_pos += (*ctx->lines)[bounds.line_number].height;
            }
        }
        ctx->cursor_y_pos += m_style.v_paddings.y.getFloat();
        float w = m_window_width - m_style.h_paddings.y.getFloat() - m_style.h_margins.y.getFloat();
        w -= m_int_dimensions.x;

        m_int_dimensions.w = w;
        m_int_dimensions.h = ctx->cursor_y_pos - m_int_dimensions.y + m_style.h_paddings.x.getFloat();

        ctx->cursor_y_pos += m_style.v_margins.y.getFloat();
        /* h margin x and h padding x got added to x_offset in hk_set_position,
         * which we must re-add to have the correct width */
        w = m_window_width - m_ext_dimensions.x;
        m_ext_dimensions.w = w;
        m_ext_dimensions.h = ctx->cursor_y_pos - m_ext_dimensions.y;

        m_is_dimension_set = true;
    }
    bool AbstractElement::hk_build_widget(DrawContext*) { return true; }
    void AbstractElement::hk_update_line_info(DrawContext*) {}

    bool AbstractElement::hk_draw_main(DrawContext* ctx) {
        //ZoneScoped;
        bool ret = true;
        for (auto& pair : m_chars.getLines()) {
            float pos = ctx->cursor_y_pos;
            (*ctx->lines)[pair.first] = LineInfo{ pos , 0.f };
            for (auto ptr : pair.second.m_chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                if (!p->draw(ctx->draw_list, ctx->boundaries, m_int_dimensions.getPos()))
                    ret = false;
            }
            (*ctx->lines)[pair.first].height = ctx->cursor_y_pos - pos;
        }
        auto x_offset = ctx->x_offset;
        for (auto& ptr : m_childrens) {
            if (!ptr->draw(ctx))
                ret = false;
            ctx->x_offset = x_offset;
        }
        return ret;
    }
    void AbstractElement::hk_draw_background(Draw::DrawList* draw_list) {

    }
    void AbstractElement::hk_draw_show_boundaries(Draw::DrawList* draw_list, const Rect& boundaries) {
        const auto& ext_dim = m_ext_dimensions;
        if (m_show_boundaries && (isInsideRectY(ext_dim.y, boundaries) || isInsideRectY(ext_dim.y + ext_dim.y, boundaries))) {
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + ext_dim.getPos();
            ImVec2 p_max = p_min + ext_dim.getDim();
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB((float)m_tree_level / 6.f, 1, 0.7, r, g, b);
            (*draw_list)->AddRect(p_min, p_max, ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.f)));
        }
    }
    bool AbstractElement::draw(DrawContext* ctx) {
        //ZoneScoped;
        bool ret = true;
        float initial_y_pos = ctx->cursor_y_pos;
        hk_set_position(ctx->cursor_y_pos, ctx->x_offset);
        m_is_visible = is_in_boundaries(ctx->boundaries);
        if (m_is_visible || !m_is_dimension_set || m_widget_dirty) {
            visible_count++;
            if (!hk_draw_main(ctx)) {
                m_widget_dirty |= DIRTY_CHARS;
                ret = false;
            }
            hk_set_dimensions(ctx, initial_y_pos);
            hk_draw_show_boundaries(ctx->draw_list, ctx->boundaries);
            hk_draw_background(ctx->draw_list);
        }
        else {
            ctx->cursor_y_pos += m_ext_dimensions.h;
        }

        if (m_no_y_update) {
            m_no_y_update = false;
            ctx->cursor_y_pos = initial_y_pos;
        }
        return ret;
    }
    void AbstractElement::setWindowWidth(float width) {
        //ZoneScoped;
        if (m_window_width == width) {
            m_widget_dirty ^= DIRTY_WIDTH;
            return;
        }
        m_window_width = width;
        m_widget_dirty |= DIRTY_WIDTH;
        for (auto ptr : m_childrens) {
            ptr->setWindowWidth(width - m_style.h_paddings.y.getFloat() - m_style.h_margins.y.getFloat());
        }
    }
}