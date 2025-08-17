#pragma once

#include "Enemy.hpp"

class Fireball : public GameObject {
public:
    Fireball(const Vector& Position, const Vector& SpeedVector);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    void onStarted() override;

    int m_life_timer = 10000;
    Mario* m_mario = nullptr;
    Vector m_speed;
    Animator m_animator;
};
