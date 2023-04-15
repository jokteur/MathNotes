#pragma once

#include <string>

// #include <md4c.h>
// #include <md4c-html.h>


#include <ab_parser.h>
#include "element.h"
#include "ui/fonts.h"
#include "markdown_config.h"
#include "rich_text_context.h"
#include "ab/ab_file.h"

namespace RichText {
    // Inspired from https://github.com/mekhontsev/imgui_md
    struct RootBlock {
        int line_start;
        int line_end;
        Type type;
    };

    class ABToWidgets {
    private:
        AB::Parser m_parser;
        MarkdownConfig m_config;

        SafeString m_safe_text;
        RichTextInfo* m_rt_info;
        int* m_line_offset = 0;
        int m_root_idx_start;
        int m_root_idx_current;
        int m_root_idx_end;
        int m_text_size;
        int m_level = -1;

        AbstractElementPtr m_current_ptr = nullptr;
        AbstractElementPtr m_root_ptr = nullptr;
        AbstractElementPtr m_last_text_ptr = nullptr;
        AbstractElementPtr m_last_block_ptr = nullptr;
        std::map<int, AbstractElementPtr>* m_root_elements;

        UIState_ptr m_ui_state = nullptr;
        AB::File* m_ab_file;

        std::string m_href;

        int text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds);
        int block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::BlockDetailPtr detail = nullptr);
        int span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::SpanDetailPtr detail = nullptr);

        void push_to_tree(AbstractElementPtr& node);
        void set_href(bool enter, const std::string& src);
        void tree_up();
        void set_infos(MarkdownConfig::type type, AbstractElementPtr ptr, bool special_style = false);

        AbstractElementPtr BLOCK_QUOTE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr BLOCK_UL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockUlDetail&);
        AbstractElementPtr BLOCK_OL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockOlDetail&);
        AbstractElementPtr BLOCK_LI(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockLiDetail&);
        AbstractElementPtr BLOCK_HR(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr BLOCK_H(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockHDetail& d);
        AbstractElementPtr BLOCK_CODE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockCodeDetail&);
        AbstractElementPtr BLOCK_P(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr BLOCK_HIDDENSPACE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);

        AbstractElementPtr SPAN_EM(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr SPAN_STRONG(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr SPAN_A(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::SpanADetail&);
        AbstractElementPtr SPAN_IMG(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::SpanImgDetail&);
        AbstractElementPtr SPAN_CODE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr SPAN_DEL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr SPAN_HIGHLIGHT(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractElementPtr SPAN_LATEXMATH(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        // AbstractElementPtr SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool, int mark_begin, int mark_end);
        // AbstractElementPtr SPAN_U(bool, int mark_begin, int mark_end);

        void configure_parser();
    public:
        ABToWidgets();

        void parse(AB::File* file, int root_idx_start, int root_idx_end, std::map<int, AbstractElementPtr>* root_elements, UIState_ptr ui_state, MarkdownConfig config = MarkdownConfig());
    };
}