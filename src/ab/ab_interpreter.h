#pragma once
#include <string>
#include <functional>

#include "definitions.h"
#include "helpers.h"


// Implementation is inspired from http://github.com/mity/md4c
namespace AB {
    struct BlockDetail {
    };

    struct BlockCodeDetail : public BlockDetail {
        std::string lang;
        int num_ticks;
    };

    struct BlockOlDetail : public BlockDetail {
        enum OL_TYPE { OL_NUMERIC, OL_ALPHABETIC, OL_ROMAN };
        char pre_marker;
        char post_marker;
        bool lower_case;
        OL_TYPE type;
    };

    struct BlockUlDetail : public BlockDetail {
        char marker;
    };

    struct BlockLiDetail : public BlockDetail {
        enum TASK_STATE { TASK_EMPTY, TASK_FAIL, TASK_SUCCESS };
        bool is_task;
        std::string number;
        TASK_STATE task_state;
    };


    struct BlockHDetail : public BlockDetail {
        unsigned char level; /* Header level (1 to 6) */
    };
    // TODO, tables

    struct SpanDetail {
        /* A span can have pre and post information (before text begins)
         * E.g. [foo](my title) is an URL, where pre->start contains [
         * and end->post contains ](my title)
         */
        OFFSET pre;
        OFFSET start;
        OFFSET end;
        OFFSET post;
    };
    struct SpanADetail : public SpanDetail {
        std::string href;
        std::string title;
    };
    struct SpanImgDetail : public SpanDetail {
        std::string src;
        std::string title;
    };
    struct SpanWikiLink : public SpanDetail {
        std::string target;
    };

    typedef std::function<bool(BLOCK_TYPE type, BlockDetail* detail)> BlockFct;
    typedef std::function<bool(SPAN_TYPE type, SpanDetail* detail)> SpanFct;
    typedef std::function<bool(TEXT_TYPE type, const OFFSET begin, const OFFSET end)> TextFct;

    struct Parser {
        BlockFct enter_block;
        BlockFct leave_block;
        SpanFct enter_span;
        SpanFct leave_span;
        TextFct text;
    };

    bool parse(const CHAR* text, SIZE size, const Parser* parser);
};