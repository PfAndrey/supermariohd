#ifndef SUPER_MARIO_GAME_HPP
#define SUPER_MARIO_GAME_HPP

#include "GameEngine.hpp"
#include "Mario.hpp"

#define MARIO_GAME (*MarioGame::instance())

const std::string MARIO_RES_PATH = "res/";

class MarioGameScene;
class MarioGUI;

class MarioGame : public Game {
public:
    ~MarioGame() = default;
    static MarioGame* instance();
    void showStatus();
    void addScore(int value, const Vector& vector = Vector::ZERO);
    void addCoin();
    void addLive();
    void setScore(int score);
    int getScore() const;
    int getGameTime() const;
    void setEndLevelStatus();
    void invincibleMode(bool value = true);
    void loadLevel(const std::string& level_name);
    void loadSubLevel(const std::string& sublevel_name);
    void unloadSubLevel();
    Label* createText(const std::string& text, const Vector& pos);
    void marioDied();
    GameObject* currentScene() const;
    Mario* getPlayer(int index = 0) const;
    void playSound(const std::string& name, const Vector& pos);
    void playSound(const std::string& name);

    template <typename T, typename ...A>
    T* spawnObject(A&& ...args) {
        T* object = new T(std::forward<A>(args)...);
        m_current_scene->addChild(object);
        return object;
    }

    TimerManager& globalTimer();

private:

    enum class GameState {
        MAIN_MENU   = 0,
        STATUS      = 1,
        PLAYING     = 2,
        LEVEL_OVER  = 3,
        GAME_OVER   = 4,
        TIME_OUT    = 5
    };

    enum class TimeOutState {
        NONE          = 0,
        START_WARNING = 1,
        WARNING       = 2,
    };

    MarioGame();
    void syncMarioRank(GameObject* from_scene, GameObject* to_scene);
    void updateGUI();
    void clearScenes();
    virtual void init() override;
    void update(int delta_time) override;
    MarioGUI* GUI();
    void setScene(GameObject* game_object);
    void pushScene(GameObject* game_object);
    void popScene();
    void reset();
    void setState(GameState state);
    void updateMusic();
    std::string nextLevelName() const;

    const std::string FIRST_STAGE_NAME = "WORLD 3-1";
    GameState m_game_state = GameState::MAIN_MENU;
    TimeOutState m_time_out_state = TimeOutState::NONE;
    TimerManager m_timer;
    std::vector<GameObject*> m_scene_stack;
    std::string m_level_name;
    std::string m_current_stage_name;
    bool m_invincible_mode = false;
    int m_delay_timer = 0;
    int m_game_time = 300000;
    int m_lives = 3;
    int m_score = 0;
    int m_coins = 0;
    MarioGUI* m_gui_object = nullptr;
    GameObject* m_current_scene = nullptr;
};

class Blocks;
class Mario;

class MarioGameScene : public GameObject {
    DECLARE_TYPE_INFO(GameObject)

public:
    MarioGameScene(const std::string& filepath);
    ~MarioGameScene();
    void loadFromFile(const std::string& filepath);
    Rect cameraRect() const;
    void setCameraOnTarget();
    Vector pointToScreen(const Vector& vector);
    Vector screenToPoint(const Vector& vector);
    void playSoundAtPoint(const std::string& name, const Vector& pos);
    const std::string& getLevelName() const;

private:
    MarioGameScene();
    void init();
    void update(int delta_time) override;
    void draw(sf::RenderWindow* render_window) override;
    void events(const sf::Event& event) override;

    sf::View m_view;
    static constexpr float SCALE_FACTOR = 1.5f;
    Vector screen_size = { 1280 / SCALE_FACTOR, 720 / SCALE_FACTOR };
    std::string m_level_name;
    Mario* m_mario = nullptr;
    Blocks* m_blocks = nullptr;
    Rect m_camera_rect;
};

enum class GUIState : uint8_t {
    NORMAL    = 0,
    STATUS    = 1,
    MENU      = 2,
    GAME_OVER = 3
};

class MarioGUI : public GameObject {
public:
    MarioGUI();
    void setScore(int value);
    void setCoins(int value);
    void setGameTime(int time);
    void setLevelName(const std::string& string);
    void setLives(int value);
    void setMarioRank(MarioRank rank);
    void setState(GUIState state);
    FlowText* createFlowText();
    Label* createLabel();
    void update(int delta_time) override;
    void pause(bool ispaused);

private:
    void draw(sf::RenderWindow* render_window) override;

    GUIState m_state;
    int m_tmr = 0;
    Animator* m_mario_pix = nullptr;
    Animator* m_coin = nullptr;
    Pallete m_fire_pallete;
    Label* m_score_lab = nullptr;
    Label* m_coin_counter_lab = nullptr;
    Label* m_world_lab = nullptr;
    Label* m_timer = nullptr;
    Label* m_level_name = nullptr;
    Label* m_lives = nullptr;
    Label* m_game_logo = nullptr;
    Label* m_one_player_lab = nullptr;
    Label* m_two_player_lab = nullptr;
    Label* m_menu_selector = nullptr;
    Label* m_game_over_lab = nullptr;
    Label* m_paused_label = nullptr;
    FlowText* m_flow_text = nullptr;
};

#endif // !SUPER_MARIO_GAME_HPP
