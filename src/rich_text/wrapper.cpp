#include "wrapper.h"
#include <iomanip>

namespace RichText {
    template<typename T>
    inline T max(T a, T b) {
        if (a < b)
            return b;
        return a;
    }
    template<typename T>
    inline T min(T a, T b) {
        if (a < b)
            return a;
        return b;
    }

    TextWrapper::TextWrapper(float width, float line_space) {
        m_width = width;
        m_line_space = line_space;

        m_lines.push_back(Line{ 0, 0.f });
        m_line_positions.insert(0);
    }
    TextWrapper::~TextWrapper() {

    }

    inline int TextWrapper::find_line_idx(int cursor_pos) {
        // m_line_positions.find(cursor_pos);
        return 0;
    }
    inline void TextWrapper::push_char_on_line(CharPtr c, float* cursor_x_coord) {
        c->_calculated_position.x = *cursor_x_coord + c->offset.x;
        *cursor_x_coord += c->advance;
    }
    inline Line* TextWrapper::push_new_line(int cursor_pos, float* cursor_x_coord) {
        m_lines.push_back(Line{ cursor_pos, 0.f });
        *cursor_x_coord = 0.f;
        m_line_positions.insert(cursor_pos);
        return &(m_lines.back());
    }
    void TextWrapper::recalculate(int from) {
        int line_idx = find_line_idx(from);

        // Delete all lines after the current one
        if (m_lines.size() > 1 && line_idx + 1 < m_lines.size()) {
            for (int i = line_idx + 1;i < m_lines.size();i++) {
                m_line_positions.erase(m_lines[i].start);
            }
            m_lines.erase(m_lines.begin() + line_idx + 1, m_lines.end());
        }

        // The calculation of the chars coordinates must be done in two passes:
        // First horizontal position to determine the line breaks
        // Then vertical position, which depends on the calculated line breaks and chars properties

        // Calculate line breaks
        {
            float cursor_x_coord = 0.f;
            Line* current_line = &m_lines[line_idx];

            int word_pos = 0;
            int word_with_whitespace_pos = 0;
            float word_x_coord = 0.f;
            float word_with_whitespace_x_coord = 0.f;

            // First, find the last breakable character from current position
            // TODO FIX ME for from > 0 
            // for (int i = from;i > 0;i--) {
            //     auto& c = m_text[i];
            //     if (c->breakable) {
            //         // Fetch next possible character position
            //         word_pos = i + 1;
            //         word_x_coord = c->_calculated_position.x - c->bearing.x + c->advance;
            //         break;
            //     }
            // }
            // if (word_pos < current_line->start) {
            //     word_pos = current_line->start;
            //     word_x_coord = 0.f;
            // }

            bool is_last_char_whitespace = false;

            // Determining the line break, char by char
            for (int cursor_pos = from;cursor_pos < m_text.size();cursor_pos++) {
                CharPtr c = m_text[cursor_pos];

                // If a breakable char follows a white space (break line or breakable)
                // then it should not considered as a breakable char
                if (c->breakable) {
                    word_pos = cursor_pos + 1;
                    word_x_coord = cursor_x_coord + c->advance;
                    if (!is_last_char_whitespace) {
                        word_with_whitespace_pos = word_pos;
                        word_with_whitespace_x_coord = word_x_coord;
                    }
                    is_last_char_whitespace = true;
                }
                else {
                    is_last_char_whitespace = false;
                }

                if (c->is_linebreak) {
                    current_line = push_new_line(cursor_pos + 1, &cursor_x_coord);
                    word_pos = cursor_pos + 1;
                    word_x_coord = 0.f;
                    is_last_char_whitespace = true;
                    continue;
                }

                float char_width = c->dimensions.x + c->offset.x;

                if (char_width + cursor_x_coord > m_width) {
                    // Current word width: a word width is counted from the last breakable character
                    // or the last line break (position 0), which ever came last
                    float word_width = cursor_x_coord - word_x_coord;

                    current_line = push_new_line(cursor_pos, &cursor_x_coord);

                    // In this case, only the char is pushed to the next
                    // line because char + word can't fit on the whole width
                    if (char_width + word_width > m_width) {
                        push_char_on_line(c, &cursor_x_coord);
                    }
                    else {
                        // Push every single character onto the next line (with the current one)
                        float tmp_cursor_pos = 0.f;
                        for (int j = word_pos;j <= cursor_pos;j++) {
                            CharPtr tmp_c = m_text[j];
                            push_char_on_line(tmp_c, &tmp_cursor_pos);
                        }
                        current_line->start = word_pos;
                        cursor_x_coord = tmp_cursor_pos;
                    }
                    // Have to update word_xxx after manipulations with current word have been made
                    word_pos = current_line->start;
                    word_x_coord = 0.f;
                }
                else {
                    push_char_on_line(c, &cursor_x_coord);
                }
            }
        }
        // Calculate vertical chars positions
        {
            float cursor_y_coord = 0.f;

            for (int line_idx = 0;line_idx < m_lines.size();line_idx++) {
                Line* line = &m_lines[line_idx];
                int line_end_pos = m_text.size();
                if (line_idx < m_lines.size() - 1) {
                    line_end_pos = m_lines[line_idx + 1].start;
                }

                // Find max char height relative to cursor pos
                // ascent is the distance from origin to bearing
                // descent is the distance from origin to bottom of char
                float max_ascent = 0.f;
                float max_descent = 0.f;
                for (int j = line->start;j < line_end_pos;j++) {
                    CharPtr c = m_text[j];
                    max_ascent = max<float>(max_ascent, c->ascent);
                    max_descent = max<float>(max_descent, c->descent);
                }

                for (int j = line->start;j < line_end_pos;j++) {
                    CharPtr c = m_text[j];
                    c->_calculated_position.y = cursor_y_coord + max_ascent - c->ascent + c->offset.y;
                }
                line->height = max_ascent + max_descent;
                cursor_y_coord += line->height * (1.f + m_line_space);
            }
        }
    }

    void TextWrapper::insertAt(const std::vector<CharPtr> string, int position) {
        // for (auto c : string) {
        //     m_text.push_back(c);
        // }
        m_text = string;
        // recalculate();
    }
    void TextWrapper::setWidth(float width) {
        m_width = width;
        recalculate();
    }
}