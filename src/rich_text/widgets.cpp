#include "widgets.h"

namespace RichText {
    // AbstractWidget
    bool AbstractWidget::buildAndAddChars(std::vector<WrapCharPtr>&) { 
       return true;
    }
    void AbstractWidget::draw(ImDrawList* draw_list, ImVec2& draw_offset) {
        for (auto ptr : m_childrens) {
            ptr->draw(draw_list, draw_offset);
            draw_offset.y += ptr->m_dimensions.y;
        }
        buildWidget();
    }
    void AbstractWidget::setWidth(float width) {
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
        m_window_width = width;
    }

    // AbstractBlock
    void AbstractBlock::setWidth(float width) {
        for (auto ptr : m_childrens) {
            ptr->setWidth(width);
        }
        m_wrapper.setWidth(width);
        m_window_width = width;
        m_dimensions.x = width - m_x_offset;
        m_dimensions.y = m_wrapper.getHeight();
    }
}