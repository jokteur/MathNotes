#include "text.h"
#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    TextString::TextString(UIState_ptr ui_state) : AbstractSpan(ui_state) {
        type = T_TEXT;
    }

    bool TextString::buildAndAddChars(std::vector<WrapCharPtr>& string, int start) {
        chars.clear();

        using namespace Fonts;
        FontInfoOut font_out;
        m_ui_state->font_manager.requestFont(font_styling, font_out);
        auto font = Tempo::GetImFont(font_out.font_id);

        if (safe_string == nullptr || font.get() == nullptr) {
            return false;
        }
        
        float font_size = font_out.size * font_out.ratio * scale * Tempo::GetScaling();
        for (int i = text_begin;i < text_end;i++) {
            unsigned int c = (unsigned int)(*safe_string)[i];
            if (c >= 0x80) {
                ImTextCharFromUtf8(&c, &(*safe_string)[i], &(*safe_string)[safe_string->size() - 1]);
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            bool force_breakable = false;
            if (c == ',' || c == '|' || c == '-' || c == '.' || c == '!' || c == '?')
                force_breakable = true;
            auto char_ptr = std::make_shared<ImChar>(font, (ImWchar)c, font_size, color, force_breakable);
            chars.push_back(std::static_pointer_cast<DrawableChar>(char_ptr));
            string.push_back(std::static_pointer_cast<WrapCharacter>(char_ptr));
        }
        return true;
    }
    void TextString::draw(ImDrawList* draw_list) {
        for (auto& c: chars) {
            c->draw(draw_list);
        }
    }
}