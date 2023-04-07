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
        bool interactive = false;
        float min_scroll_height = 10.f;
    };

    class WidgetManager;

    class Widget: public Drawable {
    private:
        bool m_redo_positions = false;
        bool m_recalculate_line_height = true;

        WidgetConfig m_config;
        AB::File* m_file;
        int m_block_idx_start = 1e9;
        int m_block_idx_end = -1;
        friend class WidgetManager;

        Draw::DrawList m_draw_list;
        float m_current_line = 0.f;
        float m_current_width = 0.f;
        float m_y_scroll = 0.f;
        float m_scale = 1.f;

        AbstractElementPtr m_top_displayed_ptr = nullptr;

        int m_line_lookahead_window = 2000;

        float m_display_height = 0.f;
        float m_approximate_min_height = 0.f;
        float m_line_height = 10.f;

        std::map<int, AbstractElementPtr> m_root_elements;

        void calculate_heights();
        void manage_elements();
        void debug_window();
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