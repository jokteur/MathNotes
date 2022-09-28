#include "wrapper.h"
#include <iomanip>

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))

namespace RichText {
    WrapAlgorithm::WrapAlgorithm(float width, float height, float line_space) {
        m_width = width;
        m_height = height;
        m_line_space = line_space;

        m_lines.push_back(Line{ 0, 0.f });
        m_line_positions.insert(0);
    }
    WrapAlgorithm::~WrapAlgorithm() {

    }

    inline int WrapAlgorithm::find_line_idx(int cursor_pos) {
        // m_line_positions.find(cursor_pos);
        return 0;
    }
    inline int WrapAlgorithm::find_next_line_break(int cursor_pos) {
        return 0;
    }
    inline void WrapAlgorithm::push_char_on_line(WrapCharPtr c, float* cursor_pos_x) {
        c->_calculated_position.x = *cursor_pos_x + c->offset.x;
        *cursor_pos_x += c->advance;
    }
    inline void WrapAlgorithm::push_new_line(std::list<Line>::iterator& line_it, int cursor_idx, float* cursor_pos_x) {
        m_lines.insert(std::next(line_it), Line{ cursor_idx, 0.f });
        line_it++;
        *cursor_pos_x = 0.f;
    }
    void WrapAlgorithm::recalculate(int start, int end, float offset_subsequent_lines) {
        if (m_width == 0.f) {
            return;
        }
        // abreviations used in this function:
        // it for iterator, pos for position, idx for index
        if (end == -1) {
            if (m_string.empty())
                end = 0;
            else
                end = m_string.size() - 1;
        }
        // ==== SECTION 1 ====
        // We need to figure out which lines are modified from start to end
        // This part is in O(# lines), but usually we have that # lines << # chars

        // Find the line that contains start position
        auto line_it_start = m_lines.begin();
        while (line_it_start != m_lines.end()) {
            auto next_it = std::next(line_it_start);
            if (next_it != m_lines.end() && next_it->start > start) {
                break;
            }
            if (next_it == m_lines.end())
                break;
            line_it_start++;
        }

        // Find the line that contains end position
        std::list<Line>::iterator line_it_end = line_it_start;
        while (line_it_end != m_lines.end()) {
            line_it_end++;
            if (line_it_end != m_lines.end() && line_it_end->start > end) {
                break;
            }
        }
        float start_to_end_height = 0.f;
        if (line_it_end != m_lines.end()) {
            start_to_end_height = line_it_end->line_pos_y - line_it_start->line_pos_y;
        }
        // All lines from start (not included) to end are invalid
        // Remove them
        if (line_it_start != line_it_end) {
            m_lines.erase(std::next(line_it_start), line_it_end);
        }

        // ==== SECTION 2 ====
        // Calculation of char and horizontal positions

        // The calculation of the chars coordinates must be done in two passes:
        // First horizontal position to determine the line breaks
        // Then vertical position, which depends on the calculated line breaks and chars properties

        // Calculate line breaks
        {
            auto current_line_it = line_it_start;
            float cursor_pos_x = 0.f;

            int word_idx = 0;
            float word_pos_x = 0.f;

            // Determining the line break, char by char
            for (int cursor_idx = line_it_start->start;cursor_idx <= end;cursor_idx++) {
                WrapCharPtr c = m_string[cursor_idx];

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
                            WrapCharPtr tmp_c = m_string[j];
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
        // ==== SECTION 3 ====
        // Calculation of char vertical positions
        float cursor_pos_y = line_it_start->line_pos_y;
        for (auto current_line_it = line_it_start;current_line_it != line_it_end;current_line_it++) {
            auto next_it = std::next(current_line_it);
            current_line_it->line_pos_y = cursor_pos_y;
            int line_end_idx = m_string.size();
            if (next_it != m_lines.end()) {
                line_end_idx = next_it->start;
            }

            // Find max char height relative to cursor pos
            // ascent is the distance from origin to bearing
            // descent is the distance from origin to bottom of char
            float max_ascent = 0.f;
            float max_descent = 0.f;
            for (int j = current_line_it->start;j < line_end_idx;j++) {
                WrapCharPtr c = m_string[j];
                max_ascent = max(max_ascent, c->ascent);
                max_descent = max(max_descent, c->descent);
            }

            for (int j = current_line_it->start;j < line_end_idx;j++) {
                WrapCharPtr c = m_string[j];
                c->_calculated_position.y = cursor_pos_y + max_ascent - c->ascent + c->offset.y;
            }
            current_line_it->height = max_ascent + max_descent;
            cursor_pos_y += current_line_it->height * (1.f + m_line_space);
        }

        // ==== SECTION 4 ====
        // Update all subsequent linesand chars in their respective y position by
        //     the y amount that may have been added in between startand end
        float y_diff = cursor_pos_y - line_it_start->line_pos_y - start_to_end_height - offset_subsequent_lines;
        if (y_diff != 0.f) {
            for (auto current_line_it = line_it_end;current_line_it != m_lines.end();current_line_it++) {
                current_line_it->line_pos_y += y_diff;
                auto next_it = std::next(current_line_it);
            }
            if (line_it_end != m_lines.end()) {
                for (int j = line_it_end->start;j < m_string.size();j++) {
                    m_string[j]->_calculated_position.y += y_diff;
                }
            }
        }
    }
    void WrapAlgorithm::setString(const std::vector<WrapCharPtr>& string) {
        clear();
        m_string = string;
        recalculate(0);
    }
    void WrapAlgorithm::insertAt(const std::vector<WrapCharPtr>& string, int position) {
        if (position == -1) {
            if (m_string.empty())
                position = 0;
            else
                position = m_string.size() - 1;
        }
        // There is no reason to make the program crash if a greater position
        // input has been given
        // TODO: make exception and bound checks
        if (position > m_string.size())
            position = m_string.size();

        int end = position;
        while (end < m_string.size()) {
            if (m_string[end]->is_linebreak)
                break;
            end++;
        }
        if (end >= m_string.size())
            end = m_string.size() - 1;
        end += string.size();

        m_string.insert(m_string.begin() + position, string.begin(), string.end());
        recalculate(position, end);
    }
    void WrapAlgorithm::insertAt(WrapCharPtr& c, int position) {
        insertAt(std::vector<WrapCharPtr>{c}, position);
    }
    void WrapAlgorithm::deleteAt(int delete_from, int delete_to) {
        // The idea for deletion is to find the paragraph(s) that contains
        // the deleted passage, and delete the desired passage in the string and
        // all the lines that the paragraph(s) contain (except the first one), and
        // make as if we insert a new paragraph(s)
        int end_of_paragraph = delete_to;
        while (end_of_paragraph < m_string.size()) {
            if (m_string[end_of_paragraph]->is_linebreak)
                break;
            end_of_paragraph++;
        }
        // Find start and end line of paragraph(s)
        auto line_it_start = m_lines.begin();
        while (line_it_start != m_lines.end()) {
            auto next_it = std::next(line_it_start);
            if (next_it != m_lines.end() && next_it->start > delete_from) {
                break;
            }
            if (next_it == m_lines.end())
                break;
            line_it_start++;
        }
        std::list<Line>::iterator line_it_end = line_it_start;
        while (line_it_end != m_lines.end()) {
            line_it_end++;
            if (line_it_end != m_lines.end() && line_it_end->start > end_of_paragraph) {
                break;
            }
        }
        float deleted_height = 0.f;
        if (line_it_end != m_lines.end())
            deleted_height = line_it_end->line_pos_y - line_it_start->line_pos_y - line_it_start->height;

        // We have found the (undeleted) paragraph(s) from line_it_start to line_it_end
        // Now we can erase the passage in the string and the corresponding lines (but keeping
        // the line_it_start)
        m_string.erase(m_string.begin() + delete_from, m_string.begin() + delete_to);
        if (line_it_start != line_it_end) {
            m_lines.erase(std::next(line_it_start), line_it_end);
        }
        end_of_paragraph -= delete_to - delete_from;
        if (end_of_paragraph >= m_string.size())
            end_of_paragraph = m_string.size() - 1;

        if (!m_string.empty())
            recalculate(delete_from, end_of_paragraph, deleted_height);
    }
    void WrapAlgorithm::clear() {
        m_string.clear();
        m_lines.clear();
        m_line_positions.clear();
        m_lines.push_back(Line{ 0, 0.f });
        m_line_positions.insert(0);
    }
    void WrapAlgorithm::setWidth(float width) {
        m_width = width;
        recalculate();
    }
    void WrapAlgorithm::setHeight(float height) {
        m_height = height;
    }
    void WrapAlgorithm::setLineSpace(float line_space) {
        m_line_space = line_space;
        recalculate();
    }
}