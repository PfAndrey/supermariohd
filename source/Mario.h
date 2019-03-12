#ifndef MARIO_H
#define MARIO_H

#include "GameEngine.h"

class CBlocks;

class CMarioBullet : public CGameObject
{
public:
	CMarioBullet(const Vector& pos, const Vector& direction);
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
private:
	enum class State { fly, splash } m_state = State::fly;
	void setState(State state);
	void start() override;
	const float bullet_speed = 0.33f;
	const float gravity_force = 0.0015f;
	Vector m_direction;
	float m_timer = 0;    
	Vector m_speed;
	CBlocks* m_blocks = NULL;
	Animator m_animator;
};
 
enum class MarioState { normal, promoting, demoting, died};

class CMario;
enum class MarioRank;

class IMarioState
{
 public:
	 ~IMarioState() = default;
	 virtual void onEnter() {};
	 virtual void onLeave() {};
	 virtual void update(int delta_time) {};
	 void setMario(CMario* mario);
protected:
	 CMario* mario();
	 void setMarioSpawnProtection();
	 void playAnimation(const std::string& anim_name);
	 void setMarioRank(const MarioRank& rank);
	 void setMarioState(const MarioState& state);
	 void setMarioNormalPallete();
	 void setMarioFirePallete();
	 void enableScene(bool value);
private:
	CMario* m_mario;
};

class CNormalMarioState : public IMarioState
{
public:
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;
};

class CDiedMarioState : public IMarioState
{
 public:
	 void onEnter() override;
	 void onLeave() override;
	 void update(int delta_time) override;
private:
	float m_delay_timer = 3000;
};

class CPromotingMarioState : public IMarioState
{
public:
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;

private:
	float m_promoting_timer = 0;
};

class CDemotingMarioState : public IMarioState
{
public:
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;

private:
	float m_promoting_timer = 0;
};

class CTransitionMarioState : public IMarioState
{
public:
	CTransitionMarioState(const Vector&  speed, int time);
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override; 
private:
	Vector m_speed;
	int m_timer = 0;
	int m_time;
};

class CGoToCastleMarioState : public IMarioState
{
private:
	enum class State {start,go_down, overturn, walk, wait, next_level} m_state = State::start;
	Vector m_speed;
	int m_timer = 0;
	int m_cell_y;
	int m_delay_timer = 0;
	std::string m_next_level;
	std::string m_next_sub_level;
public:
	CGoToCastleMarioState();
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;
};

class CGoToPortalState : public IMarioState
{
public:
	CGoToPortalState();
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;
private:

};

class CPrincess;
class CGoToPrincessState : public IMarioState
{
public:
	CGoToPrincessState();
	void onEnter() override;
	void onLeave() override;
	void update(int delta_time) override;
private:
	CPrincess* m_princess = NULL;
};

class CLadder;

enum class MarioRank { small, big, fire };

class CMario : public CGameObject
{
public:
	CMario();
	MarioRank getRank() const;
	void setRank(MarioRank rank);
	void promote();
	Vector getSpeed() const;
	void setSpeed(const Vector& speed);
	void addImpulse(Vector _speed);
	void reciveDamage();
	Vector getInputDirection();
	bool isGrounded() const;
	bool isClimbing() const;
	void setUnclimb();
	bool isSmall() const;
	void setState(MarioState state);
	void setState(IMarioState* state);
	void setInvincibleMode(bool value = true);
	void standUp();
	void seatDown();
	bool isSeated() const;
private:
	void fire();
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void setGrounded(bool val = true);
	void inputProcessing(float delta_time);
	void physicProcessing(float delta_time);
	void collisionProcessing(float delta_time);
	void kickBlocksProcessing();
	void kickEnemiesProcessing(float delta_time);
	void animationProcessing(float delta_time);
	void playAnimation(const std::string& name);
	void onActivated() override;
	void start() override;
	const float jump_force = 0.3f;
	const float jump_time = 300.f;
	const float walk_speed = 0.2f;
	const float run_speed = 0.3f;
    const float acceleration = 0.001f;
	const float friction_force = 0.0005f;
	const float gravity_force = 0.0015f;
	const float climb_speed = 0.1f;
	const int invincible_time = 11000;
	const int fire_rate = 400; //bullet per ms
	const float jump_rate = 250.f; //jump per ms
	const Vector small_mario_size = {31,32};
	const Vector big_mario_size = {31,64};
	const Vector seated_mario_size = {31,32};
	float m_jumping_timer = 0;
	float m_jump_timer = jump_rate;
    MarioRank m_rank = MarioRank::small;
	MarioState m_state = MarioState::normal;
	IMarioState* m_current_state = NULL;
	ECollisionTag m_collision_tag;
	bool m_invincible_mode = false;
	int m_invincible_timer = invincible_time;
	Vector m_speed;
	float m_x_max_speed;
	bool m_grounded = false;
	bool m_jumped = false;
	bool m_seated = false;
	bool m_climb = false;
	bool m_in_water = false;
	CLadder* m_used_ladder = NULL;
	CBlocks* m_blocks;
	Animator* m_animator;
	float m_fire_timer = 0;
	float m_spawn_timer = 0;
	Vector m_direction = Vector::right;
	Vector m_input_direcition;
	friend class IMarioState;
	friend class CPromotingMarioState;
	friend class CDemotingMarioState;
	friend class CNormalMarioState;
	friend class CTransitionMarioState;
	friend class CGoToCastleMarioState;
	friend class CGoToPortalState;
	friend class CGoToPrincessState;
	friend class CDiedMarioState;
	Pallete m_fire_pallete, m_black_pallete;
};

#endif




