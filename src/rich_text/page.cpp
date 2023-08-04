#include "page.h"

#include <cmath>

#include "ab/ab_file.h"
#include "ab_converter.h"
#include "ui/ui_utils.h"

namespace RichText {
    Page::Page(AB::File* file) : Drawable(), m_mem(file), m_file(file), m_display(&m_mem) {
        m_text_cursors.push_back(TextCursor(&m_mem, m_file));
        PageMemory::MemoryState dummy;
        m_mem.manage(dummy);
    }

    void Page::debug_window() {
        ImGui::Begin("Widget info");
        ImGui::Text("current_line: %d", m_mem.getCurrentLine());
        ImGui::Text("current block idx: %d", m_mem.getCurrentBlockIdx());
        m_display.debugInfo();
        ImGui::Text("Element count: %d", AbstractElement::count);
        // ImGui::Text("String count: %d", WrapString::count);
        ImGui::Text("Font char count: %d", (int)m_ui_state.font_manager.debugGetChars().size());
        ImGui::Text("Visible count: %d", AbstractElement::visible_count);

        ImGui::Separator();
        ImGui::End();

        ImGui::Begin("Parsed blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto& pair : m_mem.getElements()) {
                pair.second->get().hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Visible blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto& pair : m_mem.getElements()) {
                auto& ptr = pair.second->get();
                if (ptr.m_is_visible || !ptr.m_is_dimension_set || ptr.m_widget_dirty)
                    ptr.hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Top displayed block");
        if (m_mem.getCurrentBlock() != nullptr) {
            m_mem.getCurrentBlock()->get().hk_debug();
        }
        ImGui::End();

        ImGui::Begin("AB File");
        if (ImGui::TreeNode("Show")) {
            if (ImGui::CollapsingHeader("Roots")) {
                int num_roots_blocks = m_file->m_blocks.size();
                if (m_debug_root_max > num_roots_blocks) {
                    m_debug_root_max = num_roots_blocks;
                }
                ImGui::DragIntRange2(("Show root idx (max " + std::to_string(num_roots_blocks) + ")").c_str(),
                    &m_debug_root_min,
                    &m_debug_root_max,
                    1.f, 0,
                    num_roots_blocks);
                for (int i = m_debug_root_min;i < m_debug_root_max;i++) {
                    std::string str;
                    auto& block = m_file->m_blocks[i];
                    AB::Boundaries bounds{ block->line_start, block->idx_start, block->idx_start, block->idx_end, block->idx_end };
                    AB::str_from_text_boundaries(*m_file->m_safe_txt, str, { bounds });

                    int length = str.length();
                    std::string suffix = str;
                    if (length > 10) {
                        suffix = suffix.substr(0, 10) + "...";
                    }
                    suffix = "  /" + suffix;
                    ImGui::Text("%d %s %s", i, AB::block_to_name(block->type), suffix.c_str());
                }
            }
            if (ImGui::CollapsingHeader("Headers")) {
            }
            if (ImGui::CollapsingHeader("Equations")) {
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

    void Page::setLine(int line_number) {
        m_mem.gotoLine(line_number);
    }

    void Page::FrameUpdate() {
        //ZoneScoped;

        m_window_name = "RichText window ##drawable " + m_name;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Begin(m_window_name.c_str());

        manage_cursors();

        DrawContext ctx;
        ctx.lines = m_lines_infos;
        ctx.cursors = &m_text_cursors;

        m_display.setMinScrollHeight(m_config.min_scroll_height);
        m_display.FrameUpdate(m_window_name, &ctx);

        m_lines_infos = ctx.lines;
        ImGui::End();
        ImGui::PopStyleColor();

        debug_window();
    }

    /* ======================
     * Text Cursor management
     * ====================== */
    void Page::manage_cursors() {
        for (auto& cursor : m_text_cursors) {
            cursor.manage();
        }
    }
    Page::~Page() {
    }
}