#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"
#include <StateMachine.hpp>

class Goomba : public Enemy {
public:
    Goomba();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class Event : uint8_t {
        ENTERED_VIEW   = 0,
        STOMPED        = 1,
        BUMPED         = 2,
        PROJECTILE_HIT = 3,
    };

    enum class State : uint8_t {
        DEACTIVATED = 0,
        WALKING     = 1,
        CRAMPED     = 2,
        DIED        = 3
    };

    fsm::StateMachine<State, Event> m_stateMachine;
    float m_timer = 0;
};

#endif // GOOMBA_HPP
