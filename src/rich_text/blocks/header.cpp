#include "header.h"
#include "rich_text/chars/im_char.h"
#include "ui/colors.h"

namespace RichText {
    HeaderWidget::HeaderWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        type = T_BLOCK_H;
    }
    bool HeaderWidget::buildAndAddChars(std::vector<WrapCharPtr>& string, int start) {
        auto char_ptr = std::make_shared<NewLine>();
        string.push_back(char_ptr);
        chars.push_back(char_ptr);

        using namespace Fonts;
        FontInfoOut font_out;
        FontRequestInfo request;
        request.font_styling = FontStyling{F_REGULAR, W_BOLD, S_NORMAL };
        request.size_wish = 32.f;

        m_ui_state->font_manager.requestFont(request, font_out);
        auto font = Tempo::GetImFont(font_out.font_id);
        for(int i = 0;i < hlevel;i++) {
            auto ptr = std::make_shared<ImChar>(font, (ImWchar)'#', font_out.size, Colors::black, false);
            string.push_back(char_ptr);
            chars.push_back(char_ptr);
        }
        return true;
    }
}