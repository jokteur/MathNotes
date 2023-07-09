#include "block.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"
#include "rich_text/interactive/cursor.h"

#include <algorithm>

#include "profiling.h"

namespace RichText {
    void AbstractBlock::hk_draw_background(Draw::DrawList* draw_list) {
        if (m_style.bg_color != Colors::transparent) {
            draw_list->SetCurrentChannel(0);
            auto& dim = m_int_dimensions;
            auto cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 p_min = cursor_pos + dim.getPos();
            ImVec2 p_max = p_min + dim.getDim();
            draw_list->get()->AddRectFilled(p_min, p_max, m_style.bg_color, 5.f);
            draw_list->SetCurrentChannel(1);
        }
    }
    void AbstractBlock::set_pre_margins(DrawContext* ctx) {
        for (auto& pair : m_pre_delimiters) {
            ctx->x_offset.addOffset(pair.first, pair.second.width);
        }
    }
    bool AbstractBlock::hk_build_pre_delimiter_chars(DrawContext* ctx) {
        //ZoneScoped;
        bool success = true;
        m_pre_delimiters.clear();
        if (m_is_selected) {
            WrapAlgorithm wrapper;
            wrapper.setWidth(10000.f, false);
            for (const auto& bounds : m_text_boundaries) {
                m_pre_delimiters[bounds.line_number] = WrapLine{};
                auto& line = m_pre_delimiters[bounds.line_number];
                if (bounds.pre == bounds.beg) {
                    continue;
                }
                success &= Utf8StrToImCharStr(m_ui_state, &line.chars, m_safe_string, bounds.line_number, bounds.pre, bounds.beg, m_special_chars_style, true);
                float x_offset = ctx->x_offset.getOffset(bounds.line_number);
                wrapper.recalculate(&line, x_offset);
            }
        }
        return success;
    }
    // void AbstractBlock::place_text_cursor(DrawContext* ctx, int line_number, int text_pos, float x_pos, const WrapString& chars, TextCursor& cursor) {
        // auto& line_info = (*ctx->lines)[line_number];
        /* Linear search, not ideal but as long as the user doesn't have
         * 1MB of one continuous paragraph, it is fine
         */
         // float last_x_pos = 0.f;
         // float y_pos = line_info.position;
         // float height = line_info.height;
         // bool found_next_char = false;
         // for (auto ch : chars) {
         //     auto info = ch->info;
         //     if (ch->text_position > text_pos) {
         //         x_pos += ch->calculated_position.x;
         //         // height = info->ascent + info->descent;
         //         y_pos = ch->calculated_position.y - (height - ch->info->dimensions.y);
         //         found_next_char = true;
         //         break;
         //     }
         //     last_x_pos = ch->calculated_position.x + info->advance;
         //     // height = info->ascent + info->descent;
         //     y_pos = ch->calculated_position.y - (height - ch->info->dimensions.y);
         //     // height *= m_style.line_space;
         // }
         // if (!found_next_char) {
         //     x_pos += last_x_pos;
         // }
         // cursor.draw(ctx, ImVec2(x_pos, y_pos + line_info.position), height);
     // }
    // void AbstractBlock::hk_draw_text_cursor(DrawContext* ctx) {
    //     for (auto& cursor : *ctx->cursors) {
    //         int text_pos = cursor.getTextPosition();
    //         int i = 0;
    //         for (const auto bounds : m_text_boundaries) {
    //             if (text_pos >= bounds.pre && text_pos < bounds.beg) {
    //                 if (i < m_pre_delimiters.size()) {
    //                     float x_pos = m_current_offset.getOffset(bounds.line_number);
    //                     place_text_cursor(ctx, bounds.line_number, text_pos, x_pos, m_pre_delimiters[i].str, cursor);
    //                     break;
    //                 }
    //                 else {
    //                     float x_pos = ctx->x_offset.getOffset(bounds.line_number);
    //                     place_text_cursor(ctx, bounds.line_number, text_pos, x_pos, WrapString(), cursor);
    //                     break;
    //                 }
    //             }
    //             i++;
    //         }
    //     }
    // }
    void AbstractBlock::hk_set_selected(DrawContext* ctx) {
        set_selected_pre_only(ctx);
    }

    void AbstractBlock::hk_debug_attributes() {
        AbstractElement::hk_debug_attributes();
    }
    bool AbstractBlock::hk_build_hlayout(DrawContext* ctx) {
        bool ret = true;
        bool char_success = true;

        if (m_widget_dirty & (DIRTY_CHARS | DIRTY_WIDTH)) {
            hk_set_x_origin(ctx);

            char_success &= hk_build_pre_delimiter_chars(ctx);

            /* There are two offset used in normal blocks:
             * - current_offset for delimiter
             * - child_offset for all block childrens.
             *   These offsets are determined by the pre-delimiters (in set_pre_margins)
             *   or the margins of the block
             */
            set_pre_margins(ctx);

            /* If true, this helps vertically align all the pre-delimiters */
            if (m_style.align_pre_indent) {
                float max_offset = ctx->x_offset.getMax();
                int first_line = m_text_boundaries.front().line_number;
                int last_line = m_text_boundaries.back().line_number;
                ctx->x_offset.clear(first_line, last_line);
                ctx->x_offset += max_offset;
            }
            auto child_x_offset = ctx->x_offset;

            for (auto ptr : m_childrens) {
                ctx->x_offset = child_x_offset;
                ret &= ptr->hk_build_hlayout(ctx);
            }

            char_success &= hk_build_chars(ctx);

            if (char_success)
                m_widget_dirty &= ~DIRTY_CHARS;
        }
        return ret;
    }

