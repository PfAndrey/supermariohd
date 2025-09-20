#pragma once

#include "Enemy.hpp"

class Podoboo : public Enemy {
public:
    Podoboo();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:
    static constexpr float PERIOD_TIME = 3000.f;
    static constexpr float AMPLITUDE = 400.f;
    const Vector SIZE = { 32,32 };
    float m_timer = 0;
    float m_acceleration = 0;
    float m_max_speed;
    Vector m_center;
    Vector m_velocity;
};
