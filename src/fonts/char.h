#pragma once

#include <unordered_map>
#include <tempo.h>

namespace Fonts {
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
    struct Character {
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
        // static int count;
        // WrapCharacter() { count++; }
        // ~WrapCharacter() { count--; }
    };

    struct CharId {
        Tempo::FontID m_font_id;
        ImWchar m_char;
    };
    bool operator<(const CharId& c1, const CharId& c2) {
        return c1.m_font_id < c2.m_font_id && c1.m_char < c2.m_char;
    }

    void fillCharInfos(Character* char_ptr, ImWchar c, float font_size, Tempo::SafeImFontPtr font, bool force_breakable);
}