#include "TimerManager.hpp"

int TimerManager::setTimer(const std::function <void()>& func, int delay) {
    TimerData timer_data;
    timer_data.id = m_last_id++;
    timer_data.remain_time = delay;
    timer_data.func = func;
    m_invoke_list.push_back(timer_data);
    return timer_data.id;
}

void TimerManager::update(int delta_time) {
    if (m_paused) {
        return;
    }

    for (auto it = m_invoke_list.begin(); it != m_invoke_list.end();) {
        it->remain_time -= delta_time;

        if (it->remain_time <= 0) {
            it->func();
            it = m_invoke_list.erase(it);
        } else {
            ++it;
        }
    }
}

float TimerManager::getTimerRemaining(int handle) const {
    auto it = std::find_if(m_invoke_list.begin(), m_invoke_list.end(), [handle](const TimerData& data)
        {
            return data.id == handle;
        });

    if (it != m_invoke_list.end()) {
        return it->remain_time;
    }

    return -1;
}

void TimerManager::clearTimer(int id) {
    auto it = std::find_if(m_invoke_list.begin(), m_invoke_list.end(), [id](const TimerData& data) {
            return data.id == id;
        });

    if (it != m_invoke_list.end()) {
        m_invoke_list.erase(it);
    }
}

void TimerManager::setPause(bool paused) {
    m_paused = paused;
}
