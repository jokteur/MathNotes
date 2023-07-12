#pragma once

#include "page_memory.h"
#include "ui/scrollbar.h"
#include "ui/drawable.h"
#include "ui/scrollbar.h"

#include "chars/wrapper.h"

namespace RichText {
    struct PrevElementInfo {
        int prev_top_block_idx = -1;
        bool event = false;
        float prev_top_block_shift = 0.f;
        Rect prev_top_block_ext_dimensions;
    };

    class PageDisplay : public Drawable {
    private:
        PageMemory* m_mem;

        Draw::DrawList m_draw_list;
        float m_current_width = 0.f;
        float m_scale = 1.f;

        bool m_redo_positions = false;
        bool m_recalculate_line_height = true;
        int m_line_lookahead_window = 2000;

        float m_y_displacement = 0.f;
        float m_prev_y_displacement = 0.f;
        bool m_scrollbar_grab = false;

        float m_display_height = 0.f;
        float m_approximate_min_height = 0.f;
        float m_line_height = 10.f;
        float m_before_height = 0.f;
        float m_after_height = 0.f;
        float m_total_height = 0.f;

        // Config
        float m_min_scroll_height = 10.f;

        // Debug
        bool m_freeze_frame = false;
        bool m_continue = false;

        std::string m_window_name;

        VerticalScrollBar m_scrollbar;

        void calculate_heights();
        void manage_scroll(const ImVec2& mouse_pos, const Rect& box);
        void set_and_check_width(PrevElementInfo* prev_info, float width);
        void set_displacement(DrawContext* ctx);
        void build(DrawContext* ctx, PrevElementInfo* prev_top_block_shift);
        void correct_displacement(PrevElementInfo* info);
        void display_scrollbar(const Rect& boundaries, bool deactivate);
    public:
        PageDisplay(PageMemory* memory);

        void debugInfo();
        void FrameUpdate(const std::string& window_name, DrawContext* context);

        void scrollDown(float pixels);
        void scrollUp(float pixels);
        void gotoLine(int line_number);

        void setMinScrollHeight(float height) { m_min_scroll_height = height; }
    };
}