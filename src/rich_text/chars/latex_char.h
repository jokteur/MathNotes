#pragma once

#include "drawable_char.h"
#include "latex/latex.h"

#include <unordered_map>

namespace RichText {
    /**
     * @brief Character for latex equations
     *
     */
    struct LatexChar : public DrawableChar {
        Latex::LatexImageUPtr m_latex_image;
        bool m_is_defined = false;
    public:
        LatexChar(Latex::LatexImageUPtr latex_image);
        ~LatexChar();

        LatexChar(const LatexChar&) = delete;
        LatexChar& operator=(const LatexChar&) = delete;

        bool draw(Draw::DrawList& draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
    };

    typedef std::shared_ptr<LatexChar> LatexCharPtr;

    LatexCharPtr ToLatexChar(const std::string& latex_src, float font_size = 20.f, float line_space = 7.f, microtex::color text_color = microtex::BLACK, ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f), bool inlined = false);
}