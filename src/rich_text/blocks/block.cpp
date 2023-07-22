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
    void AbstractBlock::set_x_margins(DrawContext* ctx) {
        if (m_is_selected || m_style.always_show_pre) {
            for (auto& pair : m_pre_delimiters) {
                ctx->x_offset.addOffset(pair.first, pair.second.width);
            }
        }
    }
    bool AbstractBlock::hk_build_pre_delimiter_chars(DrawContext* ctx) {
        //ZoneScoped;
        bool success = true;
        m_pre_delimiters.clear();
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
        return success;
    }
    void AbstractBlock::hk_draw_text_cursor(DrawContext* ctx) {
        if (!m_style.draw_text_cursor)
            return;
        for (auto& cursor : *ctx->cursors) {
            int text_pos = cursor.getTextPosition();
            for (const auto bounds : m_text_boundaries) {
                auto* text = &m_text_column;
                bool is_at_line_break = text_pos == bounds.post && text_pos < m_safe_string->size() && (*m_safe_string)[bounds.post] == '\n';
                bool is_at_end_of_file = text_pos == bounds.post && text_pos == m_safe_string->size();
                bool is_at_end_of_line = is_at_end_of_file || is_at_line_break;
                bool line_has_children = bounds.beg < bounds.end;
                if (text_pos >= bounds.pre && text_pos < bounds.post
                    /* The next conditions check if we are at the end of a line
                     * or end of the file */
                    || is_at_end_of_line) {
                    ImVec2 pos = m_ext_dimensions.getPos();
                    float height = ctx->line_height * m_style.line_space;
                    if (m_pre_delimiters.find(bounds.line_number) != m_pre_delimiters.end()) {
                        if (text_pos >= bounds.beg && line_has_children) {
                            break;
                        }
                        if (text_pos >= bounds.pre && text_pos < bounds.beg || !line_has_children) {
                            text = &m_pre_delimiters;
                        }
                    }
                    if (text->find(bounds.line_number) != text->end()) {
                        const auto& line = (*text)[bounds.line_number];
                        if (line.sublines.empty())
                            break;
                        // Search for the line in which the char may be found
                        float last_x_pos = 0.f;
                        bool found_next_char = false;
                        auto it = line.sublines.begin();
                        int char_idx = 0;
                        for (auto ch : line.chars) {
                            auto info = ch->info;
                            if (std::next(it) != line.sublines.end()
                                && char_idx >= std::next(it)->start) {
                                it++;
                            }
                            if (ch->text_position) {
                                if (ch->text_position > text_pos) {
                                    pos.x = ch->calculated_position.x;
                                    found_next_char = true;
                                    break;
                                }
                            }
                            char_idx++;
                            last_x_pos = ch->calculated_position.x + info->advance;
                        }
                        if (text == &m_pre_delimiters)
                            pos.y += line.relative_y_pos;
                        else
                            pos.y += it->rel_y_pos;
                        height = it->height;
                        if (!found_next_char) {
                            pos.x = last_x_pos;
                        }
                    }
                    cursor.draw(ctx, pos, height);
                }
            }
        }
    }

    void AbstractBlock::hk_debug_attributes() {
        AbstractElement::hk_debug_attributes();
    }
    bool AbstractBlock::hk_build_hlayout(DrawContext* ctx) {
        bool ret = true;
        if (m_widget_dirty & (DIRTY_CHARS | DIRTY_WIDTH)) {
            hk_set_x_origin(ctx);

            ret &= hk_build_pre_delimiter_chars(ctx);

            /* There are two offset used in normal blocks:
             * - current_offset for delimiter
             * - child_offset for all block childrens.
             *   These offsets are determined by the pre-delimiters (in set_x_margins)
             *   or the margins of the block
             */
            set_x_margins(ctx);

            /* If true, this helps vertically align all the pre-delimiters */
            if (m_style.align_pre_indent && m_is_selected) {
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

            ret &= hk_build_chars(ctx);

            if (ret)
                m_widget_dirty &= ~DIRTY_CHARS;
        }
        return ret;
    }

    bool AbstractBlock::hk_build_vlayout(DrawContext* ctx, int line_number, bool force) {
        bool ret = true;
        if (m_widget_dirty || ctx->force_dirty_height || force) {
            ret = !ctx->force_dirty_height && !(m_widget_dirty & DIRTY_WIDTH);
            int first_line_number = m_text_boundaries.front().line_number;
            int last_line_number = m_text_boundaries.back().line_number;
            if (first_line_number == line_number || line_number < 0)
                hk_set_y_origin(ctx);
            if (line_number < 0) {
                for (auto& pair : m_pre_delimiters) {
                    const int current_line_number = pair.first;
                    for (auto ptr : m_childrens) {
                        ret &= ptr->hk_build_vlayout(ctx, current_line_number, true);
                    }
                    if (ctx->lines.find(current_line_number) != ctx->lines.end()) {
                        pair.second.relative_y_pos = ctx->lines[current_line_number].position - m_int_dimensions.y;
                        if (!pair.second.sublines.empty())
                            pair.second.relative_y_pos += ctx->lines[current_line_number].ascent - pair.second.sublines.front().max_ascent;
                    }
                    else {
                        pair.second.relative_y_pos = ctx->cursor_y_pos - m_int_dimensions.y;
                        ctx->cursor_y_pos += pair.second.height;
                    }
                }
                hk_set_y_dim(ctx);
                if (ret)
                    m_widget_dirty &= ~DIRTY_HEIGHT;
            }
            else {
                if (line_number < first_line_number || line_number > last_line_number)
                    return ret;
                for (auto ptr : m_childrens) {
                    ret &= ptr->hk_build_vlayout(ctx, line_number, true);
                }

                float before_pos = ctx->cursor_y_pos;
                auto* line = &m_pre_delimiters[line_number];
                if (ctx->lines.find(line_number) != ctx->lines.end()) {
                    line->relative_y_pos = ctx->lines[line_number].position - m_int_dimensions.y;
                    if (!line->sublines.empty())
                        line->relative_y_pos += ctx->lines[line_number].ascent - line->sublines.front().max_ascent;
                }
                else {
                    ctx->lines[line_number];
                    ctx->lines[line_number].position = ctx->cursor_y_pos;
                    ctx->lines[line_number].height = line->height;
                    if (line->sublines.empty())
                        ctx->lines[line_number].ascent = line->height;
                    else {
                        ctx->lines[line_number].ascent = line->sublines.front().max_ascent;
                        ctx->lines[line_number].descent = line->sublines.front().max_descent;
                    }
                    ctx->cursor_y_pos += line->height;
                }
                if (last_line_number == line_number) {
                    hk_set_y_dim(ctx);
                    if (ret)
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
            pos.x = 0.f;
            pos.y += pair.second.relative_y_pos;
            for (auto& ptr : pair.second.chars) {
                auto p = std::static_pointer_cast<DrawableChar>(ptr);
                ret &= !p->draw(ctx->draw_list, ctx->boundaries, pos);
            }
        }
        if (m_is_selected || m_style.always_show_pre) {
            for (auto& pair : m_pre_delimiters) {
                auto pos = m_int_dimensions.getPos() + ctx->draw_offset;
                pos.x = 0.f;
                pos.y += pair.second.relative_y_pos;
                for (auto& ptr : pair.second.chars) {
                    auto p = std::static_pointer_cast<DrawableChar>(ptr);
                    ret &= !p->draw(ctx->draw_list, ctx->boundaries, pos);
                }
            }
        }
        /* Display childrens */
        for (auto& child : m_childrens) {
            ret &= child->draw(ctx);
        }
        hk_draw_text_cursor(ctx);
        hk_draw_show_boundaries(ctx);
        ret &= hk_draw_secondary(ctx);

        return ret;
    }
    void AbstractBlock::hk_get_line_info(DrawContext* ctx, int line_number, LineInfo& line_info) {
        bool find_in_child = false;
        if (m_pre_delimiters.find(line_number) != m_pre_delimiters.end()) {
            const auto& line = m_pre_delimiters[line_number];
            line_info.position = line.relative_y_pos; // + m_ext_dimensions.y;
            if (!line.sublines.empty()) {
                line_info.height = line.sublines.front().height;
                line_info.ascent = line.sublines.front().max_ascent;
                line_info.descent = line.sublines.front().max_descent;
            }
            find_in_child = true;
        }
        if (m_text_column.find(line_number) != m_text_column.end()) {
            const auto& line = m_text_column[line_number];
            line_info.position = line.relative_y_pos + m_ext_dimensions.y;
            if (!line.sublines.empty()) {
                line_info.height = line.sublines.front().height;
                line_info.ascent = line.sublines.front().max_ascent;
                line_info.descent = line.sublines.front().max_descent;
            }
            find_in_child = true;
        }
        if (find_in_child) {
            for (auto ptr : m_childrens) {
                if (ptr->m_text_boundaries.front().line_number <= line_number
                    && ptr->m_text_boundaries.back().line_number >= line_number) {
                    ptr->hk_get_line_info(ctx, line_number, line_info);
                    break;
                }
            }
            if (m_childrens.empty()) {
                line_info.position = m_ext_dimensions.y;
                line_info.height = ctx->line_height * m_style.line_space;
                line_info.ascent = line_info.height / 2.f;
                line_info.descent = line_info.height / 2.f;
            }
        }
    }

    /* =========
     * LeafBlock
     * ========= */
    bool AbstractLeafBlock::hk_build_hlayout(DrawContext* ctx) {
        bool ret = true;
        if (m_widget_dirty & (DIRTY_CHARS | DIRTY_WIDTH)) {
            hk_set_x_origin(ctx);

            auto child_x_offset = ctx->x_offset;

            for (auto ptr : m_childrens) {
                ctx->x_offset = child_x_offset;
                ret &= ptr->hk_build_hlayout(ctx);
            }
            ret &= hk_build_chars(ctx);

            if (ret)
                m_widget_dirty &= ~DIRTY_CHARS;
        }
        return ret;
    }
    bool AbstractLeafBlock::hk_build_vlayout(DrawContext* ctx, int line_number, bool force) {
        bool ret = true;
        if (m_widget_dirty & DIRTY_HEIGHT || ctx->force_dirty_height) {
            ret = !ctx->force_dirty_height && !(m_widget_dirty & DIRTY_WIDTH);
            if (line_number >= 0 && m_text_boundaries.front().line_number != line_number) {
                return ret;
            }

            auto child_x_offset = ctx->x_offset;
            for (auto ptr : m_childrens) {
                ctx->x_offset = child_x_offset;
                ret &= ptr->hk_build_hlayout(ctx);
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
            if (!m_has_content) {
                if (ctx->line_height > 0.f)
                    ctx->cursor_y_pos += ctx->line_height * m_style.line_space;
                else
                    ret = false;
            }
            hk_set_y_dim(ctx);
            if (ret)
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
            m_wrapper.setWidth(m_window_width, false);
            if (!(m_widget_dirty & DIRTY_CHARS))
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
        if (m_widget_dirty) {
            m_text_column.clear();

            bool success = true;

            if (m_is_selected) {
                auto& bounds = m_text_boundaries.front();
                success &= Utf8StrToImCharStr(m_ui_state, &m_text_column, m_safe_string, bounds.line_number, bounds.pre, bounds.end, m_special_chars_style, true);
            }
            else {
                if (ctx->line_height > 0.f)
                    ctx->cursor_y_pos += ctx->line_height * m_style.line_space;
                else
                    success = false;
            }

            m_wrapper.clear();
            float internal_size = m_window_width - ctx->x_offset.getMax() - m_style.h_margins.y.getFloat();
            m_wrapper.setWidth(internal_size, false);
            m_wrapper.setLineSpace(m_style.line_space, false);
            m_wrapper.setTextColumn(&m_text_column, false);
            m_wrapper.recalculate();

            if (success) {
                m_widget_dirty &= ~DIRTY_CHARS;
                m_widget_dirty &= ~DIRTY_WIDTH;
            }
        }
        return m_widget_dirty;
    }
    bool HiddenSpace::add_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        bool success = true;
        return success;
    }
}