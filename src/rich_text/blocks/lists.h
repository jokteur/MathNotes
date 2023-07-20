#pragma once

#include "block.h"

namespace RichText {
    struct ULWidget : public AbstractBlock {
        unsigned int list_level = 0;
        char mark;
        ULWidget();
    };
    struct OLWidget : public AbstractBlock {
        unsigned int list_level = 0;
        char start;
        OLWidget();
    };
    struct LIWidget : public AbstractBlock {
    protected:
    public:
        unsigned int list_level = 0;
        bool is_task;
        char task_mark;
        std::string number;
        std::string marker;
        LIWidget();

        bool hk_build_pre_delimiter_chars(DrawContext* ctx) override;
        bool hk_draw_secondary(DrawContext* context) override;
    };
}