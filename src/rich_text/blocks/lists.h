#pragma once

#include "../widgets.h"

namespace RichText {
    struct ULWidget: public AbstractBlock {
        unsigned int list_level = 0;
        char mark;
        ULWidget(UIState_ptr ui_state);
    };
    struct OLWidget: public AbstractBlock {
        unsigned int list_level = 0;
        char start;
        OLWidget(UIState_ptr ui_state);
    };
    struct LIWidget: public AbstractBlock {
    protected:
    public:
        unsigned int list_level = 0;
        bool is_task;
        char task_mark;
        LIWidget(UIState_ptr ui_state);
    };
}