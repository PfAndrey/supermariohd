#pragma once

#include "GameEngine.hpp"

class Star : public GameObject {
public:
    Star(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    enum State {
        WAIT    = 1,
        BORNING = 2,
        NORMAL  = 3
    };

    void onStarted() override;

    const float GRAVITY_FORCE = 0.0015f;
    const float JUMP_POWER = 0.6f;
    const float HORIZONTAL_SPEED = 0.1f;
    State m_state = State::WAIT;
    sf::Sprite m_sprite;
    float m_timer = 0;
    Vector m_speed;
    Mario* m_mario = nullptr;
    Blocks* m_blocks = nullptr;
};
