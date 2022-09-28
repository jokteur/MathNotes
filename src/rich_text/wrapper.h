#pragma once

#include <vector>
#include <list>
#include <tempo.h>

namespace RichText {
    enum Alignement { LEFT, CENTER, RIGHT };

    /**
     * @brief A character can be anything that needs to be
     * displayed to the screen in a text-like widget.
     *
     * The different properties of this struct (advance, offset, ...)
     * determine how the character will be displayed along the other characters.
     *
     *     <-------advance------>
     *
     *     0 →                  0                   ▴
     *   origin                next char            |
     *     ↓     x     *       origin    ▴          |   ascent
     *       offset  * *                 |          |
     *             *   *                 | height   ▾
     *           *  *  *  *              |          ▴
     *                 *                 |          |   descent
     *                 *                 ▾          ▾
     *           <--width->
     */
    struct WrapCharacter {
    public:

        float advance = 0.f;
        ImVec2 offset = ImVec2(0.f, 0.f);
        float ascent = 0.f; // Max ascent of char family
        float descent = 0.f; // Max descent of char family
        ImVec2 dimensions = ImVec2(0.f, 0.f);
        bool is_linebreak = false;
        bool breakable = false; // If true, means that this char can be used to breakup for a new line
        bool is_whitespace = false; // If true, the char won't be pushed onto the next line 
        Alignement alignement = LEFT; // Tries to align the character (possible if no_char_before/after are true)

        // This should be only modified by WrapAlgorithm
        ImVec2 _calculated_position;
        ImVec2 _scroll_offset;
        bool _is_visible;
    };

    using WrapCharPtr = std::shared_ptr<WrapCharacter>;

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
        std::vector<WrapCharPtr> m_string;

        // Calculated quantities
        std::list<Line> m_lines;
        std::set<int> m_line_positions;
        float m_total_height;

        // User set quantities
        float m_width;
        float m_height;
        float m_line_space;

        /**
         * @brief Returns the index of the line which contains pos
         */
        inline int find_line_idx(int cursor_pos);
        inline int find_next_line_break(int cursor_pos);

        inline void push_char_on_line(WrapCharPtr c, float* cursor_x_coord);
        inline void push_new_line(std::list<Line>::iterator& line_it, int cursor_pos, float* cursor_x_coord);

        void recalculate(int from = 0, int to = -1);
    public:
        /**
         * @brief Construct a new Text Wrapper object
         *
         * @param width width (in px) of the box
         * @param height height (in px) of the box
         * @param line_space relative line space: space between lines is calculated as line_height * line_space
         */
        WrapAlgorithm(float width, float height, float line_space = 0.3);
        ~WrapAlgorithm();

        void setString(const std::vector<WrapCharPtr>& string);
        void insertAt(const std::vector<WrapCharPtr>& string, int position = -1);
        void insertAt(WrapCharPtr& c, int position = -1);
        void deleteAt(int start, int end = -1);
        void clear();

        int getCursorIndexFromPosition(ImVec2 coordinates);

        void setWidth(float width);
        void setBoxHeight(float height);
        void setLineSpace(float line_space);
    };
}