#include "time_counter.h"

#include <iostream>
#include <algorithm>

void TimeCounter::manage() {
    m_num_loops++;

    for (auto pair : m_timers) {
        float total_time = 0.f;
        int i = 0;
        for (auto timer : pair.second.timers) {
            total_time += timer.total_time;
            i++;
        }
        pair.second.num_calls_per_loop.push_back(i);
        pair.second.cumulative_times.push_back(total_time);
    }
    if (m_num_loops > m_display_after) {
        m_num_loops = 0;
        int timer_counter = 0;
        for (const auto& pair : m_timers) {
            float cumulative_time = 0.f;
            int num_calls = 0;
            for(int i = 0;i < pair.second.cumulative_times.size();i++) {
                cumulative_time += pair.second.cumulative_times[i];
                num_calls += pair.second.num_calls_per_loop[i];
            }
            if (timer_counter > 0)
                std::cout << " :: ";
            std::cout << pair.first << "(" << int(cumulative_time/1000.f) << "mus;" 
                << int(cumulative_time/(float)num_calls/1000.f) << ")";
            timer_counter++;
        }
        if (timer_counter)
            std::cout << std::endl;
        m_timers.clear();
    }
}

void TimeCounter::startCounter(const std::string& name) {
    if (m_timers.find(name) == m_timers.end()) {
        m_timers[name] = TimerCollection{};
    }
    m_timers[name].timers.push_back({ std::chrono::high_resolution_clock::now() });
    auto t = std::chrono::high_resolution_clock::now();
}
void TimeCounter::stopCounter(const std::string& name) {
    if (m_timers.find(name) != m_timers.end()) {
        if (m_timers[name].timers.size()) {
            auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now() - m_timers[name].timers.back().start);
            m_timers[name].timers.back().total_time = t.count();
        }
    }
}