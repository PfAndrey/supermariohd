#pragma once

#include "Enemy.hpp"

class Hammer : public GameObject {
public:
    Hammer(Mario* target);
    void update(int delta_time) override;
    void throwAway(const Vector& speed);
    void draw(sf::RenderWindow* render_window);

private:

    enum class State : uint8_t {
        IN_HAND = 0,
        FLY = 1,
    };

    static constexpr float GRAVITY_FORCE = 0.0015f;
    State m_state = State::IN_HAND;
    GameObject* m_parent = nullptr;
    Vector m_direction;
    Mario* m_target = nullptr;
    Vector m_speed;
    Animator m_animator;
};

class HammerBro : public Enemy {
public:
    HammerBro();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        NORMAL = 0,
        DIED = 1
    };

    void setState(State state);
    bool isCanJumpUp() const;
    bool isCanJumpDown() const;

    static constexpr int FIRE_RATE = 1200;
    static constexpr int JUMP_RATE = 5000;
    static constexpr int WALK_AMPLITUDE = 25;
    State m_state = State::NORMAL;
    bool m_collision_on = true;
    Vector m_jump_direction = Vector::UP;
    Hammer* m_hummer = nullptr;
    int m_center_x = 0;
    float m_jump_timer = 0;
    float m_fire_timer = 0;
    float m_drop_off_height = 0;
};
