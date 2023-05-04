#include "latex_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    LatexChar::LatexChar(Latex::LatexImagePtr latex_image) : DrawableChar(nullptr) {
        m_latex_image = std::move(latex_image);
        if (!m_latex_image->getLatexErrorMsg().empty())
            return;

        // info->dimensions = m_latex_image->getDimensions();
        // info->advance = dimensions.x;
        // info->ascent = m_latex_image->getAscent();
        // info->descent = m_latex_image->getDescent();
    }

    // void* LatexChar::operator new(size_t size) {
    //     void* p = ::operator new(size);

    //     auto ptr = static_cast<LatexChar*>(p);

    //     ptr->m_is_defined = true;

    //     ptr->info = new Fonts::Character;
    //     ptr->info->dimensions = ptr->m_latex_image->getDimensions();
    //     ptr->info->advance = ptr->info->dimensions.x;
    //     ptr->info->ascent = ptr->m_latex_image->getAscent();
    //     ptr->info->descent = ptr->m_latex_image->getDescent();

    //     ptr->info->ascent = ptr->m_latex_image->getAscent();
    //     ptr->info->descent = ptr->m_latex_image->getDescent();

    //     return p;
    // }
    // void LatexChar::operator delete(void* p) {
    //     auto ptr = static_cast<LatexChar*>(p);
    //     delete ptr->info;
    //     free(p);
    // }

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

    LatexChar ToLatexChar(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding, bool inlined) {
        if (inlined)
            return LatexChar(std::make_shared<Latex::LatexImage>("$$" + latex_src + "$$", font_size, line_space, text_color, scale, inner_padding));
        else
            return LatexChar(std::make_shared<Latex::LatexImage>(latex_src, font_size, line_space, text_color, scale, inner_padding));
    }
}