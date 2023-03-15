#include "wrapper.h"
#include <iomanip>

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))

namespace RichText {
    WrapAlgorithm::WrapAlgorithm(float width, float line_space) {
        m_width = width;
        m_line_space = line_space;
    }
    WrapAlgorithm::WrapAlgorithm() {
    }
    WrapAlgorithm::~WrapAlgorithm() {
    }

    inline int WrapAlgorithm::find_line_idx(int cursor_pos) {
        return 0;
    }
    inline int WrapAlgorithm::find_next_line_break(int cursor_pos) {
        return 0;
    }
    inline void WrapAlgorithm::push_char_on_line(WrapCharPtr& c, float* cursor_pos_x) {
        c->_calculated_position.x = *cursor_pos_x + c->offset.x;
        *cursor_pos_x += c->advance;
    }
    inline void WrapAlgorithm::push_new_line(std::list<Line>::iterator& line_it, int cursor_idx, float* cursor_pos_x) {
        m_lines.insert(std::next(line_it), Line{ cursor_idx, 0.f });
        line_it++;
        *cursor_pos_x = 0.f;
    }
    void WrapAlgorithm::recalculate() {
        if (m_width < 1.f) {
            return;
        }
        if (m_string.empty()) {
            return;
        }
        // Initial conditions
        m_lines.clear();
        m_line_positions.clear();
        m_lines.push_back(Line{ 0, 0.f });
        m_line_positions.insert(0);
        m_height = 0.f;

        int start = 0;
        int end = m_string.size() - 1;
        // ==== SECTION 1 ====
        // Calculation of char and horizontal positions

        // The calculation of the chars coordinates must be done in two passes:
        // First horizontal position to determine the line breaks
        // Then vertical position, which depends on the calculated line breaks and chars properties

        // Calculate line breaks
        {
            auto current_line_it = m_lines.begin();
            float cursor_pos_x = 0.f;

            int word_idx = 0;
            float word_pos_x = 0.f;

            // Determining the line break, char by char
            for (int cursor_idx = 0;cursor_idx <= end;cursor_idx++) {
                WrapCharPtr& c = m_string[cursor_idx];

                // If a breakable char follows a white space (break line or breakable)
                // then it should not considered as a breakable char
                if (c->breakable) {
                    word_idx = cursor_idx + 1;
                    word_pos_x = cursor_pos_x + c->advance;
                }
                if (c->is_linebreak) {
                    push_new_line(current_line_it, cursor_idx + 1, &cursor_pos_x);
                    word_idx = cursor_idx + 1;
                    word_pos_x = 0.f;
                    continue;
                }

                float char_width = c->dimensions.x + c->offset.x;

                if (char_width + cursor_pos_x > m_width) {
                    // Current word width: a word width is counted from the last breakable character
                    // or the last line break (position 0), which ever came last
                    float word_width = cursor_pos_x - word_pos_x;

                    push_new_line(current_line_it, cursor_idx, &cursor_pos_x);

                    // In this case, only the char is pushed to the next
                    // line because char + word can't fit on the whole width
                    if (char_width + word_width > m_width) {
                        push_char_on_line(c, &cursor_pos_x);
                    }
                    // Push every single character onto the next line (with the current one)
                    else {
                        // Edge case where a breakable char gets left on the line
                        if (word_idx == cursor_idx + 1) {
                            word_idx = cursor_idx;
                        }
                        float tmp_cursor_idx = 0.f;
                        for (int j = word_idx;j <= cursor_idx;j++) {
                            WrapCharPtr& tmp_c = m_string[j];
                            if (!tmp_c->is_whitespace)
                                push_char_on_line(tmp_c, &tmp_cursor_idx);
                        }
                        current_line_it->start = word_idx;
                        cursor_pos_x = tmp_cursor_idx;
                    }
                    // Have to update word_xxx after manipulations with current word have been made
                    word_idx = current_line_it->start;
                    word_pos_x = 0.f;
                }
                else {
                    push_char_on_line(c, &cursor_pos_x);
                }
            }
            // line_it_end = current_line_it;
        }
        // ==== SECTION 2 ====
        // Calculation of char vertical positions
        float cursor_pos_y = 0.f;
        float max_ascent = 0.f;
        float max_descent = 0.f;
        for (auto current_line_it = m_lines.begin();current_line_it != m_lines.end();current_line_it++) {
            auto next_it = std::next(current_line_it);
            current_line_it->line_pos_y = cursor_pos_y;
            int line_end_idx = m_string.size();
            if (next_it != m_lines.end()) {
                line_end_idx = next_it->start;
            }

            // Find max char height relative to cursor pos
            // ascent is the distance from origin to bearing
            // descent is the distance from origin to bottom of char
            max_ascent = 0.f;
            max_descent = 0.f;
            for (int j = current_line_it->start;j < line_end_idx;j++) {
                WrapCharPtr& c = m_string[j];
                max_ascent = max(max_ascent, c->ascent);
                max_descent = max(max_descent, c->descent);
            }

            for (int j = current_line_it->start;j < line_end_idx;j++) {
                WrapCharPtr& c = m_string[j];
                c->_calculated_position.y = cursor_pos_y + max_ascent - c->ascent + c->offset.y;
            }
            current_line_it->height = max_ascent + max_descent;
            cursor_pos_y += current_line_it->height * m_line_space;
        }
        m_height = cursor_pos_y;// + max_ascent + max_descent;
    }
    void WrapAlgorithm::setString(const std::vector<WrapCharPtr>& string) {
        m_string = string;
        recalculate();
    }
    void WrapAlgorithm::clear() {
        m_string.clear();
        m_lines.clear();
    }
    void WrapAlgorithm::setWidth(float width) {
        m_width = width;
        recalculate();
    }
    void WrapAlgorithm::setLineSpace(float line_space) {
        m_line_space = line_space;
        recalculate();
    }
}