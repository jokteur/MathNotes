#pragma once 

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    float total_time = 0.f;
};
struct TimerCollection {
    std::vector<Timer> timers;
    std::vector<int> num_calls_per_loop;
    std::vector<float> cumulative_times;
};

class TimeCounter {
private:
    std::unordered_map <std::string, TimerCollection> m_timers;
    int m_num_loops = 0;
    int m_display_after = 240;

    TimeCounter() {}
public:
    TimeCounter(TimeCounter const&) = delete;
    void operator=(TimeCounter const&) = delete;

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static TimeCounter& getInstance() {
        static TimeCounter instance;
        return instance;
    }

    /* To be called after everyloop
     * Will display statistics after n loops (parameter from constructor) */
    void manage();
    void startCounter(const std::string& name);
    void stopCounter(const std::string& name);
};