#include "block.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "rich_text/chars/im_char.h"
#include "rich_text/interactive/cursor.h"

#include <algorithm>

#include "profiling.h"

namespace RichText {

    // bool AbstractBlock::hk_build_main(DrawContext* ctx) {
    //     bool ret = true;
    //     // At this point, margins have already been taken into account
    //     hk_build_chars(ctx);
    //     /* This function roughly helps setting width and height from
    //      * pre-delimiters */
    //      // get_line_height_from_delimiters(ctx);

    //      /* There are two offset used in normal blocks:
    //       * - current_offset for delimiter
    //       * - child_offset for all block childrens.
    //       *   These offsets are determined by the pre-delimiters (in set_pre_margins)
    //       *   or the margins of the block
    //       */
    //     m_current_offset = ctx->x_offset;

    //     set_pre_margins(ctx);

    //     /* If true, this helps vertically align all the pre-delimiters */
    //     // if (m_style.align_pre_indent) {
    //     //     float max_offset = ctx->x_offset.getMax();
    //     //     int first_line = m_text_boundaries.front().line_number;
    //     //     int last_line = m_text_boundaries.back().line_number;
    //     //     ctx->x_offset.clear(first_line, last_line);
    //     //     ctx->x_offset += max_offset;
    //     // }
    //     auto child_x_offset = ctx->x_offset;

    //     /* Placing the y cursor is delicate. We may have empty delimiters to show
    //      * before showing some block children. E.g.:
    //      * >        <-- The first delimiter is empty
    //      * >> ab    <-- The sub-quote & p is only shown on the second line
    //      *    cd        cursor_y_pos must be correct. The height of the paragraph
    //      *    ef        determines the position of the next empty delimiter
    //      * >
    //      *  */
    //     auto child_it = m_childrens.begin();
    //     auto bounds_it = m_text_boundaries.begin();
    //     int i = 0;
    //     ctx->x_offset = child_x_offset;
    //     for (auto& ptr : m_childrens) {
    //         if (!ptr->hk_build(ctx)) {
    //             m_widget_dirty |= DIRTY_CHARS;
    //             ret = false;
    //         }
    //         ctx->x_offset = child_x_offset;
    //     }
    //     //     ctx->x_offset = x_offset;
    //     //     // Draw all childrens (spans)
    //     //     for (auto ptr : m_childrens) {
    //     //         if (ptr->m_category != C_SPAN) {
    //     //             break;
    //     //         }
    //     //         if (!ptr->draw(ctx))
    //     //             ret = false;
    //     //         ctx->x_offset = x_offset;
    //     //     }
    //         // Update cursor from wrapper
    //     ctx->cursor_y_pos += m_wrapper.getHeight();

    //     // while (bounds_it != m_text_boundaries.end()) {
    //     //     int line_number = bounds_it->line_number;
    //     //     bool child_drawn = false;
    //     //     float pre_y_pos = ctx->cursor_y_pos;
    //     //     /* Build the child if at this line it exists */
    //     //     if (child_it != m_childrens.end() && (*child_it)->m_text_boundaries.front().line_number == line_number) {
    //     //         ctx->x_offset = child_x_offset;
    //     //         (*child_it)->draw(ctx);
    //     //         child_drawn = true;
    //     //     }

    //     //     /* Display pre-delimiters */
    //     //     if (i < m_pre_delimiters.size()) {
    //     //         if (child_drawn) {
    //     //             while (bounds_it != m_text_boundaries.end() && bounds_it->line_number <= (*child_it)->m_text_boundaries.back().line_number) {
    //     //                 auto& del_info = m_pre_delimiters[i];
    //     //                 // pre_y_pos = (*ctx->lines)[line_number].position;
    //     //                 ret &= draw_pre_line(ctx, del_info, bounds_it->line_number, m_current_offset, pre_y_pos);
    //     //                 bounds_it++;
    //     //                 i++;
    //     //                 if (bounds_it != m_text_boundaries.end())
    //     //                     line_number = bounds_it->line_number;
    //     //                 if (i >= m_pre_delimiters.size())
    //     //                     break;
    //     //             }
    //     //         }
    //     //         else {
    //     //             auto& del_info = m_pre_delimiters[i];
    //     //             ret &= draw_pre_line(ctx, del_info, bounds_it->line_number, m_current_offset, ctx->cursor_y_pos);

