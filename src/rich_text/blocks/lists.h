#pragma once

#include "block.h"

namespace RichText {
    struct ULWidget: public AbstractBlock {
        unsigned int list_level = 0;
        char mark;
        ULWidget();
    };
    struct OLWidget: public AbstractBlock {
        unsigned int list_level = 0;
        char start;
        OLWidget();
    };
    struct LIWidget: public AbstractBlock {
    protected:
    public:
        unsigned int list_level = 0;
        bool is_task;
        char task_mark;
        LIWidget();
    };
}