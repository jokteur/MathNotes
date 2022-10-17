#include "latex_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    LatexChar::LatexChar(Latex::LatexImagePtr latex_image) {
        m_latex_image = latex_image;
        if (!latex_image->getLatexErrorMsg().empty())
            return;

        dimensions = m_latex_image->getDimensions();
        advance = dimensions.x;
        ascent = m_latex_image->getAscent();
        descent = m_latex_image->getDescent();
    }


    void LatexChar::draw(Draw::DrawList& draw_list, ImVec2 draw_offset) {
        auto cursor_pos = ImGui::GetCursorScreenPos();
        auto final_pos = cursor_pos + _calculated_position + draw_offset;
        final_pos.x = IM_ROUND(final_pos.x);
        final_pos.y = IM_ROUND(final_pos.y);
        draw_list->AddImage(
            m_latex_image->getImage()->texture(),
            final_pos,
            final_pos + dimensions
        );
    }

    DrawableCharPtr ToLatexChar(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding) {
        return std::make_unique<LatexChar>(std::make_shared<Latex::LatexImage>(latex_src, font_size, line_space, text_color, scale, inner_padding));
    }
}