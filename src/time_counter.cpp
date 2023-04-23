#include "time_counter.h"

#include <iostream>
#include <algorithm>

void TimeCounter::manage() {
    m_num_loops++;
    if (m_num_loops > m_display_after) {
        m_num_loops = 0;
        int timer_counter = 0;
        for (auto pair : m_timers) {
            /* Calculate average */
            float total_time = 0.f;
            int i = 0;
            for (auto timer : pair.second) {
                total_time += timer.total_time;
                i++;
            }
            if (timer_counter > 0)
                std::cout << " :: ";
            std::cout << pair.first << "(" << int(total_time / i) << "mus)";
            timer_counter++;
        }
        if (timer_counter)
            std::cout << std::endl;
        m_timers.clear();
    }
}

void TimeCounter::startCounter(const std::string& name) {
    m_timers[name].push_back({ std::chrono::high_resolution_clock::now() });
    auto t = std::chrono::high_resolution_clock::now();
}
void TimeCounter::stopCounter(const std::string& name) {
    if (m_timers.find(name) != m_timers.end()) {
        if (m_timers[name].size()) {
            auto t = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - m_timers[name].back().start);
            m_timers[name].back().total_time = t.count();
        }
    }
}