#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <functional>


template <typename StateT, typename EventT>
class StateMachine {
public:

    using Action = std::function<void()>;


    void start(StateT initalState) {

    }

    void addTransition(EventT event, StateT sourceState, StateT targetState) {

    }

    void addTransition(EventT event, StateT sourceState, StateT targetState, Action action) {

    }

    bool dispatch(EventT ev) {

    }


    StateT getState() const {
        return m_currentState;
    }

private:

    void setState(StateT state) {
        m_currentState = state;
    }

    StateT m_currentState;
};


#endif // !STATE_MACHINE_HPP