    //     //             /* We need to update the y position because no child
    //     //              * has had influence on it */
    //     //             if (!del_info.str.empty()) {
    //     //                 auto& info = del_info.str.front()->info;
    //     //                 float height = info->ascent + info->descent;
    //     //                 height *= m_style.line_space;
    //     //                 ctx->cursor_y_pos += height;
    //     //             }
    //     //             /* We may have empty delimiter (i.e. new line) that still count*/
    //     //             else {
    //     //                 ctx->cursor_y_pos += m_style.font_size.getFloat();
    //     //             }
    //     //             bounds_it++;
    //     //             i++;
    //     //         }
    //     //     }
    //     //     else {
    //     //         bounds_it++;
    //     //     }
    //     //     if (child_drawn) {
    //     //         child_it = std::next(child_it);
    //     //     }
    //     //     if (bounds_it == m_text_boundaries.end()) {
    //     //         break;
    //     //     }
    //     // }

    //     // ret &= hk_draw_secondary(ctx);

    //     return ret;
    // }


    // bool AbstractBlock::hk_draw_main(DrawContext* ctx) {
    //     //ZoneScoped;
    //     bool ret = true;
    //     for (auto& child : m_childrens) {
    //         ret &= child->draw(ctx);
    //     }

    //     ret &= hk_draw_secondary(ctx);

    //     return ret;
    // }

