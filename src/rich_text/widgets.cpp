#include "widgets.h"

namespace RichText {
    RootNode::RootNode(UIState_ptr ui_state) : AbstractWidget(ui_state) {
        m_type = T_ROOT;
        m_category = C_ROOT;
    }

    void RootNode::draw(ImDrawList* draw_list) {
        for (auto ptr : m_childrens) {
            if (ptr->m_type != T_TEXT) {
                ptr->draw(draw_list);
            }
        }
    }
}