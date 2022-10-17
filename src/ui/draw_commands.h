#pragma once

#include <tempo.h>
#include <list>

namespace Draw {
    class DrawList {
    private:
        ImDrawListSplitter m_splitter;
        ImDrawList* m_draw_list = nullptr;
    public:
        DrawList(ImDrawList* draw_list) { 
            m_draw_list = draw_list;
        }
        DrawList() {}

        ImDrawList* operator->() { return m_draw_list; }
        ImDrawList* operator*() { return m_draw_list; }
        void SetImDrawList(ImDrawList* draw_list) { m_draw_list = draw_list; }
        void Split(int count) { m_splitter.Split(m_draw_list, count); }
        void Merge()  { m_splitter.Merge(m_draw_list); }
        void SetCurrentChannel(int channel_idx) { m_splitter.SetCurrentChannel(m_draw_list, channel_idx); }
    };
}