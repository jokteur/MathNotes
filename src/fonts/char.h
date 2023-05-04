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

    typedef Character* CharPtr;

    struct CharId {
        Tempo::FontID m_font_id;
        float m_font_size;
        unsigned int m_char;
        bool operator==(const CharId& other) const;

    };
    bool operator<(const CharId& c1, const CharId& c2);


    void fillCharInfos(Character* char_ptr, ImWchar c, float font_size, Tempo::SafeImFontPtr font, bool force_breakable);
}

/* Creating the hash function for CharId such that they can be inserted
 * into maps / unordered_map */
namespace std {
    template <>
    struct hash<Fonts::CharId> {
        size_t operator()(const Fonts::CharId& k) const {
            // Compute individual hash values for first, second
            // http://stackoverflow.com/a/1646913/126995
            size_t res = 17;
            res = res * 31 + hash<int>()(k.m_font_id);
            res = res * 31 + hash<unsigned int>()(k.m_char);
            res = res * 31 + hash<float>()(k.m_font_size);
            return res;
        }
    };
}