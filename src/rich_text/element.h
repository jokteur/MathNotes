#pragma once

#include <tempo.h>
#include <unordered_set>
#include "ab_parser.h"

#include "ab/ab_file.h"
#include "ui/drawable.h"
#include "fonts/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"

#include "geometry/basic.h"
#include "geometry/multi_boundaries.h"
#include "geometry/multi_offset.h"
#include "types.h"

#include "widgets_enum.h"
#include "rich_text_context.h"
#include "ab_config.h"
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

    class TextCursor;

    struct LineInfo {
        float position = 0.f;
        float height = 0.f;
        float ascent = 0.f;
        float descent = 0.f;
    };

    typedef std::unordered_map<int, LineInfo> LinesInfos;

    struct DrawContext {
        Draw::DrawList* draw_list;
        float cursor_y_pos = 0.f;
        /* For children, there can be multiple x offset on each line */
        MultiOffset x_offset;
        Rect boundaries;
        LinesInfos lines;
        // WrapDocument* doc;
        std::vector<TextCursor>* cursors;
        ImVec2 draw_offset;
        float line_height = 0.f;
        bool force_dirty_height = false;
        bool rebuild = false;
    };

    struct AbstractElement : public Drawable {
    protected:
        // WrapColumn m_chars;
        WrapColumn m_text_column;
        float m_y_offset = 0.f;

        void inline set_dirty_all();
    public:
        static int count;
        static int visible_count;

        const unsigned int DIRTY_WIDTH = 0x1;
        const unsigned int DIRTY_CHARS = 0x2;
        const unsigned int DIRTY_HEIGHT = 0x4;
        const unsigned int ALL_DIRTY = DIRTY_WIDTH | DIRTY_CHARS | DIRTY_HEIGHT;

        Type m_type;
        Category m_category;
        AbstractElement();
        virtual ~AbstractElement();

        void set_selected_check(DrawContext* context, bool is_selected);
        void set_selected_pre_only(DrawContext* context);
        void set_selected_all(DrawContext* context);
        void set_selected_never(DrawContext* context);
        void set_selected_always(DrawContext* context);
        void set_selected(DrawContext* context);

        // Informations about the tree structure
        std::vector<AbstractElementPtr> m_childrens;
        AbstractElementPtr m_parent = nullptr;
        AB::RootBlockWeakPtr m_ref_to_root;

        unsigned int m_widget_dirty = ALL_DIRTY;
        bool m_is_visible = false;
        bool m_has_content = true;
        void (AbstractElement::* m_cursor_set)(DrawContext*);

        // Returns false if not succesfully build chars
        bool virtual add_chars(WrapColumn* wrap_chars);
        bool virtual draw(DrawContext* context);

        bool is_in_boundaries(const Rect& boundaries);

        // Build hooks
        bool virtual hk_build_chars(DrawContext* context);
        bool virtual hk_build(DrawContext* context);

        // New hooks for refactoring
        bool virtual hk_build_hlayout(DrawContext* context);
        bool virtual hk_build_vlayout(DrawContext* context, int line_number = -1, bool force = false);
        void virtual hk_set_x_origin(DrawContext* context);
        void virtual hk_set_y_origin(DrawContext* context);
        // void virtual hk_set_x_dim(DrawContext* context);
        void virtual hk_set_y_dim(DrawContext* context);


        // Draw hooks
        /* Implement this function if you want to draw visual elements
         * that are not part of fundamental widget construction and char drawing */
        void virtual hk_get_line_info(DrawContext* context, int line_number, LineInfo& line_info);
        bool virtual hk_draw_secondary(DrawContext* context);
        void virtual hk_draw_background(Draw::DrawList* draw_list);
        void virtual hk_draw_show_boundaries(DrawContext* context);
        void virtual hk_draw_text_cursor(DrawContext* context);

        /* Debug prints object info in a special window created by parent */
        void virtual hk_debug(const std::string& prefix = "");
        void virtual hk_debug_attributes();

        void displaceYOrigin(float displacement);

        Style m_style;
        Style m_special_chars_style;

        bool m_is_selected = false;
        WrapAlgorithm m_wrapper;

        // Position of the pointer in m_childrens;
        bool m_is_root = false; /* Is used in ab_converter */
        int m_tree_level = 0;

        std::vector<AB::Boundaries> m_text_boundaries;
        AB::Attributes m_attributes;

        // Widget position and size
        Rect m_ext_dimensions = { 0.f, -1.f };
        Rect m_int_dimensions = { 0.f, -1.f };

        bool m_is_dimension_set = false;
        float m_scale = 1.f;
        float m_window_width = 1.f;

        // Debug
        bool m_show_boundaries = false;

        // Internal
        SafeString m_safe_string;

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
        // Lines m_lines;
        RootNode(AbstractElementPtr ptr) : m_ptr(ptr) {}

        /* We don't want copy constructor to avoid accidentally deleting memory twice */
        RootNode(const RootNode&) = delete;
        RootNode& operator=(const RootNode&) = delete;

        AbstractElement& get() {
            return *m_ptr;
        }
        AbstractElementPtr getPtr() {
            return m_ptr;
        }
        ~RootNode() {
            delete m_ptr;
        }
    };

    using RootNodePtr = std::shared_ptr<RootNode>;
}