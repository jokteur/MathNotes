#pragma once

#include "ui/drawable.h"
#include "rich_text/element.h"

namespace RichText {
    class TextCursor : public Drawable {
    private:
        int m_start = 0;
        int m_end = -1;
        int m_current_line = 0;
        AbstractElementPtr m_current_ptr;
        ImVec2 position;
        float height;

        /* Access to Page data */
        std::map<int, RootNodePtr>* m_root_elements;
        AB::File* m_file;

        void go_down();
        void go_up();
        void go_left(bool ctrl);
        void go_right(bool ctrl);

        void manage_events();

    public:
        TextCursor() : Drawable() {}

        void setPosition(int start, int end = -1);

        int getCurrentLine() { return m_current_line; }
        int getTextPosition();
        int getStartPosition() { return m_start; }
        int getEndPosition() { return m_end; }

        void draw(std::map<int, RootNodePtr>* root_elements, AB::File* file);
    };
}
