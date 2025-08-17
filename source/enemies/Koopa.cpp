#include "SuperMarioGame.hpp"
#include "Koopa.hpp"

struct StateActions {
    std::function<void()> onEnter;
    std::function<void()> onLeave;
    std::function<void(int)> onUpdate;
};

Koopa::Koopa() {
    setSize({ 32, 48 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk",   texture, { { 0,32,32,48 },{ 32,32,32,48 } }, 0.005f);
    m_animator.create("flying", texture, { 224,32 }, { 32, 48 }, 2, 1, 0.005f);
    m_animator.create("climb",  texture, { { 64,48,32,32 },{ 192,48,32,32 } }, 0.005f);
    m_animator.create("hidden", texture, { 64,48,32,32 });
    m_animator.create("bullet", texture, { 64, 48 }, { 32, 32 }, 4, 1, 0.01f);
    m_animator.create("fall",   texture, { 0,80, 32, -48 });

    //                            timeout          timeout         stomped
    //                              ╭---- [WAKING] -----╮    ╭------------------╮
    //                stomped       ↓       stomped     |    ↓   kicked         |
    //  [LEVITATING] ----------→ [WALKING] ----------→ [HIDDEN] -------→ [SHELL_SLIDING]
    //                stomped       ↑
    //  [JUMPING] ------------------╯           hit_by_projectile / bumped
    //                                  <ANY_STATE> ---------------------→ [DEAD]

    m_stateMachine.addTransition(Event::STOMPED, State::LEVITATING,    State::WALKING,         [this]() { enterWalking(); });
    m_stateMachine.addTransition(Event::STOMPED, State::JUMPING,       State::WALKING,         [this]() { enterWalking(); });
    m_stateMachine.addTransition(Event::STOMPED, State::WALKING,       State::HIDDEN,          [this]() { enterShell(); });
    m_stateMachine.addTransition(Event::KICKED,  State::HIDDEN,        State::SHELL_SLIDING,   [this]() { enterShellSliding(); });
    m_stateMachine.addTransition(Event::STOMPED, State::SHELL_SLIDING, State::HIDDEN,          [this]() { exitShellSliding(); });
    m_stateMachine.addTransition(Event::TIMEOUT, State::HIDDEN,        State::WAKING);
    m_stateMachine.addTransition(Event::TIMEOUT, State::WAKING,        State::WALKING,         [this]() { exitShell(); enterWalking(); });
    m_stateMachine.addTransition(Event::STOMPED, State::HIDDEN,        State::SHELL_SLIDING,   [this]() { enterShellSliding(); });
    m_stateMachine.addTransition(Event::PROJECTILE_HIT, fsm::ANY_STATE,State::DEAD,            [this]() { enterDead(); });

    m_stateMachine.attachOnEnterMap<>(&m_animator, &Animator::play,
        { { State::JUMPING       , "flying" },
          { State::LEVITATING    , "flying" },
          { State::WALKING       , "walk"   },
          { State::HIDDEN        , "hidden" },
          { State::SHELL_SLIDING , "bullet" },
          { State::WAKING        , "climb"  },
          { State::DEAD          , "fall"   },
        });
}

void Koopa::exitShell() {
    // grow to full
    move(getBounds().size() - FULL_SIZE);
    setSize(FULL_SIZE);
    m_timer = 0;
}

void Koopa::enterShell() {
    // shrink to shell
    move(getBounds().size() - HIDDEN_SIZE);
    setSize(HIDDEN_SIZE);
    m_velocity = Vector::ZERO;
    addScoreToPlayer(100);
}

void Koopa::enterShellSliding() {
    m_velocity.x = isCharacterInFront(mario(), this) ? -std::abs(SHELL_SLIDING_SPEED)
                                                     : std::abs(SHELL_SLIDING_SPEED);
    move(15 * Vector::RIGHT * math::sign(m_velocity.x));
}

void Koopa::exitShellSliding() {
    m_velocity.x = 0;
}

void Koopa::enterWalking() {
    setVelocity({ RUN_SPEED, 0});
}

void Koopa::enterDead() {
    m_velocity.x = 0;
    m_velocity.y = -0.4f;
    MARIO_GAME.playSound("kick");
    addScoreToPlayer(500);
}

void Koopa::update(int delta_time) {
    Enemy::update(delta_time);
 
    const auto state = m_stateMachine.getState();

    bool physicsAndCollisionsOn = (state != State::DEAD) &&
                                  (state != State::LEVITATING) &&
                                  (state != State::DEACTIVATED);

    if (physicsAndCollisionsOn) {
        updatePhysics(delta_time, (state != State::JUMPING) ? GRAVITY_FORCE
                                                            : 0.5 * GRAVITY_FORCE);
        updateCollision(delta_time, LogicFlags::ON_X_BOUND);
    }
 
    m_animator.update(delta_time);
    m_animator.flipX(m_velocity.x > 0);

    switch (state) {
    case State::DEACTIVATED:
        if (isInCamera()) {
            // setState(m_initial_state);
            m_stateMachine.start(m_initial_state);
            m_velocity.x = RUN_SPEED;
        }
        break;
    case State::WALKING:
        checkNextTileUnderFoots();
        break;
    case State::JUMPING:
        if (m_collision_tag & ECollisionTag::FLOOR) { // touch floor
            m_velocity.y = -0.4f; // jump
        }
        break;
    case State::LEVITATING:
        m_timer += delta_time;
        setPosition(m_initial_pos + Vector::UP * sin(m_timer / 800.f) * 50);
        break;
    case State::HIDDEN:
        m_timer += delta_time;
        if (m_timer > 3000) {
            m_stateMachine.dispatchEvent(Event::TIMEOUT);
        }
        break;
    case State::WAKING:
        m_timer += delta_time;
        if (m_timer > 5000) {
            m_stateMachine.dispatchEvent(Event::TIMEOUT);
        }
        break;
    case State::SHELL_SLIDING:
        checkCollideOtherCharasters();
        if (m_collision_tag & ECollisionTag::X_AXIS) {
            getParent()->castTo<MarioGameScene>()->playSoundAtPoint("bump", getBounds().center());
        }
        break;
    case State::DEAD:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    }
}

void Koopa::takeDamage(DamageType damageType, Character* attacker) {
    if (!isAlive()) {
        return;
    }

    if (damageType == DamageType::HIT_FROM_ABOVE) {
        m_stateMachine.dispatchEvent(Event::STOMPED);
        MARIO_GAME.playSound("stomp");

    } else {
        m_stateMachine.dispatchEvent(Event::PROJECTILE_HIT);
    }
}

void Koopa::touch(Character* character) {
    auto state = m_stateMachine.getState();

    bool isKickable = (state == State::HIDDEN)        || (state == State::WAKING);
    bool isDanger   = (state == State::SHELL_SLIDING) || (state == State::WALKING);

    if (isKickable) {
        m_stateMachine.dispatchEvent(Event::KICKED);
        addScoreToPlayer(400);
        MARIO_GAME.playSound("kick");
    }
    else if (isDanger) {
        character->takeDamage(DamageType::KICK, this);
    }
}

void Koopa::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

bool Koopa::isAlive() const {
    return (m_stateMachine.getState() != State::DEAD);
}

void Koopa::onStarted() {
    Enemy::onStarted();

    switch(getProperty("Flying").asInt()) {
    case 1:
        m_initial_state = State::JUMPING;
        break;
    case 2:
        m_initial_state = State::LEVITATING;
        break;
    default:
        m_initial_state = State::WALKING;
        break;
    }
}
