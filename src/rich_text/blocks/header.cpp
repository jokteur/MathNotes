#include "header.h"
#include "rich_text/chars/im_char.h"
#include "ui/colors.h"

namespace RichText {
    HeaderWidget::HeaderWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_H;
    }
    bool HeaderWidget::buildAndAddChars(std::vector<DrawableCharPtr>& draw_string, std::vector<WrapCharPtr>& wrap_string, int start) {
        // auto char_ptr = std::make_shared<NewLine>();
        // // string.push_back(char_ptr);
        // m_draw_string.push_back(char_ptr);
        // m_wrap_chars.push_back(char_ptr);

        // using namespace Fonts;
        // FontInfoOut font_out;
        // m_ui_state->font_manager.requestFont(m_font_request, font_out);

        // auto font = Tempo::GetImFont(font_out.font_id);
        // if (font == nullptr) {
        //     return false;
        // }
        // for (int i = 0;i < hlevel;i++) {
        //     auto ptr = std::make_shared<ImChar>(font, (ImWchar)'#', font_out.size, m_font_color, false);
        //     m_draw_string.push_back(ptr);
        //     m_wrap_chars.push_back(ptr);
        // }
        return true;
    }

    void HeaderWidget::draw(ImDrawList* draw_list) {
        for (auto ptr : m_childrens) {
            ptr->draw(draw_list);
        }
        buildWidget();
    }
    void HeaderWidget::buildWidget() {
        if (m_widget_dirty) {
            m_draw_chars.clear();
            m_wrap_chars.clear();
            bool success = true;
            for (auto ptr : m_childrens) {
                if (ptr->m_category != C_SPAN) {
                    break;
                }
                auto res = ptr->buildAndAddChars(m_draw_chars, m_wrap_chars);
                if (!res) {
                    success = false;
                    break;
                }
            }
            if (success) {
                m_wrapper.clear();
                m_wrapper.setWidth(m_window_width - m_x_offset);
                m_wrapper.setString(m_wrap_chars);
                m_widget_dirty = false;
            }
        }
    }
}