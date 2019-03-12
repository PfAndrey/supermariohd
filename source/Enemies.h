#ifndef ENEMIES_H
#define ENEMIES_H

#include "GameEngine.h"

class CMario;
class CBlocks;

class CEnemy : public CGameObject
{
public:
	virtual void kickFromTop(CMario* mario) = 0;
	virtual void kickFromBottom(CMario* mario) = 0;
	virtual bool isAlive() const = 0;
	virtual void touchSide(CMario* mario) = 0;
	virtual bool isInBulletState() const {return false;};
	virtual void fired(CMario* mario);
	void update(int delta_time);
protected:
	virtual void start() override;
	CMario* mario();
	void addScoreToPlayer(int score);
	void checkNextTileUnderFoots();
	void checkCollideOtherCharasters();
	void checkFallUndergound();
	void updateCollision(float delta_time);
	void updatePhysics(float delta_time, float gravity);
	ECollisionTag m_collision_tag;
	const float gravity_force = 0.0015f;
	Vector m_speed;
	Vector m_direction = Vector::left;
	CMario* m_mario = nullptr;
	CBlocks* m_blocks = nullptr;
	const float m_run_speed = -0.05f;
};

class CGoomba : public CEnemy
{
public:
	CGoomba();
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
private:
	enum State { Deactivated, Normal, Cramped, Died } m_state = Deactivated;
	void setState(State state);
	Animator m_animator;
	float m_timer = 0;
};

class CKoopa : public CEnemy
{
public:
	CKoopa();
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual bool isAlive() const override;
	virtual void touchSide(CMario* mario) override;
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual bool isInBulletState() const override;
private:
	const Vector full_size = { 32,48 };
	const Vector hidden_size = { 32,32 };
	void onActivated() override;
	enum State { Deactivated, Normal, Jumping, Levitating, Hidden, Climb, Bullet, Died } m_state = Deactivated;
	void setState(State state);
	Animator m_animator;
	int m_flying_mode = 0;
	float m_timer = 0;
	Vector m_initial_pos;
};

class CBuzzyBeetle : public CEnemy
{
public:
	CBuzzyBeetle();
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void fired(CMario* mario) override;
	virtual bool isAlive() const override;
	virtual void touchSide(CMario* mario) override;
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual bool isInBulletState() const override;
private:
	enum State { Deactivated, Normal, Hidden, Bullet, Died } m_state = Deactivated;
	void setState(State state);
	Animator m_animator;
	bool m_is_flying = false;
	float m_timer = 0;
};

class CHammer : public CGameObject
{
public:
	CHammer(CMario* target);
	virtual void update(int delta_time) override;
	void  throwAway(const Vector& speed);
	void  draw(sf::RenderWindow* render_window);
private:
	enum State { in_hand, fly } m_state = in_hand;
	CGameObject* m_parent;
	Vector m_direction;
	const float gravity_force = 0.0015f;
	CMario* m_target;
	Animator m_animator;
	Vector m_speed;
};

class CHammerBro : public CEnemy
{
public:
	CHammerBro();
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void onActivated() override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
private:
	const int fire_rate = 1200;
	const int jump_rate = 5000;
	const int walk_amlitude = 25;
	bool isCanJumpUp() const;
	bool isCanJumpDown() const;
	bool m_collision_on = true;
	enum State { Normal, Died } m_state = Normal;
	Vector m_jump_direction = Vector::up;
	void setState(State state);
	CHammer* m_hummer = NULL;
	Animator m_animator;
	int m_center_x = 0;
	float m_jump_timer = 0;
	float m_fire_timer = 0;
	float m_drop_off_height = 0;
};

class CSpinny : public CEnemy
{
public:
	CSpinny(const Vector& position, const Vector& speed, const Vector& walk_direction);
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
private:
	enum State { Egg, Normal, Died } m_state = Egg;
	Vector m_walk_direction;
	void setState(State state);
	Animator m_animator;
};

class CLakity : public CEnemy
{
public:
	CLakity();
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
	void runAway(const Vector& run_direction);
	void start() override;
private:
	const int fire_rate = 2500;
	float m_fire_timer = 0;
	float m_died_timer = 0;
	enum State { Normal, Died, RunAway } m_state = Normal;
	void setState(State state);
	Animator m_animator;
};

