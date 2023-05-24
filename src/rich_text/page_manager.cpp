#include "page_manager.h"

#include <cmath>

#include "rich_text/chars/im_char.h"
#include "profiling.h" 

using namespace AB;

namespace RichText {
    PageManager::PageManager(const File& file) : m_file(file) {
    }

    WidgetId PageManager::createPage(const PageConfig& config) {
        WidgetId widget_id = 1;

        bool available_slot = false;
        for (int i = 1;i <= 32;i++) {
            if (m_current_widgets ^ widget_id) {
                available_slot = true;
                break;
            }
            widget_id = widget_id << 1;
        }

        if (!available_slot)
            return 0;

        m_current_widgets |= widget_id;

        auto widget = std::make_shared<Page>(&m_file);
        widget->setName(std::to_string(widget_id));
        widget->m_config = config;
        // widget->m_file = &m_file;
        widget->m_current_line = config.line_start;
        m_widgets[widget_id] = widget;
        return widget_id;
    }
    void PageManager::removeWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
            m_current_widgets ^= id;
        }
    }
    WidgetPtr PageManager::getWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            return it->second;
        }
        return nullptr;
    }
    PageManager::~PageManager() {
    }

    void PageManager::manage() {
        int widget_id = 1;
        for (int i = 1;i <= 32;i++) {
            if (m_current_widgets & widget_id) {
                // std::cout << "Managing " << widget_id << " ";
                // auto& widget = getWidget(widget_id);
                // int current_line = widget.m_current_line;
            }
            widget_id = widget_id << 1;
        }
    }
}
