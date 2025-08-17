#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <optional>
#include <functional>
#include <unordered_map>
#include <vector>
#include <map>
#include <utility>


namespace fsm {

struct ANY_STATE_T {};
static const ANY_STATE_T ANY_STATE;

template <typename StateT, typename EventT>
class StateMachine {
public:
    using Action = std::function<void()>;
    using StateCallback = std::function<void(StateT)>;

    void start(StateT initialState) {
        setStateInternal(initialState);
    }

    void addTransition(EventT event, StateT from, StateT to) {
        m_transitions.push_back({ from, event, to, Action{} });
    }

    void addTransition(EventT event, StateT from, StateT to, Action a) {
        m_transitions.push_back({ from, event, to, std::move(a) });
    }

    template <typename C>
    void addTransition(EventT event, StateT from, StateT to, C* obj, void (C::* method)()) {
        m_transitions.push_back({ from, event, to, [obj, method]() { (obj->*method)(); } });
    }

    void addTransition(EventT event, ANY_STATE_T from, StateT to, Action a) {
        m_transitions.push_back({ std::nullopt, event, to, std::move(a) });
    }

    void dispatchEvent(const EventT& ev) {
        for (auto& t : m_transitions) {
            bool matchState = (!t.from.has_value() || (t.from == m_current));
            bool matchEvent = (t.event == ev);
            if (matchState && matchEvent) {
                setStateInternal(t.to);
                if (t.action) {
                    t.action();
                }
                return;
            }
        }
    }

    template <class T>
    void attachOnEnterMap(std::function<void(T)> functor, std::map<StateT, T> actionMap) {
        for (auto& actionItem : actionMap) {
            auto state = actionItem.first;
            auto param = actionItem.second;
            m_onEnterActions[state].push_back([=]() { functor(param); });
        }
    }

    template <class Obj, class T>
    void attachOnEnterMap(Obj* obj, void (Obj::* method)(const T&),
        const std::map<StateT, T>& actionMap) {
        attachOnEnterMap<T>(
            std::function<void(const T&)>([obj, method](const T& v) { (obj->*method)(v); }),
            actionMap
        );
    }


    StateT getState() const {
        return m_current;
    }

    void setStateCallback(StateCallback cb) {
         m_onState = std::move(cb);
    }

private:
    struct Transition {
        std::optional<StateT> from;
        EventT event;
        StateT to;
        Action action;
        bool fromAnyState = false;
    };

    void setStateInternal(StateT s) {
        m_current = s;
        if (m_onState) {
            m_onState(s);
        }

        auto it = m_onEnterActions.find(s);
        if (it != m_onEnterActions.end()) {
            for (auto onAction : it->second) {
                onAction();
            }
        }

    }

    std::vector<Transition>  m_transitions;
    StateT                   m_current{};
    StateCallback            m_onState;
    std::unordered_map<StateT, std::vector<std::function<void()>> > m_onEnterActions;
};

} // namespace fsm

#endif // !STATE_MACHINE_HPP
