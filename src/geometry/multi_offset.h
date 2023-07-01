#pragma once

#include <unordered_map>
#include <vector>

class MultiOffset {
private:
    std::unordered_map<int, float> m_offsets;
    float m_min = 1e9;
    float m_max = -1e9;
    float m_additional_offset = 0.f;
public:
    MultiOffset& operator+=(float offset);
    MultiOffset& operator-=(float offset);
    void addOffset(int line_number, float offset);

    float getOffset(int line_number) const;

    void clear();
    void clear(const std::vector<int>& lines);
    void clear(int from, int to);

    float getMin() const;
    float getMax() const;
};