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
        AB::File* m_file;
        int m_block_idx_start;
        int m_block_idx_end;
        friend class WidgetManager;

        std::unordered_map<int, AbstractElement*> m_root_elements;
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
        UIState_ptr m_ui_state;
    public:
        WidgetManager(const AB::File& file, UIState_ptr ui_state);
        ~WidgetManager();

        WidgetId createWidget(const WidgetConfig& config);
        /**/
        void removeWidget(WidgetId id);
        /**
         * @brief Returns a reference to the widget associated
         * with the id
         *
         * @param id of the widget given by createWidget()
         * @return Widget&
         */
        Widget& getWidget(WidgetId id);
    };
}