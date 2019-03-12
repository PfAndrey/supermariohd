#ifndef  SUPERMARIOGAME_H
#define SUPERMARIOGAME_H

#include "GameEngine.h"
#include "Mario.h"

const std::string  MARIO_RES_PATH = "res/";

class CMario;
class CMarioGameScene;
class CMarioGUI;

class CMarioGame : public CGame
{
private:
	CMarioGame();
	Timer m_timer;
	CMarioGUI* m_gui_object = NULL;
	static CMarioGame* s_instance;
	CGameObject* m_current_scene = NULL;
	std::vector<CGameObject*> m_scene_stack;
	void syncMarioRank(CGameObject* from_scene, CGameObject* to_scene);
	enum class GameState {main_menu, status, playing, level_over, game_over, time_out} m_game_state = GameState::main_menu;
	enum class TimeOutState { none, start_warning, warning } m_time_out_state = TimeOutState::none;
	int m_delay_timer = 0;
	int m_game_time = 300000;
	int m_lives = 3;
	int m_score = 0;
	int m_coins = 0;
	std::string m_level_name;
	bool m_invincible_mode = false;
	void updateGUI();
	void clearScenes();
	virtual void init() override;
	void update(int delta_time) override;
	CMarioGUI* GUI();
	void setScene(CGameObject* game_object);
	void pushScene(CGameObject* game_object);
	void popScene();
	void reset();
	void setState(GameState state);
    const std::string m_first_stage_name = "WORLD 1-1";
	std::string m_current_stage_name;
	void updateMusic();
	std::string nextLevelName() const;
public:
	void showStatus();
	static CMarioGame* instance();
	~CMarioGame();
	Timer& timer();
	void addScore(int value, const Vector& vector = Vector::zero);
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
	CLabel* createText(const std::string& text, const Vector& pos);
	void marioDied();
};  

CMarioGame& MarioGame();

class CBlocks;
class CMoveablePlatform;
class CMario;

class CMarioGameScene : public CGameObject
{
private:
	CMario* m_mario = NULL;
	CBlocks* m_blocks = NULL;
	sf::View m_view;
	const float scale_factor = 1.5f;
	const Vector screen_size = {1280 / scale_factor, 720 /scale_factor };
	std::string m_level_name;
public:
	const std::string& getLevelName() const;
	CMarioGameScene(const std::string& filepath);
	~CMarioGameScene();
	void loadFromFile(const std::string& filepath);
	Rect cameraRect() const;
	void setCameraOnTarget();
	Vector pointToScreen(const Vector& vector);
	Vector screenToPoint(const Vector& vector);
    void playSoundAtPoint(const std::string& name, const Vector& pos);
protected:
    virtual void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void events(const sf::Event& event) override;
private:
	CMarioGameScene();
	void init();
};

enum class GUIState {normal, status, menu, gameover};

class CMarioGUI : public CGameObject
{
public:
	CMarioGUI();
	void setScore(int value);
	void setCoins(int value);
	void setGameTime(int time);
	void setLevelName(const std::string& string);
	void setLives(int value);
	void setMarioRank(MarioRank rank);
	void setState(GUIState state);
	CFlowText* createFlowText();
	CLabel* createLabel();
	void update(int delta_time) override;
	void pause(bool ispaused);
protected:
	void postDraw(sf::RenderWindow* render_window) override;
	void draw(sf::RenderWindow* render_window) override;
	GUIState m_state;
private:
	int m_tmr = 0;
	Animator* m_mario_pix = nullptr, *m_coin = nullptr;
	Pallete m_fire_pallete;
	CLabel *m_score_lab,
		*m_coin_counter_lab,
		*m_world_lab,
		*m_timer,
		*m_level_name,
		*m_lives,
		*m_game_logo,
		*m_one_player_lab,
		*m_two_player_lab,
		*m_menu_selector,		
		*m_game_over_lab,
		*m_paused_label;
	CFlowText* m_flow_text = nullptr;
};

#endif
