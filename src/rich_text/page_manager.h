#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>

#include "ab/ab_file.h"
#include "page.h"

namespace RichText {
    /**
     * @brief WidgetManager manages all the widgets that could be displayed from
     * one AB-file.
     *
     */
    class PageManager {
    private:
        AB::File m_file;
        std::unordered_map<WidgetId, WidgetPtr> m_widgets;
        Page m_empty_widget;
        int m_current_widgets = 0;
    public:
        PageManager(const AB::File& file);
        ~PageManager();

        /* Returns a widget id linked to a widget (limited to 32 widgets)*/
        WidgetId createPage(const PageConfig& config);
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