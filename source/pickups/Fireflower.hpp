#pragma once

#include "GameEngine.hpp"

class FireFlower : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    FireFlower(const Vector& pos);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

protected:

    void onStarted() override;
    virtual void action();
    sf::Sprite m_sprite;

private:
    enum class State {
        WAIT = 0,
        GROVING = 1,
        NORMAL = 2
    };

    const float GRAVITY_FORCE = 0.0015f;
    const float GROVING_RATE = 0.02f;
    State m_state = State::WAIT;
    Vector m_speed;
    float m_timer = 0;
    class Mario* m_mario = nullptr;
    class Blocks* m_blocks = nullptr;
};
