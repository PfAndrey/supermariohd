#ifndef ENEMIES_HPP
#define ENEMIES_HPP

#include "Character.hpp"

class Mario;
class Blocks;

class Enemy : public Character {
    DECLARE_TYPE_INFO(Character)

protected:
    void update(int delta_time);
    void onStarted() override;
    Mario* mario();
    void addScoreToPlayer(int score);
    void checkNextTileUnderFoots();
    void checkCollideOtherCharasters();
    void checkFallUndergound();
    void updateCollision(float delta_time);
    void updatePhysics(float delta_time, float gravity);
    bool isInCamera() const;
    void playAnimation(const std::string& name);

    static constexpr float GRAVITY_FORCE = 0.0015f;
    static constexpr float RUN_SPEED = -0.05f;
    Vector m_speed;
    Vector m_direction = Vector::LEFT;
    Blocks* m_blocks = nullptr;
    Animator m_animator;
    ECollisionTag m_collision_tag;

private:
    Mario* m_mario = nullptr;
};

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
 
    enum class State : uint8_t {
        DEACTIVATED = 0,
        NORMAL      = 1,
        CRAMPED     = 2,
        DIED        = 3
    };

    void setState(State state);

    State m_state = State::DEACTIVATED;
    float m_timer = 0;
};

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

    enum class State : uint8_t {
        DEACTIVATED = 0,
        NORMAL      = 1,
        JUMPING     = 2,
        LEVITATING  = 3,
        HIDDEN      = 4,
        CLIMB       = 5,
        BULLET      = 6,
        DIED        = 7
    };

    void setState(State state);
    void onStarted() override;

    const Vector full_size = { 32,48 };
    const Vector hidden_size = { 32,32 };
    State m_state = State::DEACTIVATED;
    State m_initial_state = State::NORMAL;
    float m_timer = 0;
    Vector m_initial_pos;
};

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
        NORMAL      = 1,
        HIDDEN      = 2,
        BULLET      = 3,
        DIED        = 4
    };
 
    void setState(State state);
    State m_state = State::DEACTIVATED;
    bool m_is_flying = false;
    float m_timer = 0;
};

class Hammer : public GameObject {
public:
    Hammer(Mario* target);
    void update(int delta_time) override;
    void throwAway(const Vector& speed);
    void draw(sf::RenderWindow* render_window);

private:

    enum class State : uint8_t {
        IN_HAND = 0,
        FLY     = 1,
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
        DIED   = 1
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

class Spinny : public Enemy {
public:
    Spinny(const Vector& position, const Vector& speed, const Vector& walk_direction);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum State : uint8_t {
        EGG     = 0,
        NORMAL  = 1,
        DIED    = 2
    };

    void setState(State state);

    State m_state = State::EGG;
    Vector m_walk_direction;
};

class Lakity : public Enemy {
    DECLARE_TYPE_INFO(Enemy)

public:
    Lakity();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void runAway(const Vector& run_direction);
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:
    enum class State : uint8_t {
        NORMAL   = 0,
        DIED     = 1,
        RUN_AWAY = 2
    };

    void setState(State state);

    static constexpr int FIRE_RATE = 2500;
    State m_state = State::NORMAL;
    float m_fire_timer = 0;
    float m_died_timer = 0;
};

class LakitySpawner : public GameObject {
public:
    LakitySpawner() = default;
    void update(int delta_time) override;

private:
    void onStarted() override;

    static constexpr int CHECK_INTERVAL = 5000;
    Lakity* m_lakity = nullptr;
    Mario* m_mario = nullptr;
    float m_lakity_checker_timer = 0;
};

class CheepCheep : public Enemy {
public:
    CheepCheep();
    CheepCheep(const Vector& initial_pos, const Vector& initial_speed);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        NORMAL      = 0,
        UNDERWATER  = 1,
        DIED        = 2
    };
 
    void setState(State state);
    State m_state = State::NORMAL;
};

class CheepCheepSpawner : public GameObject {
public:
    CheepCheepSpawner() = default;
    void update(int delta_time) override;

private:
    void onStarted() override;

    static constexpr int SPAWN_INTERVAL = 1000;
    const Vector spawn_speed = { 0.32f,-0.65f };
    float m_spawn_timer = 0;
    int m_map_height;
    Mario* m_mario = nullptr;
};

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
        ZIG  = 0,
        ZAG  = 1,
        DIED = 2
    };

    void enterState(State state);

    State m_state = State::ZIG;
    int m_delay_time = 0;
    Vector m_speed;
};

class BulletBill : public Enemy {
public:
    BulletBill(const Vector& infitial_pos, const Vector& initial_speed);
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:

    enum class State : uint8_t {
        NORMAL  = 0,
        DIED    = 1
    };

    void setState(State state);
    State m_state = State::NORMAL;
};

class BulletBillSpawner : public GameObject {
public:
    BulletBillSpawner();
    void update(int delta_time) override;
    void onStarted() override;

private:
    bool isBulletBillBeyondTiledMap() const;

    Mario* m_mario = nullptr;
    int m_blocks_width = 0;
    static constexpr int SPAWN_INTERVAL = 4000;
    static constexpr float BULLET_SPEED = 0.15f;
    float m_spawn_timer = 0;
};

class PiranhaPlant : public Enemy {
    DECLARE_TYPE_INFO(Enemy)

public:
    PiranhaPlant();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;
    void hideInTube();
    //! ICharacter impl:
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;
    bool isAlive() const override;

private:
    static constexpr float PERIOD_MS = 2000;
    const Vector SIZE = {32, 46};
    float m_timer = 0;
    float m_buttom = 0; // <! Initial buttom line
    bool m_dead_zone = false;
};

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
    Vector m_speed;
};

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
        WALK        = 0,
        TURN        = 1,
        JUMP        = 2,
        PRE_JUMP    = 3,
        MIDDLE_FIRE = 4,
        LAND_FIRE   = 5,
        NO_BRIDGE   = 6,
        FALL        = 7,
        DIED        = 8
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

#endif // ENEMIES_HPP