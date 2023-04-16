#include "page.h"

#include <cmath>

#include "ab/ab_file.h"
#include "ab_converter.h"

namespace RichText {
    void Page::debug_window() {
        ImGui::Begin("Widget info");
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("current_line: %d", m_current_line);
        ImGui::Text("current block idx: %d", m_current_block_idx);
        ImGui::End();

        ImGui::Begin("Parsed blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto pair : m_root_elements) {
                pair.second->hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Top displayed block");
        if (m_current_block_ptr != nullptr) {
            m_current_block_ptr->hk_debug();
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
                    AB::Boundaries bounds{block->line_start, block->idx_start, block->idx_start, block->idx_end, block->idx_end};
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

    void Page::draw() {
        //ZoneScoped;

        manage_jobs();

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Begin("RichText window");
        float width = ImGui::GetWindowContentRegionWidth();
        width -= ImGui::GetStyle().ScrollbarSize;
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        auto mouse_pos = ImGui::GetMousePos();

        vMin.y += 0;
        vMax.y -= 0;

        /* Once we know the height of the page,
         * we can estimate how many lines we should
         * look ahead for block element construction*/
        m_display_height = vMax.y - vMin.y;
        calculate_heights();
        manage_elements();

        {
            std::lock_guard lk(m_root_mutex);
            find_current_ptr();

            /* Set the width of the blocks recursively, even the ones that are not shown */
            for (auto pair : m_root_elements) {
                if (m_current_width != width || pair.second->m_widget_dirty) {
                    pair.second->setWidth(width);
                }
            }
            if (m_current_width != width) {
                m_current_width = width;
            }


            if (!m_root_elements.empty()) {
                manage_scroll(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });

                Rect boundaries;
                boundaries.y = 0.f;
                boundaries.h = vMax.y - vMin.y;
                boundaries.w = width;
                m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

                m_draw_list->AddRect(vMin, vMax, Colors::red, 0.f, 0, 2.f);

                display_scrollbar(Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });


                // Background, ForeGround
                m_draw_list.Split(2);
                m_draw_list.SetCurrentChannel(1);

                /* We want to separate elements in two groups:
                 * - before m_current_line
                 * - after m_current_line
                 *
                 * All the elements before should not be displayed, but they are still important
                 * to be build once to calculate the scroll height
                 *
                 * As the elements height can only be known when displaying them once, we still need
                 * to call the draw fct. This is why we first draw them safely outside of the screen
                 *
                 * Once we reach the m_current_line element, then we can set y_pos properly
                 */
                bool found_current = false;
                float y_pos = m_display_height + 1000.f;
                /* Designates the height taken by the elements before the current one */
                float before_height = 0.f;
                for (auto pair : m_root_elements) {
                    if (!found_current && pair.first >= m_current_block_idx) {
                        found_current = true;
                        before_height = y_pos - m_display_height - 1000.f;
                        y_pos = -roundf(m_y_displacement);
                    }
                    pair.second->draw(m_draw_list, y_pos, 0.f, boundaries);
                }
                m_draw_list.Merge();
            }

            ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
            ImGui::End();
            ImGui::PopStyleColor();

            debug_window();
        }
    }
    void Page::display_scrollbar(const Rect& b) {
        float scroll_width = ImGui::GetStyle().ScrollbarSize;
        float rounding = ImGui::GetStyle().ScrollbarRounding;
        auto color_bg = ImGui::GetColorU32(ImGuiCol_ScrollbarBg);
        ImVec2 top_left(5 + b.w + b.x - scroll_width, b.y);
        ImVec2 bottom_right(5 + b.w + b.x, b.h + b.y);
        m_draw_list->AddRectFilled(top_left, bottom_right, color_bg, rounding);
    }
    void Page::manage_scroll(const ImVec2& mouse_pos, const Rect& box) {
        if (isInsideRect(mouse_pos, box)) {
            float mouse_wheel = ImGui::GetIO().MouseWheel;
            if (ImGui::IsKeyDown(ImGuiKey_ModShift))
                mouse_wheel *= 70;
            else
                mouse_wheel *= 20;

            if (mouse_wheel > 0.f) {
                scroll_up(mouse_wheel);
            }
            else if (mouse_wheel < 0.f) {
                scroll_down(mouse_wheel);
            }
        }
    }

    void Page::calculate_heights() {
        if (m_recalculate_line_height) {
            using namespace Fonts;
            FontRequestInfo font_request;
            font_request.size_wish = 18.f;
            FontInfoOut font_out;
            m_ui_state->font_manager.requestFont(font_request, font_out);
            auto font = Tempo::GetImFont(font_out.font_id);
            if (font->im_font != nullptr) {
                m_recalculate_line_height = false;
                m_line_height = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();
            }
        }
        float lines_per_display = m_display_height / m_line_height;
        float num_pages_for_min_scroll = m_display_height / (m_config.min_scroll_height * Tempo::GetScaling());
        m_line_lookahead_window = num_pages_for_min_scroll * lines_per_display;
    }
    std::map<int, AbstractElementPtr>::iterator Page::find_prev_ptr() {
        //ZoneScoped;
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end() || current_it == m_root_elements.begin())
            return current_it;
        return std::prev(current_it);
    }
    std::map<int, AbstractElementPtr>::iterator Page::find_next_ptr() {
        //ZoneScoped;
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end()) {
            return current_it;
        }
        auto next = std::next(current_it);
        return next;
    }
    void Page::find_current_ptr() {
        //ZoneScoped;
        auto bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line + 1);
        if (m_root_elements.find(bounds.start.block_idx) != m_root_elements.end()) {
            m_current_block_idx = bounds.start.block_idx;
            m_current_block_ptr = m_root_elements[m_current_block_idx];
        }
    }
    void Page::go_to_line(int line_number) {
        //ZoneScoped;
        m_current_line = line_number;
        find_current_ptr();
        if (m_current_block_ptr != nullptr) {
            m_current_line = m_current_block_ptr->m_text_boundaries.front().line_number;
        }
    }
    void Page::scroll_down(float pixels) {
        //ZoneScoped;
        if (m_current_block_ptr == nullptr)
            return;

        pixels = abs(pixels);
        std::cout << "Scroll down " << pixels << std::endl;

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->m_dimensions.y - m_y_displacement;
        if (pixels < remaining_height) {
            m_y_displacement += pixels;
            return;
        }
        bool arrived_at_end = false;
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            auto next = find_next_ptr();
            if (next == m_root_elements.end()) {
                arrived_at_end = true;
                break;
            }
            go_to_line(next->second->m_text_boundaries.front().line_number);
            float element_height = m_current_block_ptr->m_dimensions.y;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = element_height - (total_height - pixels);
                return;
            }
        }
    }
    void Page::scroll_up(float pixels) {
        //ZoneScoped;
        if (m_current_block_ptr == nullptr)
            return;

        std::cout << "Scroll up " << pixels << std::endl;

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->m_dimensions.y - m_y_displacement;
        if (pixels <= m_y_displacement || m_current_block_idx == 0) {
            m_y_displacement -= pixels;
            if (m_y_displacement < 0.f)
                m_y_displacement = 0.f;
            return;
        }
        bool arrived_at_beg = false;
        float total_height = remaining_height;
        while (true) {
            auto prev = find_prev_ptr();
            if (prev == m_root_elements.begin() && prev->first > 0) {
                arrived_at_beg = true;
                break;
            }
            go_to_line(prev->second->m_text_boundaries.front().line_number);
            if (prev->second->m_dimensions.y == 0.f && prev->first > 0) {
                continue;
            }
            float element_height = m_current_block_ptr->m_dimensions.y;
            total_height += element_height;
            if (pixels <= total_height) {
                m_y_displacement = element_height - pixels;
                if (m_y_displacement < 0.f)
                    m_y_displacement = 0.f;
                return;
            }
        }
    }

    void Page::manage_elements() {
        //ZoneScoped;
        manage_jobs();

        int half_window = m_line_lookahead_window / 2;
        int start_line = m_current_line - half_window;
        int end_line = m_current_line + half_window;
        auto bounds = m_file->getBlocksBoundsContaining(start_line, end_line);

        if (bounds.start.block_idx == -1)
            return;

        int start = bounds.start.block_idx;
        int end = bounds.end.block_idx;

        std::unordered_set<int> to_destroy;

        /* Widget was just created or jumped to another location */
        if (m_block_idx_start >= 0 && (m_block_idx_end == -1 || start >= m_block_idx_end)) {
            if (start >= m_block_idx_end) {
                std::lock_guard<std::mutex> lk(m_root_mutex);
                m_root_elements.clear();
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
            ABToWidgets parser;
            parser.parse(m_file, m_block_idx_start, m_block_idx_end, &m_root_elements, m_ui_state);
        }
        else {
            /* Blocks to build before */
            if (start < m_block_idx_start) {
                parse_job(start, m_block_idx_start);
            }
            /* Blocks to destroy before */
            else if (start > m_block_idx_start) {
                std::cout << "Destroy " << m_block_idx_start << " to " << start << " (pre)" << std::endl;
                for (int i = m_block_idx_start; i < start;i++) {
                    to_destroy.insert(i);
                }
            }
            /* Blocks to build after */
            if (end > m_block_idx_end) {
                parse_job(m_block_idx_end, end);
            }
            /* Blocks to destroy after */
            else if (end < m_block_idx_end) {
                std::cout << "Destroy " << end << " to " << m_block_idx_end << std::endl;
                for (int i = end + 1; i <= m_block_idx_end;i++) {
                    to_destroy.insert(i);
                }
            }

            {
                std::lock_guard<std::mutex> lk(m_root_mutex);
                for (auto idx : to_destroy) {
                    m_root_elements.erase(idx);
                }
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
        }
    }
    void Page::manage_jobs() {
        std::vector<Tempo::jobId> to_remove;
        for (auto job_id : m_current_jobs) {
            auto& scheduler = Tempo::JobScheduler::getInstance();
            auto job = scheduler.getJobInfo(job_id);
            std::cout << "Job " << job_id << " pending" << std::endl;
            if (job.state == Tempo::Job::JOB_STATE_NOTEXISTING) {
                to_remove.push_back(job_id);
            }
        }
        for (auto job_id : to_remove) {
            m_current_jobs.erase(job_id);
        }
    }
    void Page::parse_job(int start_idx, int end_idx) {
        /* TODO: after job is finished, check if the job is not obsolete */
        Tempo::jobFct job = [=](float& progress, bool& abort) -> std::shared_ptr<Tempo::JobResult> {
            ABToWidgets parser;
            std::map<int, AbstractElementPtr> tmp_roots;
            parser.parse(m_file, start_idx, end_idx, &tmp_roots, m_ui_state);
            {
                std::lock_guard<std::mutex> lk(m_root_mutex);
                for (auto pair : tmp_roots) {
                    m_root_elements[pair.first] = pair.second;
                }
            }
            Tempo::JobResult jr;
            jr.success = true;
            return std::make_shared<Tempo::JobResult>(jr);
        };
        auto& scheduler = Tempo::JobScheduler::getInstance();
        auto job_id = scheduler.addJob("parse_ab_segment", job);
        m_current_jobs.insert(job_id->id);
    }
    Page::~Page() {
        auto& scheduler = Tempo::JobScheduler::getInstance();
        while (!m_current_jobs.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            manage_jobs();
        }
    }
}