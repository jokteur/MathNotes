#pragma once

#include <string>
#include <unordered_map>
#include "ab/ab_file.h"
#include "ui/drawable.h"
#include "element.h"

namespace RichText {
    typedef int WidgetId;
    struct WidgetConfig {
        float line_start;
        int line_lookahead_window = 1000;
        bool interactive = false;
    };

    class WidgetManager;

    class Widget: public Drawable {
    private:
        WidgetConfig m_config;
        float m_current_line = 0.f;
        friend class WidgetManager;

        std::vector<AbstractElement*> m_elements;
    public:
        Widget(UIState_ptr ui_state): Drawable(ui_state) {}
        void draw();
    };

    /**
     * @brief WidgetManager manages all the widgets that could be displayed from
     * one AB-file.
     *
     */
    class WidgetManager {
    private:
        AB::File m_file;
        std::unordered_map<WidgetId, Widget> m_widgets;
        static WidgetId widget_id;
    public:
        WidgetManager(const AB::File& file);
        ~WidgetManager();

        WidgetId createWidget(const WidgetConfig& config, UIState_ptr ui_state);
        void removeWidget(WidgetId id);
        void displayWidget(WidgetId id);
    };
}