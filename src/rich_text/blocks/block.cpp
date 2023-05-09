#include "block.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"

#include <algorithm>

#include "profiling.h"

namespace RichText {

    bool AbstractBlock::hk_draw_main(DrawContext* ctx) {
        //ZoneScoped;
        bool ret = true;
        // At this point, margins have already been taken into account

        hk_build_widget(ctx);
        get_line_height_from_delimiters(ctx);

        auto x_offset = ctx->x_offset;

        /* Set pre-margins must be called before displaying any subsequent child. This
         * way, we have the correct margins when displaying pre delimiters */
        set_pre_margins(ctx);

        // if (m_style.pre_indent)

        // ctx->x_offset += m_pre_max_width;

        float y_pos = ctx->cursor_y_pos;

        /* Placing the y cursor is delicate. We may have empty delimiters to show */
        for (auto ptr : m_childrens) {
            if (ptr->m_category == C_BLOCK)
                ret &= ptr->draw(ctx);
            ctx->x_offset = x_offset;
        }

        /* Draw all the pre-delimiters
         * These must be displayed after calling the children, which have determined the
         * y positions of these pre-chars
         */
        float new_y_pos = ctx->cursor_y_pos;
        auto new_x_offset = ctx->x_offset;
        ctx->cursor_y_pos = y_pos;
        ctx->x_offset = x_offset;

        ret &= hk_draw_pre_chars(ctx);

        ctx->cursor_y_pos = new_y_pos;
        ctx->x_offset = new_x_offset;

        // if (m_style.pre_indent)
        // ctx->x_offset -= m_pre_max_width;

        return ret;
    }

    void AbstractBlock::hk_draw_background(Draw::DrawList* draw_list) {
        if (m_style.bg_color != Colors::transparent) {
            draw_list->SetCurrentChannel(0);
            // auto cursor_pos = ImGui::GetCursorScreenPos();
            // ImVec2 p_min = cursor_pos + dims.getPos();
            // ImVec2 p_max = cursor_pos + p_min + dims.getDim();
            // draw_list->AddRectFilled(p_min, p_max, m_style.bg_color, 5.f);
        }
    }
    void AbstractBlock::get_line_height_from_delimiters(DrawContext* ctx) {
        float position = ctx->cursor_y_pos;
        int i = 0;
        for (const auto& bounds : m_text_boundaries) {
            if (i < m_pre_delimiters.size()) {
                /* Update line info */
                auto& str = m_pre_delimiters[i].str;
                if (str.empty())
                    continue;
                auto& info = str.front()->info;
                float height = info->ascent + info->descent;
                height *= m_style.line_space;
                (*ctx->lines)[bounds.line_number] = LineInfo{
                    position,
                    height,
                    info->ascent,
                    info->descent
                };
                position += height;
            }
            i++;
        }
    }
    bool AbstractBlock::hk_build_widget(DrawContext* ctx) {
        //ZoneScoped;
        if (m_widget_dirty) {
            bool result = hk_build_pre_delimiter_chars(ctx);
            if (result)
                m_widget_dirty ^= DIRTY_CHARS;
        }
        return true;
    }
    void AbstractBlock::set_pre_margins(DrawContext* ctx) {
        int i = 0;
        for (auto& bounds : m_text_boundaries) {
            /* Draw pre delimiters */
            if (i < m_pre_delimiters.size()) {
                ctx->x_offset.addOffset(bounds.line_number, m_pre_delimiters[i].width);
            }
            i++;
        }
    }
    void AbstractBlock::set_pre_y_position(DrawContext* ctx) {
        int i = 0;
        for (const auto& bounds : m_text_boundaries) {
            if (i >= m_pre_delimiters.size())
                break;
            auto line_it = ctx->lines->find(bounds.line_number);

            if (line_it != ctx->lines->end()) {
                m_pre_delimiters[i].y_pos = line_it->second.position;
            }
            else {
                m_pre_delimiters[i].y_pos = ctx->cursor_y_pos;
            }
            i++;
        }
    }
    bool AbstractBlock::hk_build_pre_delimiter_chars(DrawContext* ctx) {
        //ZoneScoped;
        bool success = true;
        if (m_is_selected) {
            m_pre_delimiters.clear();

            WrapAlgorithm wrapper;
            wrapper.setWidth(4000.f, false);
            for (const auto& bounds : m_text_boundaries) {
                m_pre_delimiters.push_back(DelimiterInfo{});
                auto& delimiter = m_pre_delimiters.back();
                if (bounds.pre == bounds.beg) {
                    continue;
                }
                bool res = Utf8StrToImCharStr(m_ui_state, &delimiter.str, m_safe_string, bounds.line_number, bounds.pre, bounds.beg, m_special_chars_style, true);
                if (!res) {
                    success = false;
                    continue;
                }
                wrapper.recalculate(&delimiter.str);
                auto last_char = delimiter.str.back();
                /* Delimiter y position will be calculated later, after setWidth of wrapper in children
                 * has been called
                 */
                delimiter.width = last_char->calculated_position.x + last_char->info->advance;
                delimiter.max_ascent = wrapper.getFirstMaxAscent();
                m_pre_max_width = std::max(m_pre_max_width, delimiter.width);
            }
        }
        return success;
    }

