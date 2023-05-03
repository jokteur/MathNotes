#pragma once

#include <vector>
#include <list>
#include <tempo.h>

#include "char.h"

namespace Fonts {
    struct WrapCharacter {
        Character* c;
        ImVec2 calculated_position;
    };

    typedef WrapCharacter* WrapCharPtr;

    class WrapString {
    private:
        std::vector<WrapCharPtr> m_string;
    public:

        template<class T>
        void push_back(T c) {

        }

        void clear();
    };

    /**
     * @brief The convention for line positions is as follow:
     *
     * Let's use the following text as an example. Intentional line breaks
     * have been marked as \n, auto wrapped line breaks are marked as \nn
     *  ____________________________________
     * | This text is a dummy example of\nn | Line 1, position 0 - 31
     * | a wrapped line break.\n            | Line 2, position 31 - 52
     * | \n                                 | Line 3, position 53 - 53
     * | The text ends here.                | Line 4, position 54 - 73
     *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
     * \n does count as a char, but \nn doesn't (as it is dynamically calculated)
     *
     * \nn line break positions: 31
     *  \n line break positions: 52, 53
     */
    struct Line {
        int start;
        float line_pos_y;
        float height;
    };

    /**
     * @brief Only handles wrapping and scrolling
     */
    class WrapAlgorithm {
    private:
        std::vector<WrapCharacter> m_string;

        // Calculated quantities
        std::list<Line> m_lines;
        std::set<int> m_line_positions;
        float m_total_height;

        // User set quantities
        float m_width = 1.f;
        float m_height = 0.f;
        float m_line_space = 1.3f;

        /**
         * @brief Returns the index of the line which contains pos
         */
        inline int find_line_idx(int cursor_pos);
        inline int find_next_line_break(int cursor_pos);

        inline void push_char_on_line(WrapCharPtr c, float* cursor_x_coord);
        inline void push_new_line(std::list<Line>::iterator& line_it, int cursor_pos, float* cursor_x_coord);

    public:
        /**
         * @brief Construct a new Text Wrapper object
         *
         * @param width width (in px) of the box
         * @param line_space relative line space: space between lines is calculated as line_height * line_space
         */
        WrapAlgorithm(float width, float line_space = 1.3);
        WrapAlgorithm();
        ~WrapAlgorithm();

        void setString(const std::vector<WrapCharPtr>& string, bool redo = true);
        void clear();
        void recalculate();

        const std::list<Line>& getLines() { return m_lines; }
        float getHeight() { return m_height; }

        void setWidth(float width, bool redo = true);
        void setLineSpace(float line_space, bool redo = true);
    };
}