    bool AbstractBlock::draw_pre_line(DrawContext* ctx, DelimiterInfo& del_info, int line_number, const MultiOffset& x_offset, float y_pos) {
        bool ret = true;
        auto pos = ImVec2(x_offset.getOffset(line_number), y_pos);
        /* Compensate for line ascent from child (if necessary) */
        // auto& line = ctx->doc->getLine(line_number);
        // if (!line.sublines.empty()) {
        //     auto& subline = line.sublines.front();
        //     pos.y += subline.max_ascent - del_info.max_ascent;
        // }
        // for (auto ptr : del_info.str) {
        //     auto p = std::static_pointer_cast<DrawableChar>(ptr);
        //     auto& line = ctx->doc->getLine(line_number);
        //     if (line.y_pos > 0.f)
        //         pos.y += line.y_pos;
        //     ret &= p->draw(ctx->draw_list, ctx->boundaries, pos);
        // }
        return ret;
    }

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
    void AbstractBlock::get_line_height_from_delimiters(DrawContext* ctx) {
        float position = ctx->cursor_y_pos;
        int i = 0;
        for (const auto& bounds : m_text_boundaries) {
            if (i < m_pre_delimiters.size()) {
                /* Update line info */
                auto& str = m_pre_delimiters[i].str;
                if (str.empty())
                    continue;
                float max_ascent = 0.f;
                float max_descent = 0.f;
                for (auto ch : str) {
                    if (max_ascent < ch->info->ascent)
                        max_ascent = ch->info->ascent;
                    if (max_descent < ch->info->descent)
                        max_descent = ch->info->descent;
                }
                float height = max_ascent + max_descent;
                height *= m_style.line_space;
                // (*ctx->lines)[bounds.line_number] = LineInfo{
                //     position,
                //     height,
                //     max_ascent,
                //     max_descent
                // };
                position += height;
            }
            i++;
        }
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
            // auto line_it = ctx->lines->find(bounds.line_number);

            // if (line_it != ctx->lines->end()) {
            //     m_pre_delimiters[i].y_pos = line_it->second.position;
            // }
            // else {
            m_pre_delimiters[i].y_pos = ctx->cursor_y_pos;
            // }
            i++;
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

                float x_offset = ctx->x_offset.getOffset(bounds.line_number);
                wrapper.recalculate(&delimiter.str, x_offset);
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

        if (m_widget_dirty & (DIRTY_WIDTH | DIRTY_CHARS)) {
            hk_set_x_cursor(ctx);

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
                ret &= !ptr->hk_build_hlayout(ctx);
            }

            char_success &= hk_build_chars(ctx);

            if (ret)
                m_widget_dirty ^= DIRTY_WIDTH;
            if (char_success)
                m_widget_dirty ^= DIRTY_CHARS;

        }
        return ret;
    }
    bool AbstractBlock::hk_build_vlayout(DrawContext* ctx) {
        bool ret = true;
        return ret;
    }
    bool AbstractBlock::draw(DrawContext* ctx) {
        bool ret = true;

        bool is_visible = is_in_boundaries(ctx->boundaries);
        if (!is_visible) {
            // ctx->cursor_y_pos += m_ext_dimensions.h;
            return true;
        }

        /* Display chars */
        for (auto& pair : m_text_column) {
            float pos = ctx->cursor_y_pos;
            for (auto& ptr : pair.second.chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= !p->draw(ctx->draw_list, ctx->boundaries, m_int_dimensions.getPos());
            }
        }
        for (auto& del_info : m_pre_delimiters) {
            for (auto ptr : del_info.str) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= !p->draw(ctx->draw_list, ctx->boundaries, m_int_dimensions.getPos());
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

     // void AbstractLeafBlock::hk_update_line_info(DrawContext* ctx) {
     //     float position = ctx->cursor_y_pos;
         // ctx->doc->at(this).
         // for (auto& pair : m_chars.getLines()) {
         //     /* Update line info */
         //     auto it = ctx->lines->find(pair.first);
         //     float height = pair.second.line_height;
         //     (*ctx->lines)[pair.first] = LineInfo{
         //         position,
         //         height,
         //         pair.second.first_max_ascent,
         //         pair.second.first_max_descent
         //     };
         //     position += pair.second.line_height;
         // }
     // }
    // bool AbstractLeafBlock::hk_draw_main(DrawContext* ctx) {
    //     //ZoneScoped;
    //     bool ret = true;

    //     hk_build_chars(ctx);

    //     auto x_offset = ctx->x_offset;

    //     // Draw all the chars generated in the block
    //     for (auto& pair : ctx->doc->at(this).getParagraph()) {
    //         auto int_pos = m_int_dimensions.getPos();
    //         int_pos.x = ctx->x_offset.getOffset(pair.first);
    //         for (auto ptr : pair.second.chars) {
    //             auto p = std::static_pointer_cast<DrawableChar>(ptr);
    //             if (!p->draw(ctx->draw_list, ctx->boundaries, int_pos))
    //                 ret = false;
    //         }
    //     }

    //     // hk_update_line_info(ctx);

    //     ctx->x_offset = x_offset;
    //     // Draw all childrens (spans)
    //     for (auto ptr : m_childrens) {
    //         if (ptr->m_category != C_SPAN) {
    //             break;
    //         }
    //         if (!ptr->draw(ctx))
    //             ret = false;
    //         ctx->x_offset = x_offset;
    //     }

    //     // Update cursor from wrapper
    //     ctx->cursor_y_pos += m_wrapper.getHeight();

    //     ret &= hk_draw_secondary(ctx);

    //     return ret;
    // }


    // bool AbstractLeafBlock::hk_build_main(DrawContext* ctx) {
    //     bool ret = true;
    //     ret &= hk_build_chars(ctx);
    //     for (auto& ptr : m_childrens) {
    //         if (!ptr->hk_build(ctx)) {
    //             m_widget_dirty |= DIRTY_CHARS;
    //             ret = false;
    //         }
    //     }
    //     ctx->cursor_y_pos += m_wrapper.getHeight();
    //     return true;
    // }
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
                m_widget_dirty ^= DIRTY_CHARS;

            m_wrapper.clear();
            m_wrapper.setLineSpace(m_style.line_space, false);
            m_wrapper.setTextColumn(&m_text_column, false);
            m_wrapper.setMultiOffset(&ctx->x_offset, false);
        }
        if (m_widget_dirty & DIRTY_WIDTH) {
            float internal_size = m_int_dimensions.w;
            m_wrapper.setWidth(internal_size, false);
            m_widget_dirty ^= DIRTY_WIDTH;
            m_wrapper.setMultiOffset(&ctx->x_offset, false);
        }
        m_wrapper.recalculate();
        return m_widget_dirty;
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
    // bool HiddenSpace::hk_draw_main(DrawContext* ctx) {
    //     //ZoneScoped;
    //     bool ret = true;

    //     hk_build_chars(ctx);

    //     auto x_offset = ctx->x_offset;

    //     auto& paragraph = m_paragraph;

    //     // Draw all the chars generated in the block
    //     for (auto& pair : paragraph.getParagraph()) {
    //         auto int_pos = m_int_dimensions.getPos();
    //         int_pos.x = ctx->x_offset.getOffset(pair.first);
    //         for (auto ptr : pair.second.chars) {
    //             auto p = std::static_pointer_cast<DrawableChar>(ptr);
    //             if (!p->draw(ctx->draw_list, ctx->boundaries, int_pos))
    //                 ret = false;
    //         }
    //     }
    //     if (paragraph.empty()) {
    //         ctx->cursor_y_pos += m_style.font_size.getFloat();
    //     }

    //     // hk_update_line_info(ctx);

    //     return ret;
    // }
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