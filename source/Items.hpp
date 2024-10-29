#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <array>
#include <functional>

#include "GameEngine.hpp"

class Mario;

class Item : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    virtual void collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) = 0;
};

class Jumper : public Item {
public:
    Jumper();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) override;

private:
    void onStarted() override;

    Animator m_animator;
    Mario* m_mario = nullptr;
    float m_bottom;
    int m_state = 4;
    float m_timer = 0;
};

class Platform : public Item {
    DECLARE_TYPE_INFO(Item)

 public:
    virtual Vector getSpeedVector() = 0;
    void collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) override;
};

enum class PlatformType : uint8_t {
    VERTICAL    = 0,
    HORIZONTAL  = 1,
    ELEVATOR    = 2,
    SKATE       = 3,
    UNKNOWN     = 100
};

class MoveablePlatform : public Platform {
public:
    MoveablePlatform();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    Vector getSpeedVector() override;
    void collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) override;

private:
    void onStarted() override;

    static constexpr float SKATE_SPEED = 0.17f;
    PlatformType m_platform_type = PlatformType::UNKNOWN;
    Vector m_center;
    int m_size;
    int m_amplitude;
    float m_period_time = 0;
    int m_bottom = 0;
    float m_timer = 0;
    sf::Sprite m_sprite;
    Vector m_speed;
    float m_last_delta;
    float m_acceleration = 0;
    Vector m_orientation;
    Mario* m_mario = nullptr;
};

class FallingPlatform : public Platform {
public:
    FallingPlatform() = default;
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    Vector getSpeedVector() override;
    void addImpulse(const Vector& speed);
    void setSpeed(const Vector& speed);
    void setMovingCallback(const std::function<void()>& func);
    void collsionResponse(Mario* mario, ECollisionTag& collision_tag, int delta_time) override;

private:
    void onStarted() override;

    std::function<void()> m_moving_callback;
    sf::Sprite m_sprite;
    Vector m_speed;
    bool m_stayed = false;
    Mario* m_mario = nullptr;
};

class PlatformSystem : public GameObject {
 public:
    PlatformSystem();
    void onStarted() override;
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

 private:
    void onLeftPlatformMove();
    void onRightPlatformMove();
    SpriteSheet m_sprite_sheet;
    FallingPlatform* m_left_platform = nullptr;
    FallingPlatform* m_right_platform = nullptr;
};

class Mario;

class Ladder : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

 public:
    Ladder();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;
    void onStarted() override;

private:
    sf::Sprite m_sprite;
    float m_height, m_width, m_bottom;
    float m_timer = 0;
};

class FireBar : public GameObject {
public:
    FireBar();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

protected:
    void onStarted() override;

private:
    Animator m_animator;
    Mario* m_mario = nullptr;
    sf::CircleShape m_shape;
    std::vector<Vector> m_fire_pos;
    float m_timer  = 0;
    float m_speed = -1;
};

class LevelPortal : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
     LevelPortal();

private:

    enum class PortalType {
        ENTER_LEVEL      = 0,   //!< Enter to the next level.
        ENTER_SUBLEVEL   = 1,   //!< Enter to the sublevel. Main level is paused.
        LEAVE_SUBLEVEL   = 2,   //!< Leave the sublevel. Main level is resumed.
    };

    enum class State {
        IDLE             = 0,   //!< Portal is not used.
        TRANSITION       = 1,   //!< Transition is in progress.
    };

    void update(int delta_time) override;
    void onStarted() override;
    void goToLevel();
    void goToSublevel();
    void cameBackFromSublevel();

    static constexpr int TRANSITION_TIME = 1500;
    bool m_used = false;
    bool m_show_status = false;
    float m_timer = 0;
    PortalType m_portal_type = PortalType::ENTER_LEVEL;
    State m_state = State::IDLE;
    Vector m_direction;
    std::string m_level_name;
    std::string m_sub_level_name;
    Mario* m_mario = nullptr;
    LevelPortal* m_came_back_portal = nullptr;
};

class EndLevelFlag : public GameObject {
public:
    EndLevelFlag();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    void onStarted() override;

    static constexpr float FALLING_SPEED = 0.25f;
    int m_cell_y = 0;
    Animator m_animator;
    Mario* m_mario = nullptr;
    bool m_touched = false;
};

class Bowser;
class Blocks;

class EndLevelKey : public GameObject {
public:
    EndLevelKey();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    enum class State : uint8_t {
        PLAY                    = 0,
        BRIDGE_HIDING           = 1,
        BOWSER_RUN              = 2,
        BOWSER_FALL             = 3,
        MARIO_GOING_TO_PRINCESS = 4,
        TEXT_SHOW               = 5,
        GO_TO_NEXT_LEVEL        = 6
    };

    void enterState(State state);
    void onStarted() override;

    State m_state = State::PLAY;
    sf::Sprite m_sprite;
    int m_delay_timer = 0;
    std::vector<Vector> m_bridge_blocks;
    Mario* m_mario = nullptr;
    Bowser* m_bowser = nullptr;
    Blocks* m_blocks = nullptr;

};

class CastleFlag : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    CastleFlag();
    void liftUp();
    void draw(sf::RenderWindow* render_window) override;
    void update(int delta_time) override;

private:
    void onStarted() override;

    Animator m_animator;
    int m_pos_y = 0;
};

class Princess : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    Princess();
    void draw(sf::RenderWindow* render_window) override;

private:
    Animator m_animator;
};

class Trigger : public GameObject {
private:
    void update(int delta_time) override;
    void onStarted() override;

private:
    bool m_trigered = false;
    Mario* m_mario = nullptr;
};

#endif // ITEMS_HPP