class CLakitySpawner : public CGameObject
{
public:
	CLakitySpawner();
	void update(int delta_time) override;
protected:
	void onActivated() override;
	void start() override;
private:
	const int check_interval = 5000;
	CLakity* m_lakity = NULL;
	CMario* m_mario = NULL;
	float m_lakity_checker_timer = 0;
};

class CCheepCheep : public CEnemy
{
public:
	CCheepCheep();
	CCheepCheep(const Vector& initial_pos, const Vector& initial_speed);
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
private:
	enum State { Normal, Underwater, Died } m_state = Normal;
	void setState(State state);
	Animator m_animator;
};

class CCheepCheepSpawner : public CGameObject
{
public:
	CCheepCheepSpawner();
	void update(int delta_time) override;
	void onActivated() override;
	void start() override;
private:
	const int spawn_interval = 1000;
	const Vector spawn_speed = { 0.32f,-0.65f };
	float m_spawn_timer = 0;
	int m_map_height;
	CMario* m_mario = NULL;
};

class CBlooper : public CEnemy
{
public:
	CBlooper();
	void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
private:
	enum State { Zig, Zag, Died } m_state = State::Zig;
	void enterState(State state);
	int m_delay_time = 0;
	Vector m_speed;
	Animator m_animator;
};

class CBulletBill : public CEnemy
{
public:
	CBulletBill(const Vector& initial_pos, const Vector& initial_speed);
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
	virtual void start() override;
private:
	enum State { Normal, Died } m_state = Normal;
	void setState(State state);
	Animator m_animator;
};

class CBulletBillSpawner : public CGameObject
{
public:
	CBulletBillSpawner();
	void update(int delta_time) override;
	void onActivated() override;
	void start() override;
private:
	bool isBulletBillBeyondTiledMap() const;
	CMario* m_mario = NULL;
	int m_blocks_width = 0;
	const int spawn_interval = 4000;
	const float bullet_speed = 0.15f;
	float m_spawn_timer = 0;
};

class CPiranhaPlant : public CEnemy
{
public:
	CPiranhaPlant();
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual void fired(CMario* mario) override;
	virtual bool isAlive() const override;
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void onActivated() override;
	void hideInTube();
private:
	const Vector size = { 32,46 };
	const float period_time = 2000;
	sf::Sprite m_sprite;
	float m_timer = 0;
	float m_buttom;
	bool m_dead_zone = false;
};

class CPodoboo : public CEnemy
{
public:
	CPodoboo();
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual void fired(CMario* mario) override;
	virtual bool isAlive() const override;
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void onActivated() override;
private:
	float m_timer = 0;
	const float period_time = 3000.f;
	const float amplitude = 400.f;
	float m_acceleration = 0;
	float m_max_speed;
	Vector m_center;
	Vector m_speed;
	const Vector size = { 32,32 };
	Animator m_animator;
	sf::CircleShape m_shape;
};

class CFireball : public CGameObject
{
public:
	CFireball(const Vector& Position, const Vector& SpeedVector);
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void start() override;
private:
	int m_life_timer = 10000;
	CMario* m_mario = NULL;
	Vector m_speed;
	Animator m_animator;
};

class CBowser : public CEnemy
{
public:
	CBowser();
	virtual void draw(sf::RenderWindow* render_window) override;
	virtual void update(int delta_time) override;
	virtual void onActivated() override;
	virtual void kickFromTop(CMario* mario) override;
	virtual void kickFromBottom(CMario* mario) override;
	virtual void touchSide(CMario* mario) override;
	virtual bool isAlive() const override;
	const int c_walk_amlitude = 100;
	const int c_run_speed = 100;
	const int c_jump_period = 4000;
	void noBridge();
private:
	const float anim_speed = 0.008f;
	enum class State { walk, turn, jump, pre_jump, middle_fire, land_fire, no_bridge, fall, died } m_state = State::walk;
	void enterState(State state);
	int m_lives = 5;
	int m_delay_timer = 0;
	Animator m_animator;
	float m_center_x;
	Vector m_old_speed;
};

#endif