    bool AbstractBlock::hk_build_vlayout(DrawContext* ctx, int line_number) {
        bool ret = true;
        if (m_widget_dirty) {
            float y_offset = 0.f;
            if (m_text_boundaries.front().line_number == line_number || line_number < 0) {
                hk_set_y_origin(ctx);
            }
            if (line_number < 0) {
                for (auto& pair : m_pre_delimiters) {
                    const int current_line_number = pair.first;
                    for (auto ptr : m_childrens) {
                        ptr->hk_build_vlayout(ctx, current_line_number);
                    }
                    if (ctx->lines.find(current_line_number) != ctx->lines.end()) {
                        // Relative position
                        pair.second.relative_y_pos = ctx->lines[current_line_number].position - m_int_dimensions.y;
                    }
                    else {
                        // Relative position
                        pair.second.relative_y_pos = ctx->cursor_y_pos - m_int_dimensions.y;
                        ctx->cursor_y_pos += pair.second.height;
                    }
                }
                hk_set_y_dim(ctx);
                m_widget_dirty &= ~DIRTY_HEIGHT;
            }

            else {
                if (m_pre_delimiters.find(line_number) == m_pre_delimiters.end()) {
                    return true;
                }
                for (auto ptr : m_childrens) {
                    ptr->hk_build_vlayout(ctx, line_number);
                }
                auto& line = m_pre_delimiters[line_number];
                if (ctx->lines.find(line_number) != ctx->lines.end()) {
                    // Relative position
                    line.relative_y_pos = ctx->lines[line_number].position - m_int_dimensions.y;
                }
                else {
                    // Relative position
                    line.relative_y_pos = ctx->cursor_y_pos - m_int_dimensions.y;
                    ctx->lines[line_number];
                    ctx->lines[line_number].position = ctx->cursor_y_pos;
                    ctx->cursor_y_pos += line.height;
                }
                if (m_text_boundaries.back().line_number == line_number) {
                    hk_set_y_dim(ctx);
                    m_widget_dirty &= ~DIRTY_HEIGHT;
                }
            }
        }
        return ret;
    }
    bool AbstractBlock::draw(DrawContext* ctx) {
        bool ret = true;

        bool is_visible = is_in_boundaries(ctx->boundaries);
        if (!is_visible) {
            return true;
        }

        /* Display chars */
        for (auto& pair : m_text_column) {
            auto pos = m_int_dimensions.getPos() + ctx->draw_offset;
            pos.y += pair.second.relative_y_pos;
            for (auto& ptr : pair.second.chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= !p->draw(ctx->draw_list, ctx->boundaries, pos);
            }
        }
        for (auto& pair : m_pre_delimiters) {
            auto pos = m_int_dimensions.getPos() + ctx->draw_offset;
            pos.y += pair.second.relative_y_pos;
            for (auto& ptr : pair.second.chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= !p->draw(ctx->draw_list, ctx->boundaries, pos);
            }
        }
        /* Display childrens */
        for (auto& child : m_childrens) {
            ret &= child->draw(ctx);
        }

        ret &= hk_draw_secondary(ctx);

        return ret;
    }

