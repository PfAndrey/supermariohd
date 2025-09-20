#include "SuperMarioGame.hpp"
#include "Goomba.hpp"

Goomba::Goomba() {
    setSize({ 32,32 });
    const sf::Texture& texture = *MARIO_GAME.textureManager().get("Enemies");
    m_animator.create("walk",    texture, { {{0,0}, {32,32}},{{32,0}, {32,32}} }, 0.005f);
    m_animator.create("squashed", texture, { 64, 0, 32, 32 });
    m_animator.create("fall",    texture, { 0, 32, 32, -32 });
    m_animator.setSpriteOffset("squashed", 0, { 0,8 });

    m_stateMachine.attachOnEnterMap<>(&m_animator, &Animator::play,
        {{ State::WALKING , "walk"    },
         { State::SQUASHED, "squashed"},
         { State::DIED    , "fall"    }
        });

    m_stateMachine.addTransition(Event::ENTERED_VIEW  , State::DEACTIVATED, State::WALKING, [this]() { enterWalking(); });
    m_stateMachine.addTransition(Event::PROJECTILE_HIT, State::WALKING,     State::DIED,    [this]() { enterDead();    });
    m_stateMachine.addTransition(Event::STOMPED       , State::WALKING,     State::SQUASHED,[this]() { enterSquashed();});
    m_stateMachine.start(State::DEACTIVATED);
}

void Goomba::update(int delta_time) {
    Enemy::update(delta_time);

    switch (m_stateMachine.getState()) {
    case State::DEACTIVATED:
        if (isInCamera()) {
            m_stateMachine.dispatchEvent(Event::ENTERED_VIEW);
        }
        break;
    case State::WALKING:
        updatePhysics(delta_time, GRAVITY_FORCE);
        updateCollision(delta_time, LogicFlags::ON_X_BOUND);
        m_animator.update(delta_time);
        break;
    case State::DIED:
        updatePhysics(delta_time, GRAVITY_FORCE);
        break;
    case State::SQUASHED:
        m_timer += delta_time;
        if (m_timer > 3000) {
            removeLater();
        }
        break;
    }
}

void Goomba::draw(sf::RenderWindow* render_window) {
    m_animator.setPosition(getPosition());
    m_animator.draw(render_window);
}

void Goomba::takeDamage(DamageType damageType, Character*) {
    if (damageType == DamageType::HIT_FROM_ABOVE) {
        m_stateMachine.dispatchEvent(Event::STOMPED);
    } else {
        m_stateMachine.dispatchEvent(Event::PROJECTILE_HIT);
    }
}

void Goomba::touch(Character* character) {
    character->takeDamage(DamageType::KICK, this);
}

bool Goomba::isAlive() const {
    return (m_stateMachine.getState() == State::WALKING);
}

void Goomba::enterWalking() {
    m_velocity.x = RUN_SPEED;
}

void Goomba::enterSquashed() {
    m_velocity.x = 0;
    addScoreToPlayer(100);
    MARIO_GAME.playSound("stomp");
}

void Goomba::enterDead() {
    m_velocity.x = 0;
    m_velocity += 0.4f * Vector::UP;
    addScoreToPlayer(100);
    MARIO_GAME.playSound("kick");
}