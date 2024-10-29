#ifndef TIMER_MANAGER_HPP
#define TIMER_MANAGER_HPP

#include <functional>
#include <list>

class TimerManager {
public:
    int setTimer(const std::function <void()>& func, int delay);

    template<typename Class, typename ...Args>
    int setTimer(Class* object, void (Class::* func)(Args...), int delay, Args... args) {
        return setTimer([object, func, args...]() { (object->*func)(args...); }, delay);
    }

    float getTimerRemaining(int handle) const;
    void clearTimer(int id);

    void update(int delta_time);
    void setPause(bool paused);

private:

    struct TimerData {
        int id;
        int remain_time;
        std::function<void()> func;
    };

    int m_last_id = 0;
    bool m_paused = false;
    std::list<TimerData> m_invoke_list;
};

#endif // !TIMER_MANAGER_HPP
