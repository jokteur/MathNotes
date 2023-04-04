#pragma once

#include <tempo.h>
#include <unordered_set>
#include "ab_parser.h"

#include "ab/ab_file.h"
#include "types.h"
#include "ui/drawable.h"
#include "ui/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"
#include "types.h"

#include "widgets_enum.h"
#include "rich_text_context.h"
#include "markdown_config.h"
#include "wrapper.h"
#include "chars/drawable_char.h"

namespace RichText {
    struct AbstractElement;
    using AbstractElementPtr = std::shared_ptr<AbstractElement>;
    using AbstractElementWeakPtr = std::weak_ptr<AbstractElement>;

    struct AbstractElement: public Drawable {
    protected:
        std::vector<DrawableCharPtr> m_draw_chars;
        std::vector<DrawableCharPtr> m_draw_delimiter_chars;
        std::vector<WrapCharPtr> m_wrap_chars;
        int m_id = 0;

    public:
        Type m_type;
        Category m_category;
        static int count;
        AbstractElement(UIState_ptr ui_state): Drawable(ui_state) { count++; m_id = count; }
        ~AbstractElement();

        // Informations about the tree structure
        std::vector<AbstractElementPtr> m_childrens;
        AbstractElementWeakPtr m_parent;
        AB::RootBlockWeakPtr m_ref_to_root;

        bool m_widget_dirty = true;
        int m_display_status = 0;

        // Returns false if not succesfully build chars
        bool virtual add_chars(std::vector<WrapCharPtr>& wrap_chars);
        bool virtual draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries);

        bool is_in_boundaries(const Rect& boundaries);

        // Draw hooks
        float virtual hk_set_position(float& cursor_y_pos, float& x_offset);
        void virtual hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset);
        bool virtual hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries);
        void virtual hk_draw_background(Draw::DrawList& draw_list);
        void virtual hk_draw_show_boundaries(Draw::DrawList& draw_list, float cursor_y_pos, const Rect& boundaries);
        /* Debug prints object info in a special window created by parent */
        void virtual hk_debug(const std::string& prefix = "");
        void virtual hk_debug_attributes();

        Style m_style;
        Style m_special_chars_style;

        bool m_is_selected = true;
        WrapAlgorithm m_wrapper;
        std::unordered_set<int> m_lines;

        // Position of the pointer in m_childrens;
        int m_child_number = -1;

        std::vector<AB::Boundaries> m_text_boundaries;
        AB::Attributes m_attributes;

        // Widget position and size
        ImVec2 m_position;
        ImVec2 m_dimensions;
        bool m_is_dimension_set = false;
        float m_scale = 1.f;
        float m_window_width = 1.f;

        // Debug
        bool m_show_boundaries = false;

        // Internal
        SafeString m_safe_string;
        RichTextInfo* m_rt_info;

        // Events
        void virtual onClick() {}
        void virtual onSelect() {}
        void virtual onDeselect() {}
        void virtual setWidth(float width);
    };

    struct RootNode: public AbstractElement {
        RootNode(UIState_ptr ui_state): AbstractElement(ui_state) {
            m_type = T_ROOT;
            m_category = C_ROOT;
        }
    };
}