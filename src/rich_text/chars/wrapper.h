#pragma once

#include <vector>
#include <list>
#include <tempo.h>

#include "fonts/char.h"

namespace RichText {
    struct WrapCharacter {
        virtual ~WrapCharacter() = default;

        Fonts::Character* info;
        ImVec2 calculated_position;
    };

    typedef std::shared_ptr<WrapCharacter> WrapCharPtr;
    typedef std::weak_ptr<WrapCharacter> WrapCharWPtr;

    typedef std::vector<WrapCharPtr> WrapString;

    struct WrapLine {
        WrapString m_chars;
        float line_height = 0.f;
        float first_max_ascent = 0.f;
        float first_max_descent = 0.f;
    };

    class WrapParagraph {
    private:
        std::map<int, WrapLine> m_lines;
    public:
        void push_back(const WrapCharPtr& ptr, int line) {
            if (m_lines.find(line) == m_lines.end()) {
                m_lines[line] = WrapLine();
            }
            m_lines[line].m_chars.push_back(ptr);
        }
        void clear() { m_lines.clear(); }

        bool empty() const { return m_lines.empty(); }

        std::map<int, WrapLine>& getLines() { return m_lines; }
        bool isCharInParagraph(int line_number, int char_number) const {
            return m_lines.find(line_number) != m_lines.end() &&
                char_number < m_lines.at(line_number).m_chars.size();
        }
        WrapCharPtr getChar(int line_number, int char_number) const {
            return m_lines.at(line_number).m_chars.at(char_number);
        }
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
        WrapParagraph* m_paragraph = nullptr;
        WrapString* m_current_string;

        // Calculated quantities
        std::list<Line> m_lines;
        std::set<int> m_line_positions;
        float m_total_height;

        // User set quantities
        std::vector<float> m_widths = { 1.f };
        float m_width;
        float m_height = 0.f;
        float m_line_space = 1.3f;
        float m_first_max_ascent = 0.f;
        float m_first_max_descent = 0.f;

        /**
         * @brief Returns the index of the line which contains pos
         */
        inline int find_line_idx(int cursor_pos);
        inline int find_next_line_break(int cursor_pos);

        inline void push_char_on_line(WrapCharPtr c, float* cursor_x_coord);
        inline void push_new_line(std::list<Line>::iterator& line_it, int cursor_pos, float* cursor_x_coord);

        void algorithm();
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

        // void setString(WrapString* string, bool redo = true);
        void setParagraph(WrapParagraph* paragraph, bool redo = true);
        void clear();
        void recalculate();
        void recalculate(WrapString* string);

        const std::list<Line>& getLines() { return m_lines; }
        float getHeight() { return m_height; }
        float getFirstMaxAscent() { return m_first_max_ascent; }
        float getFirstMaxDescent() { return m_first_max_descent; }

        void setWidth(float width, bool redo = true);
        void setWidth(const std::vector<float>& width, bool redo = true);
        void setLineSpace(float line_space, bool redo = true);
    };
}