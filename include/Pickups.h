#ifndef PICKUPS_H
#define PICKUPS_H

#include "GameEngine.h"

class CMario;
class CBlocks;

class CCoin : public CGameObject
{
public:
	CCoin();
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void kick();
protected:
	void onActivated() override;
	void start() override;
private:
	enum class State { twist, shane } m_state = State::twist;
	int m_remove_timer = 1000;
	Animator m_animator;
	CMario* m_mario = 0;
};

class CMushroom : public CGameObject
{
public:
	CMushroom(const Vector& pos, CMario* mario);
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
	void addImpulse(const Vector& speed);
protected:
	void start() override;
	virtual void action();
	sf::Sprite m_sprite;
	bool m_as_flower = false;
private:
	CMario * m_mario;
	CBlocks* m_blocks;
	Vector m_speed;
	enum State { wait, borning, normal } m_state = State::wait;
	float m_timer = 0;
	float run_speed = 0.1f;
	const float gravity_force = 0.0015f;
};

class COneUpMushroom : public CMushroom
{
public:
	COneUpMushroom(const Vector& pos, CMario* mario);
private:
	virtual void action() override;
};

class CStar : public CGameObject
{
private:
	enum State { wait, borning, normal } m_state = State::wait;
	sf::Sprite m_sprite;
	float m_timer = 0;
	CMario* m_mario = nullptr;
	Vector m_speed;
	const float gravity_force = 0.0015f;
	const float jump_power = 0.6f;
	const float horizontal_speed = 0.1f;
	void start() override;
	CBlocks* m_blocks;
public:
	CStar(const Vector& pos, CMario* mario);
	void draw(sf::RenderWindow* render_window) override;
	void update(int delta_time) override;
};

#endif
