#pragma once

#include <string>
#include <unordered_map>
#include "ab/ab_file.h"
#include "markdown.h"
#include "ui/drawable.h"
#include "element.h"


namespace RichText {
    typedef int WidgetId;
    struct WidgetConfig {
        float line_start;
        int line_lookahead_window = 40;
        bool interactive = false;
    };

    class WidgetManager;

    class Widget: public Drawable {
    private:
        bool m_redo_positions = false;

        WidgetConfig m_config;
        AB::File* m_file;
        int m_block_idx_start = 1e9;
        int m_block_idx_end = -1;
        friend class WidgetManager;

        Draw::DrawList m_draw_list;
        float m_current_line = 0.f;
        float m_current_width = 0.f;
        float m_y_scroll = 0.f;

        std::unordered_map<AB::RootBlockPtr, AbstractElementPtr> m_root_elements;

        void manage_elements();
    public:
        Widget(UIState_ptr ui_state): Drawable(ui_state) {}
        Widget(const Widget&) = delete;

        ~Widget() { std::cout << "Destroy widget" << std::endl; }
        Widget& operator= (const Widget&) = delete;
        void draw();
    };
    typedef std::shared_ptr<Widget> WidgetPtr;

    /**
     * @brief WidgetManager manages all the widgets that could be displayed from
     * one AB-file.
     *
     */
    class WidgetManager {
    private:
        AB::File m_file;
        std::unordered_map<WidgetId, WidgetPtr> m_widgets;
        Widget m_empty_widget;
        ABToWidgets m_ab_to_widgets;
        int m_current_widgets = 0;
        UIState_ptr m_ui_state;
    public:
        WidgetManager(const AB::File& file, UIState_ptr ui_state);
        ~WidgetManager();

        /* Returns a widget id linked to a widget (limited to 32 widgets)*/
        WidgetId createWidget(const WidgetConfig& config);
        /**/
        void removeWidget(WidgetId id);
        /**
         * @brief Returns a pointer to the widget associated with the id
         *
         *
         * @param id of the widget given by createWidget()
         * @return WidgetPtr
         */
        WidgetPtr getWidget(WidgetId id);

        /**
         * Call this function periodically to manage the memory of
         * all the widgets ; create / destroy blocks, spans, ...
        */
        void manage();
    };
}