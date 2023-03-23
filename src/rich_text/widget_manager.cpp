#include "widget_manager.h"

using namespace AB;

namespace RichText {
    void Widget::draw() {

    }


    WidgetId WidgetManager::widget_id = 0;
    WidgetManager::WidgetManager(const File& file): m_file(file) {

    }

    WidgetId WidgetManager::createWidget(const WidgetConfig& config, UIState_ptr ui_state) {
        m_widgets[widget_id] = Widget(ui_state);
        m_widgets[widget_id].m_config = config;
        m_widgets[widget_id].m_current_line = config.line_start;
    }
    void WidgetManager::removeWidget(WidgetId id) {
        if (m_widgets.find(id) != m_widgets.end()) {
            m_widgets.erase(id);
        }
    }
    void WidgetManager::displayWidget(WidgetId id) {
        if (m_widgets.find(id) != m_widgets.end()) {
            m_widgets[id].draw();
        }
    }
    WidgetManager::~WidgetManager() {
        for (auto block : m_file.m_blocks) {
            if (block.widget_ptr != nullptr) {
                delete block.widget_ptr;
            }
        }
    }
}
