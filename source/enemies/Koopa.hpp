#ifndef KOOPA_HPP
#define KOOPA_HPP

#include <StateMachine.hpp>
#include "Enemy.hpp"

class Koopa : public Enemy {
public:
    Koopa();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* charaster) override;
    bool isAlive() const override;

private:

    enum class Event : uint8_t {
        ENTERED_VIEW   = 0, ///< Emitted when the actor enters the camera/activation bounds (wake from DEACTIVATED).
        PROJECTILE_HIT = 1, ///< A projectile struck this actor (e.g., fireball, shell); usually leads to DIED.
        STOMPED        = 2, ///< A character landed on top of this actor (stomp from above).
        BUMPED         = 3, ///< The block/platform beneath was hit from below, bumping the actor upward.
        KICKED         = 4, ///< The actor (or shell) was pushed from the side, giving it horizontal impulse.
        TIMEOUT        = 5, ///< Fired when the shell's wake-up timer expires (HIDDEN -> WAKING -> NORMAL).
    };

    enum class State : uint8_t {
        DEACTIVATED   = 0, ///< Offscreen/inactive; no physics/AI.
        WALKING       = 1, ///< Grounded/walking Koopa Troopa.
        JUMPING       = 2, ///< Paratroopa: hopping pattern (airborne).
        LEVITATING    = 3, ///< Paratroopa: hovering/flying pattern (airborne).
        HIDDEN        = 4, ///< Shell on ground, stationary (can be STOMPED/KICKED/BUMPED).
        WAKING        = 5, ///< Shell “wiggling/peeking”; after timeout returns to NORMAL.
        SHELL_SLIDING = 6, ///< Sliding shell acting as a projectile.
        DEAD          = 7  ///< Dead/falling; pending despawn.
    };

    //void setState(State state);
    void onStarted() override;

    void enterShell();
    void exitShell();

    void enterShellSliding();
    void exitShellSliding();

    void enterWalking();
    void enterDead();

    const Vector FULL_SIZE   = { 32,48 };
    const Vector HIDDEN_SIZE = { 32,32 };

    State m_initial_state = State::WALKING;

    fsm::StateMachine<State, Event> m_stateMachine;

    float m_timer = 0;
    Vector m_initial_pos;
};

#endif // KOOPA_HPP
