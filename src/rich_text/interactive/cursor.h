#pragma once

#include "ui/drawable.h"
#include "rich_text/page_memory.h"

namespace RichText {
    class TextCursor : public Drawable {
    private:
        int m_start = 0;
        int m_end = -1;
        int m_current_line = 0;
        ImVec2 position;
        float height;

        const int LEFT = 0x1;
        const int RIGHT = 0x2;
        const int UP = 0x4;
        const int DOWN = 0x8;
        int m_key_presses = 0;

        /* Access to Page data */
        AB::File* m_file;
        PageMemory* m_mem;

        void go_down();
        void go_up();
        void go_left(bool ctrl);
        void go_right(bool ctrl);

        bool is_cursor_in_memory();

        void manage_events();

    public:
        TextCursor(PageMemory* page_memory, AB::File* file) : Drawable(), m_mem(page_memory), m_file(file) {}

        void setPosition(int start, int end = -1);

        int getCurrentLine() const { return m_current_line; }
        int getTextPosition() const;
        int getStartPosition() const { return m_start; }
        int getEndPosition() const { return m_end; }

        void manage();
    };
}