    /* =========
     * LeafBlock
     * ========= */
    bool AbstractLeafBlock::hk_build_hlayout(DrawContext* ctx) {
        bool ret = true;
        bool char_success = true;

        if (m_widget_dirty & (DIRTY_CHARS | DIRTY_WIDTH)) {
            hk_set_x_origin(ctx);

            /* There are two offset used in normal blocks:
             * - current_offset for delimiter
             * - child_offset for all block childrens.
             *   These offsets are determined by the pre-delimiters (in set_pre_margins)
             *   or the margins of the block
             */
            set_pre_margins(ctx);

            /* If true, this helps vertically align all the pre-delimiters */
            if (m_style.align_pre_indent) {
                float max_offset = ctx->x_offset.getMax();
                int first_line = m_text_boundaries.front().line_number;
                int last_line = m_text_boundaries.back().line_number;
                ctx->x_offset.clear(first_line, last_line);
                ctx->x_offset += max_offset;
            }
            auto child_x_offset = ctx->x_offset;

            for (auto ptr : m_childrens) {
                ctx->x_offset = child_x_offset;
                ret &= ptr->hk_build_hlayout(ctx);
            }

            char_success &= hk_build_chars(ctx);

            if (char_success)
                m_widget_dirty &= ~DIRTY_CHARS;
        }
        return ret;
    }
    bool AbstractLeafBlock::hk_build_vlayout(DrawContext* ctx, int line_number) {
        bool ret = true;
        if (m_widget_dirty & DIRTY_HEIGHT) {
            float y_offset = 0.f;
            if (line_number >= 0 && m_text_boundaries.front().line_number != line_number) {
                return ret;
            }

            hk_set_y_origin(ctx);
            /* Update line infos for parent elements which need to align the delimiters */
            for (auto& pair : m_text_column) {
                const auto& line = pair.second;
                auto& line_info = ctx->lines[pair.first];
                line_info.position = ctx->cursor_y_pos;
                ctx->cursor_y_pos += line.height;
                line_info.height = line.height;
                line_info.ascent = line.sublines.front().max_ascent;
                line_info.descent = line.sublines.front().max_descent;
            }
            hk_set_y_dim(ctx);
            m_widget_dirty &= ~DIRTY_HEIGHT;
        }
        return ret;
    }
    bool AbstractLeafBlock::hk_build_pre_delimiter_chars(DrawContext* ctx) {
        bool success = true;
        for (const auto& bounds : m_text_boundaries) {
            int line_number = bounds.line_number;
            if (bounds.pre < bounds.beg && m_is_selected) {
                success &= Utf8StrToImCharStr(m_ui_state, &m_text_column, m_safe_string, line_number, bounds.pre, bounds.beg, m_special_chars_style, true);
            }
        }
        return success;
    }
    bool AbstractLeafBlock::hk_build_post_delimiter_chars(DrawContext* ctx) {
        bool success = true;
        const auto& bounds = m_text_boundaries.back();
        int line_number = bounds.line_number;
        if (bounds.end < bounds.post && m_is_selected) {
            success &= Utf8StrToImCharStr(m_ui_state, &m_text_column, m_safe_string, line_number, bounds.end, bounds.post, m_special_chars_style, true);
        }
        return success;
    }

    bool AbstractLeafBlock::hk_build_chars(DrawContext* ctx) {
        //ZoneScoped;
        if (m_widget_dirty & DIRTY_CHARS) {
            m_text_column.clear();

            bool success = true;
            if (m_is_selected)
                success &= hk_build_pre_delimiter_chars(ctx);

            for (auto ptr : m_childrens) {
                success &= ptr->add_chars(&m_text_column);
            }
            if (m_is_selected)
                success &= hk_build_post_delimiter_chars(ctx);

            if (success)
                m_widget_dirty &= ~DIRTY_CHARS;

            m_wrapper.clear();
            m_wrapper.setLineSpace(m_style.line_space, false);
            m_wrapper.setTextColumn(&m_text_column, false);
            m_wrapper.setMultiOffset(&ctx->x_offset, false);
        }
        if (m_widget_dirty & DIRTY_WIDTH) {
            float internal_size = m_int_dimensions.w;
            m_wrapper.setWidth(internal_size, false);
            m_widget_dirty &= ~DIRTY_WIDTH;
            m_wrapper.setMultiOffset(&ctx->x_offset, false);
        }
        m_wrapper.recalculate();
        return !(m_widget_dirty & (DIRTY_CHARS | DIRTY_WIDTH));
    }

    /* ===========
     * HiddenSpace
     * =========== */
    bool HiddenSpace::hk_build_chars(DrawContext* ctx) {
        //ZoneScoped;
        // ctx->cursor_y_pos += 1.f;
        if (m_widget_dirty) {
            m_text_column.clear();

            bool success = true;

            if (m_is_selected) {
                auto& bounds = m_text_boundaries.front();
                success &= Utf8StrToImCharStr(m_ui_state, &m_text_column, m_safe_string, bounds.line_number, bounds.pre, bounds.end, m_special_chars_style, true);
            }

            m_wrapper.clear();
            float internal_size = m_window_width - ctx->x_offset.getMax() - m_style.h_margins.y.getFloat();
            m_wrapper.setWidth(internal_size, false);
            m_wrapper.setLineSpace(m_style.line_space, false);
            m_wrapper.setTextColumn(&m_text_column);

            if (success)
                m_widget_dirty = false;
        }
        return m_widget_dirty;
    }
    bool HiddenSpace::add_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        return success;
    }
    void AbstractLeafBlock::hk_draw_text_cursor(DrawContext* ctx) {
        for (auto& cursor : *ctx->cursors) {
            int text_pos = cursor.getTextPosition();
            int i = 0;
            for (const auto bounds : m_text_boundaries) {
                if (text_pos >= bounds.pre && text_pos <= bounds.post) {
                    float x_pos = ctx->x_offset.getOffset(bounds.line_number);
                    // place_text_cursor(ctx, bounds.line_number, text_pos, x_pos, paragraph.getLines()[bounds.line_number].m_chars, cursor);
                    break;
                }
                i++;
            }
        }
    }
    void AbstractLeafBlock::hk_set_selected(DrawContext* ctx) {
        set_selected_all(ctx);
    }
}