#include "multi_offset.h"

MultiOffset& MultiOffset::operator+=(float offset) {
    for (auto& pair : m_offsets) {
        pair.second += offset;
    }
    m_min += offset;
    m_max += offset;
    return *this;
}
MultiOffset& MultiOffset::operator-=(float offset) {
    for (auto& pair : m_offsets) {
        pair.second -= offset;
    }
    m_min -= offset;
    m_max -= offset;
    return *this;
}
float MultiOffset::getOffset(int line_number) const {
    if (m_offsets.find(line_number) == m_offsets.end()) {
        return m_min;
    }
    return m_offsets.at(line_number);
}
void MultiOffset::addOffset(int line_number, float offset) {
    if (m_offsets.find(line_number) == m_offsets.end()) {
        m_offsets[line_number] = 0.f;
    }
    m_offsets[line_number] += offset;

    m_min = 1e9;
    m_max = -1e9;
    for (auto& pair : m_offsets) {
        if (pair.second < m_min)
            m_min = pair.second;
        if (pair.second > m_max)
            m_max = pair.second;
    }
}
void MultiOffset::clear() {
    m_offsets.clear();
    m_min = 0.f;
    m_max = 0.f;
}
void MultiOffset::clear(const std::vector<int>& line_numbers) {
    m_offsets.clear();
    for (auto line_number : line_numbers) {
        m_offsets[line_number] = 0.f;
    }
    m_min = 0.f;
    m_max = 0.f;
}
void MultiOffset::clear(int from, int to) {
    m_offsets.clear();
    for (int i = from;i <= to;i++) {
        m_offsets.insert({ i, 0.f });
    }
    m_min = 0.f;
    m_max = 0.f;
}

float MultiOffset::getMin() const {
    return m_min;
}
float MultiOffset::getMax() const {
    return m_max;
}
