#pragma once

#include "GameEngine.hpp"

class Coin : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Coin();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void kick();

private:

    enum class State {
        TWIST = 0,
        SHANE = 1
    };
 
    void onStarted() override;
    State m_state = State::TWIST;
    int m_remove_timer = 1000;
    Animator m_animator;
    class Mario* m_mario = nullptr;
};