#pragma once

#include "ui/drawable.h"
#include "element.h"
#include "interactive/cursor.h"
#include "page_memory.h"
#include "page_display.h"

namespace RichText {

    typedef int WidgetId;
    struct PageConfig {
        float line_start;
        bool interactive = false;
        float min_scroll_height = 10.f;
    };

    class PageManager;

    class Page : public Drawable {
    private:
        PageConfig m_config;
        AB::File* m_file;
        // int m_block_idx_start = 1e9;
        // int m_block_idx_end = -1;
        friend class PageManager;

        /* Debug infos */
        int m_debug_root_min = 0;
        int m_debug_root_max = 100;

        std::string m_name;
        std::string m_window_name;

        std::vector<TextCursor> m_text_cursors;

        PageMemory m_mem;
        PageDisplay m_display;

        void debug_window();

        void manage_cursors();

        // void go_to_line(int line_number);
        // void scroll_up(float pixels);
        // void scroll_down(float pixels);
    public:
        Page(AB::File* file);
        Page(const Page&) = delete;

        void setName(const std::string& name) { m_name = name; }
        void setLine(int line_number);

        ~Page();
        Page& operator= (const Page&) = delete;
        void FrameUpdate() override;
    };
    typedef std::shared_ptr<Page> WidgetPtr;
}