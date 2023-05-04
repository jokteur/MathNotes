#include "latex_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    LatexChar::LatexChar(Latex::LatexImageUPtr latex_image) : DrawableChar(nullptr) {
        m_latex_image = std::move(latex_image);
        if (!m_latex_image->getLatexErrorMsg().empty())
            return;

        m_is_defined = true;

        info = new Fonts::Character;

        info->dimensions = m_latex_image->getDimensions();
        info->advance = info->dimensions.x;
        info->ascent = m_latex_image->getAscent();
        info->descent = m_latex_image->getDescent();
    }

    LatexChar::~LatexChar() {
        delete info;
    }

    bool LatexChar::draw(Draw::DrawList& draw_list, const Rect& boundaries, ImVec2 draw_offset) {
        auto cursor_pos = ImGui::GetCursorScreenPos();
        auto final_pos = cursor_pos + calculated_position + draw_offset;
        final_pos.x = IM_ROUND(final_pos.x);
        final_pos.y = IM_ROUND(final_pos.y);
        draw_list->AddImage(
            m_latex_image->getImage()->texture(),
            final_pos,
            final_pos + info->dimensions
        );
        return true;
    }

    LatexCharPtr ToLatexChar(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding, bool inlined) {
        if (inlined)
            return std::make_shared<LatexChar>(std::make_unique<Latex::LatexImage>("$$" + latex_src + "$$", font_size, line_space, text_color, scale, inner_padding));
        else
            return std::make_shared<LatexChar>(std::make_unique<Latex::LatexImage>(latex_src, font_size, line_space, text_color, scale, inner_padding));
    }
}