#pragma once

#include "ui/drawable.h"
#include "element.h"


namespace RichText {

    typedef int WidgetId;
    struct PageConfig {
        float line_start;
        bool interactive = false;
        float min_scroll_height = 10.f;
    };

    class PageManager;

    class Page: public Drawable {
    private:
        bool m_redo_positions = false;
        bool m_recalculate_line_height = true;

        PageConfig m_config;
        AB::File* m_file;
        int m_block_idx_start = 1e9;
        int m_block_idx_end = -1;
        friend class PageManager;

        Draw::DrawList m_draw_list;
        float m_current_width = 0.f;
        float m_scale = 1.f;

        /* Here, current_xxx designates the element that should
         * be drawn at the top of the widget */
        AbstractElementPtr m_current_block_ptr = nullptr;
        int m_current_block_idx = -1;
        /* Current_line is the first line corresponding in the raw text of the current element */
        int m_current_line = 0.f;
        float m_y_displacement = 0.f;

        int m_line_lookahead_window = 2000;

        float m_display_height = 0.f;
        float m_approximate_min_height = 0.f;
        float m_line_height = 10.f;
        float m_before_height = 0.f;
        float m_after_height = 0.f;

        /* Debug infos */
        int m_debug_root_min = 0;
        int m_debug_root_max = 100;

        std::map<int, AbstractElementPtr> m_root_elements;
        std::mutex m_root_mutex;
        std::unordered_set<Tempo::jobId> m_current_jobs;

        void display_scrollbar(const Rect& boundaries);
        void manage_scroll(const ImVec2& mouse_pos, const Rect& box);
        void calculate_heights();
        void manage_elements();
        void debug_window();

        void manage_jobs();
        void parse_job(int start_idx, int end_idx);

        void find_current_ptr();
        std::map<int, AbstractElementPtr>::iterator find_prev_ptr();
        std::map<int, AbstractElementPtr>::iterator find_next_ptr();
        void go_to_line(int line_number);
        void scroll_up(float pixels);
        void scroll_down(float pixels);
    public:
        Page(UIState_ptr ui_state): Drawable(ui_state) {}
        Page(const Page&) = delete;

        ~Page();
        Page& operator= (const Page&) = delete;
        void draw();
    };
    typedef std::shared_ptr<Page> WidgetPtr;
}