#include "Pickups.h"
#include "SuperMarioGame.h"
#include "Blocks.h"
#include "Mario.h"

CCoin::CCoin()
{
	m_animator.create("twist", *MarioGame().textureManager().get("Items"), Vector(0, 84), Vector(32, 32), 4, 1, 0.01f);
	m_animator.create("shine", *MarioGame().textureManager().get("Items"), Vector(0, 116), Vector(40, 32), 5, 1, 0.01f, AnimType::forward);
	m_animator.get("shine")->setOrigin(Vector(4, 0));
	static float rot_offset = 0;
	rot_offset += 0.4f;
}

void CCoin::draw(sf::RenderWindow* render_window)
{
	m_animator.setPosition(getPosition());
	m_animator.draw(render_window);
}

void CCoin::update(int delta_time)
{
	m_animator.update(delta_time);
	if (m_state == State::twist)
	{
		if (m_mario->getBounds().isIntersect(getBounds()))
		{
			MarioGame().addScore(100);
			MarioGame().addCoin();
			MarioGame().playSound("coin");
			m_animator.play("shine");
			m_state = State::shane;
		}
	}
	else if (m_state == State::shane)
	{
		m_remove_timer -= delta_time;
		if (m_remove_timer < 0)
			getParent()->removeObject(this);
	}
}

void CCoin::kick()
{
	MarioGame().addScore(100);
	MarioGame().addCoin();
	MarioGame().playSound("coin");
	getParent()->addObject(new CTwistedCoin(getPosition() + Vector::up * 32, nullptr));
	getParent()->removeObject(this);
}

void  CCoin::onActivated()
{
	setSize({ getProperty("height").asFloat(),getProperty("width").asFloat() });
}

void CCoin::start()
{
	m_mario = getParent()->findObjectByName<CMario>("Mario");
}

//---------------------------------------------------------------------------------------------------

CMushroom::CMushroom(const Vector& pos, CMario* mario)
{
	m_as_flower = !mario->isSmall();
	m_sprite.setTexture(*MarioGame().textureManager().get("Items"));
	setSize({ 31, 1 });
	setPosition(pos.x, pos.y + 32);
	m_mario = mario;
	if (!m_as_flower)
		m_sprite.setTextureRect({ 128,150,0,0 });
	else
		m_sprite.setTextureRect({ 32,212,0,0 });
}

void CMushroom::draw(sf::RenderWindow* render_window)
{
	m_sprite.setPosition(getPosition());
	render_window->draw(m_sprite);
}

void CMushroom::update(int delta_time)
{
	m_timer += delta_time;
	if (getPosition().y > 1000) //falled underground
		getParent()->removeObject(this);

	switch (m_state)
	{
		case(wait):
		{
			if (m_timer > 400)
			{
				m_state = borning;
				MarioGame().playSound("powerup_appears");
				m_timer = 0;
			}
			break;
		}
		case(borning):
		{
			int height = m_timer * 0.02;
			if (height <= 32)
			{
				m_sprite.setTextureRect({ m_sprite.getTextureRect().left, m_sprite.getTextureRect().top,32,height });
				auto old_bounds = getBounds();
				old_bounds.setTop(old_bounds.bottom() - height);
				setBounds(old_bounds);
			}
			else
			{
				m_state = State::normal;
				m_speed = Vector::zero;
			}
			break;
		}
		case(normal):
		{
			if (!m_as_flower)
			{

				if (m_state == normal)
				{
					m_speed.x = run_speed;
					m_speed += Vector::down*gravity_force * delta_time;   // Gravity force
					move(delta_time*m_speed);
					ECollisionTag collision_tag = ECollisionTag::none;
					setPosition(m_blocks->collsionResponse(getBounds(), m_speed, delta_time, collision_tag));
					if ((collision_tag & ECollisionTag::left) || (collision_tag & ECollisionTag::right))
						run_speed = -run_speed;
					if ((collision_tag & ECollisionTag::floor) || (collision_tag & ECollisionTag::cell))
						m_speed.y = 0;
				}
			}
			else
			{
				int sprite_index = int(m_timer *0.01) % 5;
				if (sprite_index > 2)
					sprite_index = 5 - sprite_index;
				m_sprite.setTextureRect({ 32 + sprite_index * 32 ,212,32,32 });
			}
			if (m_mario && m_mario->getBounds().isIntersect(getBounds()))
			{
				action();
				getParent()->removeObject(this);
			}
			break;
		}
	}
}

void CMushroom::action()
{
	m_mario->promote();
	MarioGame().addScore(1000, getBounds().center());
	MarioGame().playSound("powerup");
}

void CMushroom::start()
{
	m_blocks = getParent()->findObjectByType<CBlocks>();
}

void CMushroom::addImpulse(const Vector& speed)
{
	m_speed += speed;
}

//-----------------------------------------------------------------------------------------------------------

COneUpMushroom::COneUpMushroom(const Vector& pos, CMario* mario) :CMushroom(pos, mario)
{
	m_as_flower = false;
	m_sprite.setTextureRect({ 160,150,32,0 });
}

void COneUpMushroom::action()
{
	MarioGame().addLive();
	MarioGame().playSound("1-up");
}

//------------------------------------------------------------------------------------------------

CStar::CStar(const Vector& pos, CMario* mario)
{
	m_sprite.setTexture(*MarioGame().textureManager().get("Items"));
	m_sprite.setTextureRect({ 0,0,0,0 });
	setSize({ 31, 1 });
	setPosition(pos.x, pos.y + 32);
	m_mario = mario;
	m_speed.y = -jump_power;
	m_speed.x = horizontal_speed;
}

void CStar::draw(sf::RenderWindow* render_window)
{
	m_sprite.setPosition(getPosition());
	render_window->draw(m_sprite);
}

void CStar::start()
{
	m_blocks = getParent()->findObjectByType<CBlocks>();
}

void CStar::update(int delta_time)
{
	m_timer += delta_time;

	int sprite_index = int(m_timer *0.01) % 5;
	if (sprite_index > 2)
		sprite_index = 5 - sprite_index;

	switch (m_state)
	{
	case(wait):
	{
		if (m_timer > 400)
		{
			m_state = borning;
			MarioGame().playSound("powerup_appears");
			m_timer = 0;
		}
		break;
	}
	case(borning):
	{
		int height = m_timer * 0.02;
		if (height <= 32)
		{
			m_sprite.setTextureRect({ 128 + sprite_index * 32 ,212,32,height });
			auto old_bounds = getBounds();
			old_bounds.setTop(old_bounds.bottom() - height);
			setBounds(old_bounds);
		}
		else
			m_state = State::normal;
		break;
	}
	case(normal):
	{
		m_sprite.setTextureRect({ 128 + sprite_index * 32 ,212,32,32 });

		//update collissions 
		auto collision_tag = ECollisionTag::none;
		m_blocks->collsionResponse(getBounds(), m_speed, delta_time, collision_tag);

		//jumping processing
		if (collision_tag & ECollisionTag::floor)
			m_speed.y = -jump_power;

		if (collision_tag & ECollisionTag::right || collision_tag & ECollisionTag::left)
			m_speed.x = -m_speed.x;

		//update physics 
		m_speed += Vector::down*gravity_force*delta_time;
		move(delta_time*m_speed);

		//check if fall undergound
		if (getPosition().y > 1000)
			getParent()->removeObject(this);

		//check if mario pickup star
		if (m_mario && m_mario->getBounds().isIntersect(getBounds()))
		{
			getParent()->removeObject(this);
			CMarioGame::instance()->invincibleMode(true);
		}
		break;
	}
	}
}
