#pragma once

#include "drawable_char.h"
#include "../../latex/latex.h"

namespace RichText {
    /**
     * @brief Character for latex equations
     *
     */
    struct LatexChar : public DrawableChar {
        Latex::LatexImagePtr m_latex_image;
    public:
        LatexChar(Latex::LatexImagePtr latex_image);

        void draw(ImDrawList* draw_list) override;
    };

    DrawableCharPtr ToLatexChar(const std::string& latex_src, float font_size = 20.f, float line_space = 7.f, microtex::color text_color = microtex::BLACK, ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f));
}