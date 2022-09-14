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
        bearing.y = m_latex_image->getAscent();
    }


    void LatexChar::draw(ImDrawList* draw_list) {
        auto& cursor_pos = ImGui::GetCursorScreenPos();
        draw_list->AddImage(
            m_latex_image->getImage()->texture(),
            cursor_pos + _calculated_position,
            cursor_pos + _calculated_position + dimensions
        );
    }

    CharPtr ToLatexChar(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding) {
        return std::make_shared<LatexChar>(std::make_shared<Latex::LatexImage>(latex_src, font_size, line_space, text_color, scale, inner_padding));
    }
}