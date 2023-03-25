#include "widget_manager.h"

using namespace AB;

namespace RichText {
    void Widget::draw() {

    }


    WidgetId WidgetManager::widget_id = 0;
    WidgetManager::WidgetManager(const File& file, UIState_ptr ui_state): m_file(file) {
        m_ui_state = ui_state;
    }

    WidgetId WidgetManager::createWidget(const WidgetConfig& config) {
        m_widgets[widget_id] = Widget(m_ui_state);
        m_widgets[widget_id].m_config = config;
        m_widgets[widget_id].m_current_line = config.line_start;
    }
    void WidgetManager::removeWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
        }
    }
    Widget& WidgetManager::getWidget(WidgetId id) {
        if (m_widgets.find(id) != m_widgets.end()) {
            return m_widgets[id];
        }
        return Widget(m_ui_state);
    }
    WidgetManager::~WidgetManager() {
    }
}
