#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <ab_parser.h>

namespace RichText {
    class WidgetManager;
}
namespace AB {
    struct RootBlock {
        int line_start;
        int idx_start;
        int line_end;
        int idx_end;
        BLOCK_TYPE type;
        int is_dirty = 0;
    };
    typedef std::list<RootBlock>::iterator RootIterator;

    struct Header {
        std::vector<AB::Boundaries> bounds;
        std::string content;
    };
    struct Equation {
        std::vector<AB::Boundaries> bounds;
        std::string content;
    };
    struct Label {};

    enum TrackingPolicy { NONE, LABELED, ALL };

    struct FileConfig {
        TrackingPolicy track_headers = ALL;
        TrackingPolicy track_equations = LABELED;
        bool track_labels = true;
    };

    struct Bound {
        int txt_idx = -1;
        int line_idx = -1;
        int block_idx = -1;
    };

    struct BlockBounds {
        RootIterator start;
        RootIterator end;
    };


    struct File {
    private:
        void add_to_text(std::string& str, const std::vector<AB::Boundaries>& bounds);
        void parse(int start = 0, int end = -1);
    public:
        std::list<RootBlock> m_blocks;
        std::string m_txt;
        std::vector<int> m_line_begins;
        std::vector<Equation> m_equations;
        std::vector<Header> m_headers;
        std::vector<Label> m_labels;
        FileConfig m_config;
        int m_level = -1;

        bool m_h_accumulating = false;
        std::string m_h_accumulator;

        File(const std::string& path_or_txt, bool is_path = true, FileConfig config = FileConfig{});

        void insertText(int position, const std::string& text);
        void deleteText(int start, int end);

        /**
         * @brief Get the iterators of the (root) blocks that contain
         * line_start to line_end
         *
         * @param line_start
         * @param line_end
         * @return Boundaries
         */
        BlockBounds getBlocksBoundsContaining(int line_start, int line_end);
    };
};