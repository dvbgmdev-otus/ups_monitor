/**
 * @file ups_state_buffer.cpp
 * @ingroup ups
 * @brief Реализация потокобезопасного контейнера состояния UPS.
 */
#include "ups_state_buffer.h"

namespace ups {

void UpsStateBuffer::storeState(const UpsState& state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = state;
    m_hasState = true;
}

bool UpsStateBuffer::tryConsumeState(UpsState& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_hasState) {
        return false;
    }
    out = m_state;
    m_hasState = false;
    return true;
}

}  // namespace ups
