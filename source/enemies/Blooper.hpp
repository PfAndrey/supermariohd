#pragma once

#include "Enemy.hpp"

class Blooper : public Enemy {
public:
    Blooper();
    void update(int delta_time) override;
    void draw(sf::RenderWindow* render_window) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum State : uint8_t {
        ZIG = 0,
        ZAG = 1,
        DIED = 2
    };

    void enterState(State state);

    State m_state = State::ZIG;
    int m_delay_time = 0;
    Vector m_speed;
};
