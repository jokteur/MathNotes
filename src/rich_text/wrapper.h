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
     * The different properties of this struct (advance, bearing, ...)
     * determine how the character will be displayed along the other characters.
     *
     *     <-------advance------>
     *
     *        + is bearing (x,y)
     *
     *           +     *                 \
     *               * *                 |
     *             *   *                 | height
     *     0     *  *  *  *     0        |
     *  origin         *     next char   |
     *                 *      origin     /
     *           <--width->
     */
    struct Character {
    public:

        float advance = 0.f;
        ImVec2 bearing = ImVec2(0.f, 0.f);
        ImVec2 dimensions = ImVec2(0.f, 0.f);
        bool is_linebreak = false;
        bool breakable = false; // If true, means that this char can be used to breakup for a new line
        Alignement alignement = LEFT; // Tries to align the character (possible if no_char_before/after are true)

        // This should be only modified by Textwrapper
        ImVec2 _calculated_position;

        virtual void draw(ImDrawList* im_draw_list) = 0;

        friend class TextWrapper;
    };

    using CharPtr = std::shared_ptr<Character>;

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
        // float line_coord_Y0;
        float height;
    };

    class TextWrapper {
    private:
        std::vector<CharPtr> m_text;

        std::vector<Line> m_lines;
        std::set<int> m_line_positions;

        float m_width;
        float m_line_space;

        /**
         * @brief Returns the index of the line which contains pos
         */
        inline int find_line_idx(int cursor_pos);

        inline void push_char_on_line(CharPtr c, float* cursor_x_coord);
        inline Line* push_new_line(int cursor_pos, float* cursor_x_coord);

        void recalculate(int from = 0);
    public:
        /**
         * @brief Construct a new Text Wrapper object
         *
         * @param width width (in px) of the box
         * @param line_space relative line space: space between lines is calculated as line_height * line_space
         */
        TextWrapper(float width = 100.f, float line_space = 0.3);
        ~TextWrapper();

        void setWidth(float width);
        void setLineSpace(float line_space);

        std::vector<Line> getLines() { return m_lines; }

        void deleteAt(int start, int end = -1);
        void insertAt(const std::vector<CharPtr> string, int position = -1);
        void insertAt(CharPtr& c, int position = -1);
    };
}