    bool AbstractBlock::hk_draw_pre_chars(DrawContext* ctx) {
        bool ret = true;
        int i = 0;
        set_pre_y_position(ctx);
        for (const auto& bounds : m_text_boundaries) {
            /* Draw pre delimiters */
            if (i < m_pre_delimiters.size()) {
                auto pre_chars = m_pre_delimiters[i];
                auto pos = ImVec2(ctx->x_offset.getOffset(bounds.line_number), pre_chars.y_pos);
                pos.y += (*ctx->lines)[bounds.line_number].ascent - pre_chars.max_ascent;
                for (auto ptr : pre_chars.str) {
                    auto p = std::static_pointer_cast<DrawableChar>(ptr);
                    if (!p->draw(ctx->draw_list, ctx->boundaries, pos))
                        ret = false;
                }
            }
            i++;
        }
        return ret;
    }

    void AbstractBlock::hk_debug_attributes() {
        AbstractElement::hk_debug_attributes();
    }

    void AbstractLeafBlock::hk_update_line_info(DrawContext* ctx) {
        float position = ctx->cursor_y_pos;
        for (auto& pair : m_chars.getLines()) {
            /* Update line info */
            auto it = ctx->lines->find(pair.first);
            float height = pair.second.line_height;
            (*ctx->lines)[pair.first] = LineInfo{
                position,
                pair.second.line_height,
                pair.second.first_max_ascent,
                pair.second.first_max_descent
            };
            position += pair.second.line_height;
        }
    }
    bool AbstractLeafBlock::hk_draw_main(DrawContext* ctx) {
        //ZoneScoped;
        bool ret = true;

        hk_build_widget(ctx);
        get_line_height_from_delimiters(ctx);

        auto x_offset = ctx->x_offset;

        set_pre_margins(ctx);

        // Draw all the chars generated in the block
        for (auto& pair : m_chars.getLines()) {
            auto int_pos = m_int_dimensions.getPos();
            int_pos.x = ctx->x_offset.getOffset(pair.first);
            for (auto ptr : pair.second.m_chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                if (!p->draw(ctx->draw_list, ctx->boundaries, int_pos))
                    ret = false;
            }
        }

        hk_update_line_info(ctx);

        ctx->x_offset = x_offset;
        ret &= hk_draw_pre_chars(ctx);

        // ctx->x_offset += m_pre_max_width;
        ctx->x_offset = x_offset;
        // Draw all childrens (spans)
        for (auto ptr : m_childrens) {
            if (ptr->m_category != C_SPAN) {
                break;
            }
            if (!ptr->draw(ctx))
                ret = false;
            ctx->x_offset = x_offset;
        }

        // Update cursor from wrapper
        ctx->cursor_y_pos += m_wrapper.getHeight();
        // Draw all childrens (blocks)
        for (auto ptr : m_childrens) {
            if (ptr->m_category == C_BLOCK)
                if (!ptr->draw(ctx))
                    ret = false;
        }
        // ctx->x_offset -= m_pre_max_width;
        return ret;
    }

    bool AbstractLeafBlock::hk_build_widget(DrawContext* ctx) {
        //ZoneScoped;
        if (m_widget_dirty) {
            if (m_widget_dirty & DIRTY_CHARS) {
                m_chars.clear();

                bool success = true;
                success &= hk_build_pre_delimiter_chars(ctx);
                for (auto ptr : m_childrens) {
                    if (ptr->m_category != C_SPAN) {
                        break;
                    }
                    auto res = ptr->add_chars(&m_chars);
                    if (!res) {
                        success = false;
                    }
                }
                if (success)
                    m_widget_dirty ^= DIRTY_CHARS;

                m_wrapper.clear();
                m_wrapper.setLineSpace(m_style.line_space, false);
                m_wrapper.setParagraph(&m_chars, false);
            }
            if (m_widget_dirty & DIRTY_WIDTH) {
                float internal_size = m_window_width - ctx->x_offset.getMin() - m_style.h_margins.y.getFloat();
                m_wrapper.setWidth(internal_size, false);

                m_widget_dirty ^= DIRTY_WIDTH;
            }
            m_wrapper.recalculate();
        }
        return m_widget_dirty;
    }

    /* ===========
     * HiddenSpace
     * =========== */
    bool HiddenSpace::hk_build_widget(DrawContext* ctx) {
        //ZoneScoped;
        ctx->cursor_y_pos += 1.f;
        if (m_widget_dirty) {
            m_chars.clear();

            bool success = true;

            success = add_chars(&m_chars);

            m_wrapper.clear();
            float internal_size = m_window_width - ctx->x_offset.getMax() - m_style.h_margins.y.getFloat();
            m_wrapper.setWidth(internal_size, false);
            m_wrapper.setLineSpace(m_style.line_space, false);
            m_wrapper.setParagraph(&m_chars);

            float position = ctx->cursor_y_pos;
            for (auto pair : m_chars.getLines()) {
                (*ctx->lines)[pair.first] = LineInfo{ position, pair.second.line_height };
                position += pair.second.line_height;
            }

            if (success)
                m_widget_dirty = false;
        }
        return m_widget_dirty;
    }
    bool HiddenSpace::add_chars(WrapParagraph* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        m_chars.clear();
        m_is_selected = true;

        if (m_is_selected) {
            auto& bounds = m_text_boundaries.front();
            auto res = Utf8StrToImCharStr(m_ui_state, wrap_chars, m_safe_string, bounds.line_number, bounds.pre, bounds.end, m_special_chars_style, true);
            if (!res) {
                success = false;
            }
        }
        return success;
    }
}