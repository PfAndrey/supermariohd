#pragma once

#include <Enemy.hpp>

class BuzzyBeetle : public Enemy {
public:
    BuzzyBeetle();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* charaster) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        DEACTIVATED = 0,
        NORMAL = 1,
        HIDDEN = 2,
        BULLET = 3,
        DIED = 4
    };

    void setState(State state);
    State m_state = State::DEACTIVATED;
    bool m_is_flying = false;
    float m_timer = 0;
};
