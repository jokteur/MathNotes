#include "element.h"

#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "rich_text/chars/im_char.h"
#include "interactive/cursor.h"
#include "ab/ab_file.h"

#include "profiling.h"
#include "time_counter.h"

namespace RichText {
    int AbstractElement::count = 0;
    int AbstractElement::visible_count = 0;

    /* =========
     * Debugging
     * ========= */
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
    /* ========
     * Building
     * ======== */
    bool AbstractElement::add_chars(WrapColumn*) {
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
    void AbstractElement::hk_set_y_origin(DrawContext* ctx) {
        m_ext_dimensions.y = ctx->cursor_y_pos;
        /* Margins */
        ctx->cursor_y_pos += m_style.v_margins.x.getFloat();
        m_int_dimensions.y = ctx->cursor_y_pos;
        /* Paddings */
        ctx->cursor_y_pos += m_style.v_paddings.x.getFloat();
    }
    void AbstractElement::hk_set_x_origin(DrawContext* ctx) {
        m_ext_dimensions.x = ctx->x_offset.getMin();

        /* Margins */
        if (!m_is_selected)
            ctx->x_offset += m_style.h_margins.x.getFloat();

        if (!m_is_selected)
            m_int_dimensions.x = ctx->x_offset.getMin();

        /* Paddings */
        if (!m_is_selected)
            ctx->x_offset += m_style.h_paddings.x.getFloat();

        /* Width */
        float w = m_window_width - m_style.h_paddings.y.getFloat() - m_style.h_margins.y.getFloat();
        m_int_dimensions.w = w - m_int_dimensions.x;
        m_ext_dimensions.w = m_window_width - m_ext_dimensions.x;
    }
    void AbstractElement::hk_set_y_dim(DrawContext* ctx) {
        ctx->cursor_y_pos += m_style.v_paddings.y.getFloat();
        float h = ctx->cursor_y_pos - m_int_dimensions.y + m_style.h_paddings.y.getFloat();
        m_int_dimensions.h = h;
        ctx->cursor_y_pos += m_style.v_margins.y.getFloat();
        m_ext_dimensions.h = ctx->cursor_y_pos - m_ext_dimensions.y;
    }

    bool AbstractElement::hk_build_hlayout(DrawContext* ctx) {
        bool ret = true;
        if (m_widget_dirty & (DIRTY_WIDTH | DIRTY_CHARS)) {
            hk_set_x_origin(ctx);
            auto child_x_offset = ctx->x_offset;

            for (auto ptr : m_childrens) {
                ctx->x_offset = child_x_offset;
                ret &= !ptr->hk_build_hlayout(ctx);
            }

            if (ret) {
                m_widget_dirty &= ~DIRTY_WIDTH;
            }
        }
        return ret;
    }
    bool AbstractElement::hk_build_vlayout(DrawContext* ctx) {
        bool ret = true;
        if (m_widget_dirty & DIRTY_HEIGHT) {
            hk_set_y_origin(ctx);

            float y_offset = ctx->cursor_y_pos;
            for (auto ptr : m_childrens) {
                ret &= !ptr->hk_build_vlayout(ctx);
            }
            hk_set_y_dim(ctx);

            if (ret) {
                m_widget_dirty &= ~DIRTY_HEIGHT;
            }
        }
        return ret;
    }
    void AbstractElement::resetYOrigin() {
        for (auto ptr : m_childrens) {
            ptr->resetYOrigin();
        }
        m_widget_dirty |= DIRTY_HEIGHT;
    }

    // void AbstractElement::hk_set_dimensions(DrawContext* ctx, float last_y_pos) {
    //     if (m_category == C_BLOCK && ctx->cursor_y_pos - last_y_pos == 0.f) {
    //         for (const auto& bounds : m_text_boundaries) {
    //             // auto& set = ctx->doc->getWidgetsOnLine(bounds.line_number);
    //             // if (set.empty())
    //             //     continue;
    //             // auto ptr = *set.begin();
    //             // ctx->cursor_y_pos += ptr->m_ext_dimensions.h;
    //         }
    //     }
    //     ctx->cursor_y_pos += m_style.v_paddings.y.getFloat();
    //     float w = m_window_width - m_style.h_paddings.y.getFloat() - m_style.h_margins.y.getFloat();
    //     w -= m_int_dimensions.x;

    //     m_int_dimensions.w = w;
    //     m_int_dimensions.h = ctx->cursor_y_pos - m_int_dimensions.y + m_style.h_paddings.x.getFloat();

    //     ctx->cursor_y_pos += m_style.v_margins.y.getFloat();
    //     /* h margin x and h padding x got added to x_offset in hk_set_position,
    //      * which we must re-add to have the correct width */
    //     w = m_window_width - m_ext_dimensions.x;
    //     m_ext_dimensions.w = w;
    //     m_ext_dimensions.h = ctx->cursor_y_pos - m_ext_dimensions.y;

    //     m_is_dimension_set = true;
    // }
    bool AbstractElement::hk_build_chars(DrawContext*) { return true; }
    bool AbstractElement::hk_build(DrawContext* ctx) {
        bool ret = true;
        float initial_y_pos = ctx->cursor_y_pos;
        // hk_set_position(ctx->cursor_y_pos, ctx->x_offset);
        if (m_widget_dirty) {
            hk_build_hlayout(ctx);
            hk_build_vlayout(ctx);
            // hk_set_selected(ctx);
            // if (!hk_build_main(ctx)) {
            //     m_widget_dirty |= DIRTY_CHARS;
            //     ret = false;
            // }
            // hk_set_dimensions(ctx, initial_y_pos);
            // hk_draw_background(ctx->draw_list);
            // hk_draw_text_cursor(ctx); 
        }
        // ctx->cursor_y_pos += m_ext_dimensions.h;

        // if (m_no_y_update) {
        //     m_no_y_update = false;
        //     ctx->cursor_y_pos = initial_y_pos;
        // }
        return ret;
    }

    void AbstractElement::set_selected_all(DrawContext* ctx) {
        // bool is_selected = false;
        // for (const auto& cursor : *ctx->cursors) {
        //     int start = cursor.getStartPosition();
        //     int end = cursor.getEndPosition();

        //     int i = 0;
        //     for (const auto bounds : m_text_boundaries) {
        //         if (start >= bounds.pre && start <= bounds.post || end >= bounds.pre && end <= bounds.post) {
        //             is_selected = true;
        //             break;
        //         }
        //     }
        //     if (is_selected)
        //         break;
        // }
        // if (m_is_selected != is_selected) {
        //     m_is_selected = is_selected;
        //     m_widget_dirty = ALL_DIRTY;
        //     auto ptr = m_parent;
        //     while (ptr != nullptr) {
        //         ptr->m_widget_dirty = ALL_DIRTY;
        //         ptr = ptr->m_parent;
        //     }
        // }
    }
    void AbstractElement::set_selected_pre_only(DrawContext* ctx) {
        // bool is_selected = false;
        // for (const auto& cursor : *ctx->cursors) {
        //     int start = cursor.getStartPosition();
        //     int end = cursor.getEndPosition();

        //     int i = 0;
        //     for (const auto bounds : m_text_boundaries) {
        //         if (start >= bounds.pre && start <= bounds.beg || end >= bounds.pre && end <= bounds.beg) {
        //             is_selected = true;
        //             break;
        //         }
        //     }
        //     if (is_selected)
        //         break;
        // }
        // if (m_is_selected != is_selected) {
        //     m_is_selected = is_selected;
        //     m_widget_dirty = ALL_DIRTY;
        //     auto ptr = m_parent;
        //     while (ptr != nullptr) {
        //         ptr->m_widget_dirty = ALL_DIRTY;
        //         ptr = ptr->m_parent;
        //     }
        // }
    }
    void AbstractElement::set_selected_never(DrawContext*) {
        m_is_selected = false;
    }
    void AbstractElement::hk_set_selected(DrawContext* ctx) {
        set_selected_all(ctx);
    }
    void AbstractElement::setWindowWidth(float width) {
        //ZoneScoped;
        if (m_window_width == width) {
            m_widget_dirty &= ~DIRTY_WIDTH;
            return;
        }
        m_window_width = width;
        m_widget_dirty |= DIRTY_WIDTH | DIRTY_HEIGHT;
        for (auto ptr : m_childrens) {
            ptr->setWindowWidth(width);
        }
    }

    /* =======
     * Drawing
     * ======= */
    void AbstractElement::hk_draw_background(Draw::DrawList* draw_list) {

    }
    bool AbstractElement::hk_draw_secondary(DrawContext*) { return true; }
    void AbstractElement::hk_draw_show_boundaries(DrawContext* ctx) {
        const auto& ext_dim = m_ext_dimensions;
        // if (m_show_boundaries && (isInsideRectY(ext_dim.y, boundaries) || isInsideRectY(ext_dim.y + ext_dim.y, boundaries))) {
        auto cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 p_min = cursor_pos + ext_dim.getPos();
        ImVec2 p_max = p_min + ext_dim.getDim();
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB((float)m_tree_level / 6.f, 1, 0.7, r, g, b);
        (*ctx->draw_list)->AddRect(p_min, p_max, ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.f)));
        // }
    }
    void AbstractElement::hk_draw_text_cursor(DrawContext* ctx) {

    }
    bool AbstractElement::draw(DrawContext* ctx) {
        bool ret = true;

        bool is_visible = is_in_boundaries(ctx->boundaries);
        if (!is_visible) {
            return true;
        }

        /* Display chars */
        for (auto& pair : m_text_column) {
            auto pos = m_int_dimensions.getPos() + ctx->draw_offset;;
            pos.y += pair.second.y_pos;
            for (auto& ptr : pair.second.chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= p->draw(ctx->draw_list, ctx->boundaries, pos);
            }
        }

        /* Display childrens */
        for (auto& child : m_childrens) {
            ret &= child->draw(ctx);
        }

        hk_draw_show_boundaries(ctx);
        ret &= hk_draw_secondary(ctx);

        return ret;
    }
}