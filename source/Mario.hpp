#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "GameEngine.hpp"

class Blocks;
class Ladder;

class MarioBullet : public GameObject {
public:
    MarioBullet(const Vector& pos, const Vector& direction);

private:

    enum class State : uint8_t {
        FLY    = 0,
        SPLASH = 1
    };

    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void setState(State state);
    void onStarted() override;

    const float SPEED = 0.33f;
    const float GRAVITY_FORCE = 0.0015f;
    State m_state = State::FLY;
    Vector m_direction;
    float m_timer = 0;
    Vector m_speed;
    Blocks* m_blocks = nullptr;
    Animator m_animator;
};

enum class MarioRank : uint8_t {
    SMALL = 0,
    BIG   = 1,
    FIRE  = 2
};

class IMarioState;

class Mario : public Character {
    DECLARE_TYPE_INFO(Character)

public:
    Mario();
    MarioRank getRank() const;
    bool isSmall() const;
    void promote();
    const Vector& getSpeed() const;
    void setSpeed(const Vector& speed);
    void addImpulse(const Vector& speed);
    void kill();
    const Vector& getInputDirection() const;
    bool isGrounded() const;
    bool isClimbing() const;
    void setUnclimb();
    void setState(IMarioState* state);
    void setInvincibleMode(bool value = true);
    void syncState(Mario* otherMario);

    bool isAlive() const override;
    void takeDamage(DamageType damageType, Character* attacker) override;
    void touch(Character* character) override;

private:

    enum class State : uint8_t {
        NORMAL    = 0,
        PROMOTING = 1,
        DEMOTING  = 2,
        DIED      = 3,
    };

    enum class EnvState : uint8_t {
        NORMAL = 0,
        WATER  = 1,
        LADDER = 2
    };

    enum AnimType : uint8_t {
        IDLE      = 0,
        WALK      = 1,
        SWIM      = 2,
        JUMP      = 3,
        SLIP      = 4,
        SEAT      = 5,
        CLIMB     = 6,
        GROWING   = 7,
        DEMOTING  = 8,
        FIRING    = 9,
        SHOOT     = 10,
        DIED      = 11
    };

    enum PalleteType : uint8_t {
        NORMAL = 0,
        BLACK  = 1,
        FIRE   = 2
    };

    void standUp();
    void seatDown();
    bool isSeated() const;
    void setRank(MarioRank rank);
    void setState(State state);
    void fire();
    bool canFire() const;
    void jump();
    bool canJump() const;
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void inputProcessing(float delta_time);
    void physicProcessing(float delta_time);
    void collisionProcessing(float delta_time);
    void kickBlocksProcessing();
    void kickEnemiesProcessing(float delta_time);
    void animationProcessing(float delta_time);
    void playAnimation(AnimType animType, float speed = -1.f);
    void setPallete(PalleteType pallete);
    void onStarted() override;
    bool isInWater() const;

    MarioRank m_rank = MarioRank::SMALL;
    State m_state = State::NORMAL;
    EnvState m_env_state = EnvState::NORMAL;
    IMarioState* m_current_state = nullptr;
    ECollisionTag m_collision_tag;
    bool m_invincible_mode = false;
    float m_x_max_speed = 0.f;
    bool m_seated = false;
    float m_jump_timer = 0.f;
    float m_fire_timer = 0.f;
    float m_spawn_timer = 0.f;
    int m_invincible_timer = 0;
    Vector m_direction = Vector::RIGHT;
    Vector m_input_direction = Vector::ZERO;
    Vector m_speed = Vector::ZERO;
    Pallete m_fire_pallete;
    Pallete m_black_pallete;
    Ladder* m_used_ladder = nullptr;
    Blocks* m_blocks = nullptr;
    Animator* m_animator = nullptr;
    friend class IMarioState;
    friend class PromotingMarioState;
    friend class DemotingMarioState;
    friend class NormalMarioState;
    friend class TransitionMarioState;
    friend class GoToCastleMarioState;
    friend class GoToPortalState;
    friend class GoToPrincessState;
};

class IMarioState {
public:
    ~IMarioState() = default;
    virtual void onEnter() {};
    virtual void onLeave() {};
    virtual void update(int delta_time) {};
    void setMario(Mario* mario);

protected:
    using State = Mario::State;
    using AnimType = Mario::AnimType;
    using PalleteType = Mario::PalleteType;

    Mario* getMario();
    GameObject* getScene();
    void setMarioSpawnProtection();
    void playAnimation(Mario::AnimType animType, float speed = -1.f);
    void setPallete(Mario::PalleteType pallete);
    void setMarioRank(MarioRank rank);
    void setMarioState(State state);
    void setMarioSpeed(const Vector& speed);
    void enableScene(bool value);
    void marioPhysicProcessing(float delta_time);

private:
    Mario* m_mario = nullptr;
};

class NormalMarioState : public IMarioState {
public:
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;
};

class DiedMarioState : public IMarioState {
public:
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    float m_delay_timer = 3000;
};

class PromotingMarioState : public IMarioState {
public:
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    float m_promoting_timer = 0;
};

class DemotingMarioState : public IMarioState {
public:
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    float m_promoting_timer = 0;
};

class TransitionMarioState : public IMarioState {
public:
    TransitionMarioState(const Vector& speed, int time);
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    Vector m_speed;
    int m_timer = 0;
    int m_time;
};

class GoToCastleMarioState : public IMarioState {
public:
    GoToCastleMarioState();
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    enum class State {
        START      = 0,
        GO_DOWN    = 1,
        OVERTURN   = 2,
        WALK       = 3,
        WAIT       = 4,
        NEXT_LEVEL = 5
    };

    State m_state = State::START;
    Vector m_speed;
    int m_timer = 0;
    int m_cell_y;
    int m_delay_timer = 0;
    std::string m_next_level;
    std::string m_next_sub_level;
};

class GoToPortalState : public IMarioState {
public:
    GoToPortalState();
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;
};

class Princess;

class GoToPrincessState : public IMarioState {
public:
    GoToPrincessState();
    void onEnter() override;
    void onLeave() override;
    void update(int delta_time) override;

private:
    Princess* m_princess = nullptr;
};

#endif // !MARIO_HPP
