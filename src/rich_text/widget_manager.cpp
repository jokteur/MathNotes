#include "widget_manager.h"

using namespace AB;

namespace RichText {
    void Widget::draw() {

    }


    WidgetId WidgetManager::widget_id = 0;
    WidgetManager::WidgetManager(const File& file): m_file(file) {

    }

    WidgetId WidgetManager::createWidget(const WidgetConfig& config, UIState_ptr ui_state) {
        widget_id++;
        Widget widget(ui_state);
        widget.m_config = config;
        widget.m_file = &m_file;
        widget.m_current_line = config.line_start;
        auto pair = m_widgets.emplace(std::pair<WidgetId, Widget>(widget_id, widget));
        return widget_id;
    }
    void WidgetManager::removeWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
        }
    }
    void WidgetManager::displayWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            it->second.draw();
        }
    }
    WidgetManager::~WidgetManager() {
        for (auto block : m_file.m_blocks) {
            if (block.widget_ptr != nullptr) {
                delete (AbstractElement*)block.widget_ptr;
            }
        }
    }
}
