#pragma once

#include "Enemy.hpp"

class Bowser : public Enemy {
    DECLARE_TYPE_INFO(Enemy)

public:
    Bowser();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;
    void noBridge();
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        WALK = 0,
        TURN = 1,
        JUMP = 2,
        PRE_JUMP = 3,
        MIDDLE_FIRE = 4,
        LAND_FIRE = 5,
        NO_BRIDGE = 6,
        FALL = 7,
        DIED = 8
    };

    void fire(const Vector& fireBallOffset);
    void enterState(State state);

    static constexpr int WALK_AMPLITUDE = 100;
    static constexpr int JUMP_PERIOD = 4000;
    static constexpr float ANIM_SPEED = 0.008f;
    State m_state = State::WALK;
    int m_lives = 5;
    int m_delay_timer = 0;
    float m_center_x = 0.f;
    Vector m_old_speed;
};
