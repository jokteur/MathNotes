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
#include "ab_config.h"
#include "wrapper.h"
#include "chars/drawable_char.h"

namespace RichText {
    struct AbstractElement;
    // using AbstractElementPtr = std::shared_ptr<AbstractElement>;
    /**
     * Unfortunately, shared_ptr have a big performance hit in tree structures
     *
     * Here is how memory is managed:
     * - AbstractElements are created in ab_converted with the "new" keyword
     * - each AbstractElement (and derived) is responsible for the memory
     *   of its children
     *    - thus, even though the memory is created in ab_convert.cpp, it is then handled
     *      in AbstractElement destructor
     * - memory of root AbstractElement is handled by RootNode
     *
     */
    using AbstractElementPtr = AbstractElement*;
    // using AbstractElementWeakPtr = std::weak_ptr<AbstractElement>;
    // using AbstractElementWeakPtr = AbstractElement*;

    struct AbstractElement: public Drawable {
    protected:
        std::vector<DrawableCharPtr> m_draw_chars;
        std::vector<DrawableCharPtr> m_draw_delimiter_chars;
        std::vector<WrapCharPtr> m_wrap_chars;
    public:
        static int count;
        const unsigned int DIRTY_WIDTH = 0x1;
        const unsigned int DIRTY_CHARS = 0x2;
        const unsigned int ALL_DIRTY = DIRTY_WIDTH | DIRTY_CHARS;

        Type m_type;
        Category m_category;
        AbstractElement(UIState_ptr ui_state): Drawable(ui_state) { count++; }
        ~AbstractElement();

        // Informations about the tree structure
        std::vector<AbstractElementPtr> m_childrens;
        AbstractElementPtr m_parent;
        AB::RootBlockWeakPtr m_ref_to_root;

        unsigned int m_widget_dirty = ALL_DIRTY;
        bool m_is_visible = false;
        bool m_no_y_update = false;

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
        bool m_is_root = false; /* Is used in ab_converter */

        std::vector<AB::Boundaries> m_text_boundaries;
        AB::Attributes m_attributes;

        // Widget position and size
        ImVec2 m_position;
        ImVec2 m_dimensions;
        ImVec2 m_ext_dimensions;
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
        void virtual setWindowWidth(float width);
    };

    class RootNode {
    private:
        AbstractElementPtr m_ptr = nullptr;
    public:
        RootNode(AbstractElementPtr ptr): m_ptr(ptr) {}

        /* We don't want copy constructor to avoid accidentally deleting memory twice */
        RootNode(const RootNode&) = delete;
        RootNode& operator=(const RootNode&) = delete;

        AbstractElement& get() {
            return *m_ptr;
        }
        ~RootNode() {
            delete m_ptr;
        }
    };

    using RootNodePtr = std::shared_ptr<RootNode>;
}